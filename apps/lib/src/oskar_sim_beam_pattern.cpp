/*
 * Copyright (c) 2012-2013, The University of Oxford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Oxford nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <cuda_runtime_api.h>
#include "apps/lib/oskar_settings_load.h"
#include "apps/lib/oskar_set_up_telescope.h"
#include "apps/lib/oskar_sim_beam_pattern.h"
#include "fits/oskar_fits_image_write.h"
#include "interferometry/oskar_telescope_model_copy.h"
#include "interferometry/oskar_telescope_model_free.h"
#include "interferometry/oskar_telescope_model_init.h"
#include "interferometry/oskar_telescope_model_multiply_by_wavenumber.h"
#include "imaging/oskar_evaluate_image_lon_lat_grid.h"
#include "imaging/oskar_image_resize.h"
#include "imaging/oskar_image_write.h"
#include "sky/oskar_mjd_to_gast_fast.h"
#include "sky/oskar_ra_dec_to_rel_lmn.h"
#include "station/oskar_evaluate_source_horizontal_lmn.h"
#include "station/oskar_evaluate_station_beam_aperture_array.h"
#include "station/oskar_evaluate_station_beam_gaussian.h"
#include "station/oskar_work_station_beam_free.h"
#include "station/oskar_work_station_beam_init.h"
#include "utility/oskar_cuda_mem_log.h"
#include "utility/oskar_curand_state_free.h"
#include "utility/oskar_curand_state_init.h"
#include "utility/oskar_log_error.h"
#include "utility/oskar_log_message.h"
#include "utility/oskar_log_section.h"
#include "utility/oskar_log_settings.h"
#include "utility/oskar_mem_copy.h"
#include "utility/oskar_mem_free.h"
#include "utility/oskar_mem_init.h"
#include "utility/oskar_mem_insert.h"
#include "utility/oskar_mem_type_check.h"
#include "utility/oskar_Settings.h"
#include "utility/oskar_settings_free.h"
#include "utility/oskar_vector_types.h"

#include <cmath>
#include <QtCore/QTime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void oskar_set_up_beam_pattern(oskar_Image* image,
        const oskar_Settings* settings, int* status);

extern "C"
int oskar_sim_beam_pattern(const char* settings_file, oskar_Log* log)
{
    int err = 0;
    QTime timer;

    // Load the settings file.
    oskar_Settings settings;
    oskar_log_section(log, "Loading settings file '%s'", settings_file);
    err = oskar_settings_load(&settings, log, settings_file);
    if (err) return err;
    int type = settings.sim.double_precision ? OSKAR_DOUBLE : OSKAR_SINGLE;

    // Log the relevant settings.
    log->keep_file = settings.sim.keep_log_file;
    oskar_log_settings_simulator(log, &settings);
    oskar_log_settings_observation(log, &settings);
    oskar_log_settings_telescope(log, &settings);
    oskar_log_settings_beam_pattern(log, &settings);

    // Check that a data file has been specified.
    if (!(settings.beam_pattern.oskar_image_voltage ||
            settings.beam_pattern.oskar_image_phase ||
            settings.beam_pattern.oskar_image_complex ||
            settings.beam_pattern.fits_image_voltage ||
            settings.beam_pattern.fits_image_phase))
    {
        oskar_log_error(log, "No output file(s) specified.");
        return OSKAR_ERR_SETTINGS;
    }

    // Get time data.
    int num_times            = settings.obs.num_time_steps;
    double obs_start_mjd_utc = settings.obs.start_mjd_utc;
    double dt_dump           = settings.obs.dt_dump_days;

    // Get the telescope model.
    oskar_TelescopeModel tel_cpu;
    oskar_set_up_telescope(&tel_cpu, log, &settings, &err);
    if (err) return err;

    // Get the beam pattern settings.
    int station_id = settings.beam_pattern.station_id;
    int image_size = settings.beam_pattern.size;
    int num_channels = settings.obs.num_channels;
    int num_pols = settings.telescope.aperture_array.element_pattern.functional_type ==
            OSKAR_ELEMENT_MODEL_TYPE_ISOTROPIC ? 1 : 4;
    int num_pixels = image_size * image_size;
    int num_pixels_total = num_pixels * num_times * num_channels * num_pols;
    int beam_pattern_data_type = type | OSKAR_COMPLEX;
    if (num_pols == 4)
        beam_pattern_data_type |= OSKAR_MATRIX;

    // Check station ID is within range.
    if (station_id < 0 || station_id >= tel_cpu.num_stations)
        return OSKAR_ERR_OUT_OF_RANGE;

    // Set up beam pattern hyper-cubes.
    oskar_Image complex_cube(type | OSKAR_COMPLEX, OSKAR_LOCATION_CPU);
    oskar_Image image_cube(type, OSKAR_LOCATION_CPU);
    oskar_set_up_beam_pattern(&complex_cube, &settings, &err);
    oskar_set_up_beam_pattern(&image_cube, &settings, &err);
    if (err) return err;

    // Temporary CPU memory, used to re-order polarisation data.
    oskar_Mem beam_cpu(beam_pattern_data_type, OSKAR_LOCATION_CPU, num_pixels);

    // All GPU memory used within these braces.
    {
        oskar_Mem RA, Dec, beam_pattern, l, m, n;
        oskar_WorkStationBeam work;

        // Generate equatorial coordinates for beam pattern pixels.
        oskar_mem_init(&RA, type, OSKAR_LOCATION_GPU, num_pixels, 1, &err);
        oskar_mem_init(&Dec, type, OSKAR_LOCATION_GPU, num_pixels, 1, &err);
        oskar_evaluate_image_lon_lat_grid(&RA, &Dec, image_size,
                settings.beam_pattern.fov_deg * M_PI / 180.0,
                settings.obs.ra0_rad[0], settings.obs.dec0_rad[0], &err);

        // Initialise work array and GPU memory for a beam pattern.
        oskar_work_station_beam_init(&work, type, OSKAR_LOCATION_GPU, &err);
        oskar_mem_init(&beam_pattern, beam_pattern_data_type,
                OSKAR_LOCATION_GPU, num_pixels, 1, &err);
        oskar_mem_init(&l, type, OSKAR_LOCATION_GPU, 0, 1, &err);
        oskar_mem_init(&m, type, OSKAR_LOCATION_GPU, 0, 1, &err);
        oskar_mem_init(&n, type, OSKAR_LOCATION_GPU, 0, 1, &err);

        // Evaluate source relative l,m,n values if not an aperture array.
        if (tel_cpu.station[station_id].station_type != OSKAR_STATION_TYPE_AA)
        {
            oskar_ra_dec_to_rel_lmn(num_pixels, &RA, &Dec,
                    tel_cpu.ra0_rad, tel_cpu.dec0_rad, &l, &m, &n, &err);
        }

        // Loop over channels.
        oskar_log_section(log, "Starting simulation...");
        timer.start();
        for (int c = 0; c < num_channels; ++c)
        {
            oskar_CurandState curand_state;
            oskar_TelescopeModel telescope;
            oskar_StationModel* station;
            double frequency;

            // Initialise local data structures.
            oskar_curand_state_init(&curand_state, tel_cpu.max_station_size,
                    tel_cpu.seed_time_variable_station_element_errors, 0, 0,
                    &err);

            // Get the channel frequency.
            frequency = settings.obs.start_frequency_hz +
                    c * settings.obs.frequency_inc_hz;
            oskar_log_message(log, 0, "Channel %3d/%d [%.4f MHz]",
                    c + 1, num_channels, frequency / 1e6);

            // Copy the telescope model and scale coordinates to radians.
            oskar_telescope_model_init(&telescope, type, OSKAR_LOCATION_GPU,
                    tel_cpu.num_stations, &err);
            oskar_telescope_model_copy(&telescope, &tel_cpu, &err);
            oskar_telescope_model_multiply_by_wavenumber(&telescope,
                    frequency, &err);

            // Loop over times.
            for (int t = 0; t < num_times; ++t)
            {
                double t_dump, gast;

                // Check error code.
                if (err) continue;

                // Start time for the data dump, in MJD(UTC).
                oskar_log_message(log, 1, "Snapshot %4d/%d",
                        t+1, num_times);
                t_dump = obs_start_mjd_utc + t * dt_dump;
                gast = oskar_mjd_to_gast_fast(t_dump + dt_dump / 2.0);

                // Evaluate horizontal x,y,z directions for source positions.
                station = &(telescope.station[station_id]);
                oskar_evaluate_source_horizontal_lmn(num_pixels,
                        &work.hor_x, &work.hor_y, &work.hor_z, &RA, &Dec,
                        station, gast, &err);

                // Evaluate the station beam.
                if (station->station_type == OSKAR_STATION_TYPE_AA)
                {
                    oskar_evaluate_station_beam_aperture_array(&beam_pattern,
                            station, num_pixels, &work.hor_x, &work.hor_y,
                            &work.hor_z, gast, &work, &curand_state, &err);
                }
                else if (station->station_type == OSKAR_STATION_TYPE_GAUSSIAN_BEAM)
                {
                    oskar_evaluate_station_beam_gaussian(&beam_pattern,
                            num_pixels, &l, &m, &work.hor_z,
                            station->gaussian_beam_fwhm_deg, &err);
                }
                else
                {
                    return OSKAR_ERR_SETTINGS_TELESCOPE;
                }

                // Save complex beam pattern data in the right order.
                // Cube has dimension order (from slowest to fastest):
                // Channel, Time, Polarisation, Declination, Right Ascension.
                int offset = (t + c * num_times) * num_pols * num_pixels;
                if (oskar_mem_is_scalar(beam_pattern.type))
                {
                    oskar_mem_insert(&complex_cube.data, &beam_pattern,
                            offset, &err);
                }
                else
                {
                    // Copy beam pattern back to host memory for re-ordering.
                    oskar_mem_copy(&beam_cpu, &beam_pattern, &err);
                    if (err) continue;

                    // Re-order the polarisation data.
                    if (type == OSKAR_SINGLE)
                    {
                        float2* bp = (float2*)complex_cube.data.data + offset;
                        float4c* tc = (float4c*)beam_cpu.data;
                        for (int i = 0; i < num_pixels; ++i)
                        {
                            bp[i]                  = tc[i].a; // theta_X
                            bp[i +     num_pixels] = tc[i].b; // phi_X
                            bp[i + 2 * num_pixels] = tc[i].c; // theta_Y
                            bp[i + 3 * num_pixels] = tc[i].d; // phi_Y
                        }
                    }
                    else if (type == OSKAR_DOUBLE)
                    {
                        double2* bp = (double2*)complex_cube.data.data + offset;
                        double4c* tc = (double4c*)beam_cpu.data;
                        for (int i = 0; i < num_pixels; ++i)
                        {
                            bp[i]                  = tc[i].a; // theta_X
                            bp[i +     num_pixels] = tc[i].b; // phi_X
                            bp[i + 2 * num_pixels] = tc[i].c; // theta_Y
                            bp[i + 3 * num_pixels] = tc[i].d; // phi_Y
                        }
                    }
                }

            } // Time loop

            // Record GPU memory usage.
            oskar_cuda_mem_log(log, 1, 0);

            // Free memory.
            oskar_curand_state_free(&curand_state, &err);
            oskar_telescope_model_free(&telescope, &err);
        } // Channel loop

        // Free memory.
        oskar_mem_free(&RA, &err);
        oskar_mem_free(&Dec, &err);
        oskar_mem_free(&beam_pattern, &err);
        oskar_mem_free(&l, &err);
        oskar_mem_free(&m, &err);
        oskar_mem_free(&n, &err);
        oskar_work_station_beam_free(&work, &err);
    } // GPU memory section
    oskar_log_section(log, "Simulation completed in %.3f sec.",
            timer.elapsed() / 1e3);

    // Check error code.
    if (err) return err;

    // Write out complex data if required.
    if (settings.beam_pattern.oskar_image_complex)
    {
        oskar_image_write(&complex_cube, log,
                settings.beam_pattern.oskar_image_complex, 0, &err);
        if (err) return err;
    }

    // Write out power data if required.
    if (settings.beam_pattern.oskar_image_voltage ||
            settings.beam_pattern.fits_image_voltage)
    {
        // Convert complex values to power (amplitude of complex number).
        if (type == OSKAR_SINGLE)
        {
            float x, y, *image_data = (float*)image_cube.data;
            float2* complex_data = (float2*)complex_cube.data;
            for (int i = 0; i < num_pixels_total; ++i)
            {
                x = complex_data[i].x;
                y = complex_data[i].y;
                image_data[i] = sqrt(x*x + y*y);
            }
        }
        else if (type == OSKAR_DOUBLE)
        {
            double x, y, *image_data = (double*)image_cube.data;
            double2* complex_data = (double2*)complex_cube.data;
            for (int i = 0; i < num_pixels_total; ++i)
            {
                x = complex_data[i].x;
                y = complex_data[i].y;
                image_data[i] = sqrt(x*x + y*y);
            }
        }

        // Write OSKAR image
        if (settings.beam_pattern.oskar_image_voltage)
        {
            oskar_image_write(&image_cube, log,
                    settings.beam_pattern.oskar_image_voltage, 0, &err);
            if (err) return err;
        }
#ifndef OSKAR_NO_FITS
        // Write FITS image.
        if (settings.beam_pattern.fits_image_voltage)
        {
            err = oskar_fits_image_write(&image_cube, log,
                    settings.beam_pattern.fits_image_voltage);
            if (err) return err;
        }
#endif
    }

    // Write out phase data if required.
    if (settings.beam_pattern.oskar_image_phase ||
            settings.beam_pattern.fits_image_phase)
    {
        // Convert complex values to phase.
        if (type == OSKAR_SINGLE)
        {
            float* image_data = (float*)image_cube.data;
            float2* complex_data = (float2*)complex_cube.data;
            for (int i = 0; i < num_pixels_total; ++i)
            {
                image_data[i] = atan2(complex_data[i].y, complex_data[i].x);
            }
        }
        else if (type == OSKAR_DOUBLE)
        {
            double* image_data = (double*)image_cube.data;
            double2* complex_data = (double2*)complex_cube.data;
            for (int i = 0; i < num_pixels_total; ++i)
            {
                image_data[i] = atan2(complex_data[i].y, complex_data[i].x);
            }
        }

        // Write OSKAR image
        if (settings.beam_pattern.oskar_image_phase)
        {
            oskar_image_write(&image_cube, log,
                    settings.beam_pattern.oskar_image_phase, 0, &err);
            if (err) return err;
        }
#ifndef OSKAR_NO_FITS
        // Write FITS image.
        if (settings.beam_pattern.fits_image_phase)
        {
            err = oskar_fits_image_write(&image_cube, log,
                    settings.beam_pattern.fits_image_phase);
            if (err) return err;
        }
#endif
    }

    cudaDeviceReset();
    return OSKAR_SUCCESS;
}

static void oskar_set_up_beam_pattern(oskar_Image* image,
        const oskar_Settings* settings, int* status)
{
    int num_times, image_size, num_channels, num_pols;
    num_times    = settings->obs.num_time_steps;
    image_size   = settings->beam_pattern.size;
    num_channels = settings->obs.num_channels;
    num_pols     = settings->telescope.aperture_array.element_pattern.functional_type ==
            OSKAR_ELEMENT_MODEL_TYPE_ISOTROPIC ? 1 : 4;

    /* Resize image cube. */
    oskar_image_resize(image, image_size, image_size, num_pols, num_times,
            num_channels, status);

    /* Set beam pattern meta-data. */
    image->image_type         = (num_pols == 1) ?
            OSKAR_IMAGE_TYPE_BEAM_SCALAR : OSKAR_IMAGE_TYPE_BEAM_POLARISED;
    image->centre_ra_deg      = settings->obs.ra0_rad[0] * 180.0 / M_PI;
    image->centre_dec_deg     = settings->obs.dec0_rad[0] * 180.0 / M_PI;
    image->fov_ra_deg         = settings->beam_pattern.fov_deg;
    image->fov_dec_deg        = settings->beam_pattern.fov_deg;
    image->freq_start_hz      = settings->obs.start_frequency_hz;
    image->freq_inc_hz        = settings->obs.frequency_inc_hz;
    image->time_inc_sec       = settings->obs.dt_dump_days * 86400.0;
    image->time_start_mjd_utc = settings->obs.start_mjd_utc;
    oskar_mem_copy(&image->settings_path, &settings->settings_path, status);
}
