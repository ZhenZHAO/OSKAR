/*
 * Copyright (c) 2013, The University of Oxford
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

#include <oskar_global.h>

#include <apps/lib/oskar_OptionParser.h>

#include <apps/lib/oskar_sim_tec_screen.h>
#include <apps/lib/oskar_evaluate_station_pierce_points.h>
#include <apps/lib/oskar_settings_load.h>
#include <apps/lib/oskar_set_up_telescope.h>
#include <apps/lib/oskar_set_up_sky.h>

#include <interferometry/oskar_TelescopeModel.h>
#include <interferometry/oskar_offset_geocentric_cartesian_to_geocentric_cartesian.h>

#include <sky/oskar_SkyModel.h>
#include <sky/oskar_mjd_to_gast_fast.h>
#include <sky/oskar_ra_dec_to_hor_lmn.h>
#include <sky/oskar_sky_model_free.h>

#include <utility/oskar_get_error_string.h>
#include <utility/oskar_log_error.h>
#include <utility/oskar_log_message.h>
#include <utility/oskar_Log.h>
#include <utility/oskar_log_settings.h>
#include <utility/oskar_mem_get_pointer.h>

#include <utility/oskar_mem_init.h>
#include <utility/oskar_mem_free.h>

#include <imaging/oskar_Image.h>
#include <imaging/oskar_image_free.h>
#include <imaging/oskar_evaluate_image_lon_lat_grid.h>

#include <station/oskar_evaluate_pierce_points.h>

#include <cstdlib>
#include <cstdio>
#include <cmath>

#include <unistd.h>
#include <cstring>
#include <cstdarg>
#include <algorithm>
#define PNG_DEBUG 3
#include <png.h>
#include <cfloat>


int get_lon_lat_quad_coords(double* coords, oskar_Settings* settings,
        oskar_Log* log);
void evalaute_station_beam_pp(double* pp_lon0, double* pp_lat0,
        int stationID, oskar_Settings* settings,
        oskar_TelescopeModel* telescope, int* status);

void write_kml(const char* kml_file, const char* image_file, double* coords,
        oskar_Mem& pp_lon, oskar_Mem& pp_lat);
void write_kml_ground_overlay(FILE* file, const double* coords, const char* image);
void write_kml_pp_scatter(FILE* file, oskar_Mem& pp_lon, oskar_Mem& pp_lat,
        double alt_m = 300010.0);
void write_kml_pp_vectors(FILE* file);
inline void setRGB(png_byte *ptr, float val, float red_val, float blue_val);
void abort_(const char* s, ...);
void image_to_png(const char* filename, oskar_Image& image);
int evaluate_pp(oskar_Mem& pp_lon, oskar_Mem& pp_lat, oskar_Settings& settings,
        oskar_Log* log);


// Add a KML settings group into the ionosphere settings ??
// KML
//   - Show TEC screen image. [bool]
//        function of station?
//   - Show pierce points.    [bool]
//        for one or all stations?
//
// === Make RGBA image instead of RBG.
// === How to deal with time?
//

int main(int argc, char** argv)
{
    int status = OSKAR_SUCCESS;

    oskar_OptionParser opt("oskar_sim_tec_screen");
    opt.addRequired("settings file");
    if (!opt.check_options(argc, argv))
        return OSKAR_ERR_INVALID_ARGUMENT;

    // Create the log.
    oskar_Log log;
    oskar_log_message(&log, 0, "Running binary %s", argv[0]);

    const char* settings_file = opt.getArg(0);
    oskar_Settings settings;
    oskar_settings_load(&settings, &log, settings_file);
    log.keep_file = settings.sim.keep_log_file;

    oskar_log_settings_telescope(&log, &settings);
    oskar_log_settings_observation(&log, &settings);
    oskar_log_settings_ionosphere(&log, &settings);

    // Generate or load TEC screen image
    // -------------------------------------------------------------------------
    oskar_Image TEC_screen;
    status = oskar_sim_tec_screen(&TEC_screen, &settings, &log);

    // Extract corner lon, lat values.
    double coords[8];
    status = get_lon_lat_quad_coords(coords, &settings, &log);
    //    for (int i = 0; i < 8; i+=2)
    //        printf("%i) %f %f\n", i, coords[i], coords[i+1]);

    // Convert the image to PNG and save the file.
    const char* im_file = "TEMP_TEC.png";
    image_to_png(im_file, TEC_screen);

    oskar_Mem pp_lon, pp_lat;

    // Generate or load pierce points
    // -------------------------------------------------------------------------
    status = evaluate_pp(pp_lon, pp_lat, settings, &log);
    if (status != OSKAR_SUCCESS)
        oskar_log_error(&log, "XXX: %s.", oskar_get_error_string(status));

    // Write out KML
    // -------------------------------------------------------------------------
    const char* kml_file = "test.kml";
    write_kml(kml_file, im_file, coords, pp_lon, pp_lat);

    oskar_image_free(&TEC_screen, &status);
    oskar_mem_free(&pp_lon, &status);
    oskar_mem_free(&pp_lat, &status);
}

int evaluate_pp(oskar_Mem& pp_lon, oskar_Mem& pp_lat, oskar_Settings& settings,
        oskar_Log* log)
{
    int status = OSKAR_SUCCESS;

    oskar_TelescopeModel telescope;
    oskar_set_up_telescope(&telescope, log, &settings, &status);

    oskar_SkyModel* sky = NULL;
    int num_sky_chunks = 0;
    status = oskar_set_up_sky(&num_sky_chunks, &sky, log, &settings);

    // FIXME remove this restriction ... (loop over chunks)
    if (num_sky_chunks != 1)
        return OSKAR_ERR_SETUP_FAIL_SKY;

    // FIXME remove this restriction ... (see evaluate Z)
    if (settings.ionosphere.num_TID_screens != 1)
        return OSKAR_ERR_SETUP_FAIL;

    int type = settings.sim.double_precision ? OSKAR_DOUBLE : OSKAR_SINGLE;
    int loc = OSKAR_LOCATION_CPU;

    oskar_SkyModel* chunk = &sky[0];
    int num_sources = chunk->num_sources;
    oskar_Mem hor_x, hor_y, hor_z;
    int owner = OSKAR_TRUE;
    oskar_mem_init(&hor_x, type, loc, num_sources, owner, &status);
    oskar_mem_init(&hor_y, type, loc, num_sources, owner, &status);
    oskar_mem_init(&hor_z, type, loc, num_sources, owner, &status);

    oskar_Mem pp_rel_path;
    int num_stations = telescope.num_stations;

    int num_pp = num_stations * num_sources;
    oskar_mem_init(&pp_lon, type, loc, num_pp, owner, &status);
    oskar_mem_init(&pp_lat, type, loc, num_pp, owner, &status);
    oskar_mem_init(&pp_rel_path, type, loc, num_pp, owner, &status);

    // Pierce points for one station (non-owned oskar_Mem pointers)
    oskar_Mem pp_st_lon, pp_st_lat, pp_st_rel_path;
    oskar_mem_init(&pp_st_lon, type, loc, num_sources, !owner, &status);
    oskar_mem_init(&pp_st_lat, type, loc, num_sources, !owner, &status);
    oskar_mem_init(&pp_st_rel_path, type, loc, num_sources, !owner, &status);

    int num_times = settings.obs.num_time_steps;
    double obs_start_mjd_utc = settings.obs.start_mjd_utc;
    double dt_dump = settings.obs.dt_dump_days;
    double screen_height_m = settings.ionosphere.TID->height_km * 1000.0;

    num_times = 1; // XXX restriction made to match image.
    for (int t = 0; t < num_times; ++t)
    {
        double t_dump = obs_start_mjd_utc + t * dt_dump; // MJD UTC
        double gast = oskar_mjd_to_gast_fast(t_dump + dt_dump / 2.0);

        for (int i = 0; i < num_stations; ++i)
        {
            oskar_StationModel* station = &telescope.station[i];
            double lon = station->longitude_rad;
            double lat = station->latitude_rad;
            double alt = station->altitude_m;
            double x_ecef, y_ecef, z_ecef;
            double x_offset,y_offset,z_offset;

            if (type == OSKAR_DOUBLE)
            {
                x_offset = ((double*)telescope.station_x.data)[i];
                y_offset = ((double*)telescope.station_y.data)[i];
                z_offset = ((double*)telescope.station_z.data)[i];
            }
            else
            {
                x_offset = (double)((float*)telescope.station_x.data)[i];
                y_offset = (double)((float*)telescope.station_y.data)[i];
                z_offset = (double)((float*)telescope.station_z.data)[i];
            }

            oskar_offset_geocentric_cartesian_to_geocentric_cartesian(
                    1, &x_offset, &y_offset, &z_offset, lon, lat,
                    alt, &x_ecef, &y_ecef, &z_ecef);
            double last = gast + lon;

            if (type == OSKAR_DOUBLE)
            {
                oskar_ra_dec_to_hor_lmn_d(chunk->num_sources,
                        (double*)chunk->RA.data, (double*)chunk->Dec.data,
                        last, lat, (double*)hor_x.data, (double*)hor_y.data,
                        (double*)hor_z.data);
            }
            else
            {
                oskar_ra_dec_to_hor_lmn_f(chunk->num_sources,
                        (float*)chunk->RA.data, (float*)chunk->Dec.data,
                        last, lat, (float*)hor_x.data, (float*)hor_y.data,
                        (float*)hor_z.data);
            }

            int offset = i * num_sources;
            oskar_mem_get_pointer(&pp_st_lon, &pp_lon, offset, num_sources,
                    &status);
            oskar_mem_get_pointer(&pp_st_lat, &pp_lat, offset, num_sources,
                    &status);
            oskar_mem_get_pointer(&pp_st_rel_path, &pp_rel_path, offset, num_sources,
                    &status);
            oskar_evaluate_pierce_points(&pp_st_lon, &pp_st_lat, &pp_st_rel_path,
                    lon, lat, alt, x_ecef, y_ecef, z_ecef, screen_height_m,
                    num_sources, &hor_x, &hor_y, &hor_z, &status);
        } // Loop over stations.

        if (status != OSKAR_SUCCESS)
            continue;
    } // Loop over times

    // clean up memory
    oskar_mem_free(&hor_x, &status);
    oskar_mem_free(&hor_y, &status);
    oskar_mem_free(&hor_z, &status);
    oskar_mem_free(&pp_rel_path, &status);
    oskar_mem_free(&pp_st_lon, &status);
    oskar_mem_free(&pp_st_lat, &status);
    oskar_mem_free(&pp_st_rel_path, &status);
    oskar_sky_model_free(&sky[0], &status);
    free(sky);

    return status;
}


int get_lon_lat_quad_coords(double* coords, oskar_Settings* settings,
        oskar_Log* log)
{
    int status = OSKAR_SUCCESS;
    oskar_TelescopeModel telescope;
    oskar_set_up_telescope(&telescope, log, settings, &status);
    double fov = settings->ionosphere.TECImage.fov_rad;
    oskar_Mem pp_lon, pp_lat;
    int loc = OSKAR_LOCATION_CPU;
    int owner = OSKAR_TRUE;
    int type = settings->sim.double_precision ? OSKAR_DOUBLE : OSKAR_SINGLE;
    int im_size = settings->ionosphere.TECImage.size;
    int num_pixels = im_size * im_size;
    int st_idx = settings->ionosphere.TECImage.stationID;
    double pp_lon0, pp_lat0;
    if (settings->ionosphere.TECImage.beam_centred)
    {
        evalaute_station_beam_pp(&pp_lon0, &pp_lat0, st_idx, settings,
                &telescope, &status);
    }
    else
    {
        pp_lon0 = telescope.station[st_idx].beam_longitude_rad;
        pp_lat0 = telescope.station[st_idx].beam_latitude_rad;
    }
    //printf("lon0, lat0: %f, %f\n", pp_lon0*180./M_PI, pp_lat0*180./M_PI);

    oskar_mem_init(&pp_lon, type, loc, num_pixels, owner, &status);
    oskar_mem_init(&pp_lat, type, loc, num_pixels, owner, &status);
    oskar_evaluate_image_lon_lat_grid(&pp_lon, &pp_lat, im_size, fov,
            pp_lon0, pp_lat0, &status);
    double rad2deg = 180.0/M_PI;
    if (type == OSKAR_DOUBLE)
    {
        double* lon = (double*)pp_lon.data;
        double* lat = (double*)pp_lat.data;
        // Counter clockwise from lower-left (convex shape)
        int x = 0, y = 0;
        coords[0] = lon[y * im_size + x] * rad2deg; // 0
        coords[1] = lat[y * im_size + x] * rad2deg;
        x = im_size-1; y = 0;
        coords[2] = lon[y * im_size + x] * rad2deg; // 1
        coords[3] = lat[y * im_size + x] * rad2deg;
        x = im_size-1; y = im_size-1;
        coords[5] = lon[y * im_size + x] * rad2deg; // 2
        coords[6] = lat[y * im_size + x] * rad2deg;
        x = 0; y = im_size-1;
        coords[6] = lon[y * im_size + x] * rad2deg; // 3
        coords[7] = lat[y * im_size + x] * rad2deg;
    }
    else
    {
        float* lon = (float*)pp_lon.data;
        float* lat = (float*)pp_lat.data;
        int x = 0, y = 0;
        coords[0] = (double)lon[y * im_size + x] * rad2deg; // 0
        coords[1] = (double)lat[y * im_size + x] * rad2deg;
        x = im_size-1; y = 0;
        coords[2] = (double)lon[y * im_size + x] * rad2deg; // 1
        coords[3] = (double)lat[y * im_size + x] * rad2deg;
        x = im_size-1; y = im_size-1;
        coords[4] = (double)lon[y * im_size + x] * rad2deg; // 3
        coords[5] = (double)lat[y * im_size + x] * rad2deg;
        x = 0; y = im_size-1;
        coords[6] = (double)lon[y * im_size + x] * rad2deg; // 2
        coords[7] = (double)lat[y * im_size + x] * rad2deg;
    }

    oskar_mem_free(&pp_lon, &status);
    oskar_mem_free(&pp_lat, &status);
    return status;
}

void evalaute_station_beam_pp(double* pp_lon0, double* pp_lat0,
        int stationID, oskar_Settings* settings, oskar_TelescopeModel* telescope,
        int* status)
{
    int type = settings->sim.double_precision ? OSKAR_DOUBLE : OSKAR_SINGLE;

    int loc = OSKAR_LOCATION_CPU;
    int owner = OSKAR_TRUE;

    oskar_StationModel* station = &telescope->station[stationID];

    // oskar_Mem holding beam p.p. horizontal coordinates.
    oskar_Mem hor_x, hor_y, hor_z;
    oskar_mem_init(&hor_x, type, loc, 1, owner, status);
    oskar_mem_init(&hor_y, type, loc, 1, owner, status);
    oskar_mem_init(&hor_z, type, loc, 1, owner, status);

    // Offset geocentric cartesian station position
    double st_x, st_y, st_z;

    // ECEF coordinates of the station for which the beam p.p. is being evaluated.
    double st_x_ecef, st_y_ecef, st_z_ecef;

    double st_lon = station->longitude_rad;
    double st_lat = station->latitude_rad;
    double st_alt = station->altitude_m;

    // Time at which beam p.p. is evaluated.
    int t = 0; // XXX
    double obs_start_mjd_utc = settings->obs.start_mjd_utc;
    double dt_dump = settings->obs.dt_dump_days;
    double t_dump = obs_start_mjd_utc + t * dt_dump; // MJD UTC
    double gast = oskar_mjd_to_gast_fast(t_dump + dt_dump / 2.0);
    double last = gast + st_lon;

    if (type == OSKAR_DOUBLE)
    {
        st_x = ((double*)telescope->station_x.data)[stationID];
        st_y = ((double*)telescope->station_y.data)[stationID];
        st_z = ((double*)telescope->station_z.data)[stationID];

        oskar_offset_geocentric_cartesian_to_geocentric_cartesian(1,
                &st_x, &st_y, &st_z, st_lon, st_lat, st_alt, &st_x_ecef,
                &st_y_ecef, &st_z_ecef);

        double beam_ra = station->beam_longitude_rad;
        double beam_dec = station->beam_latitude_rad;

        // Obtain horizontal coordinates of beam p.p.
        oskar_ra_dec_to_hor_lmn_d(1, &beam_ra, &beam_dec, last, st_lat,
                (double*)hor_x.data, (double*)hor_y.data, (double*)hor_z.data);
    }
    else // (type == OSKAR_SINGLE)
    {
        st_x = (double)((float*)telescope->station_x.data)[stationID];
        st_y = (double)((float*)telescope->station_y.data)[stationID];
        st_z = (double)((float*)telescope->station_z.data)[stationID];

        oskar_offset_geocentric_cartesian_to_geocentric_cartesian(1,
                &st_x, &st_y, &st_z, st_lon, st_lat, st_alt, &st_x_ecef,
                &st_y_ecef, &st_z_ecef);

        float beam_ra = (float)station->beam_longitude_rad;
        float beam_dec = (float)station->beam_latitude_rad;

        // Obtain horizontal coordinates of beam p.p.
        oskar_ra_dec_to_hor_lmn_f(1, &beam_ra, &beam_dec, last, st_lat,
                (float*)hor_x.data, (float*)hor_y.data, (float*)hor_z.data);
    }

    // oskar_Mem functions holding the pp for the beam centre.
    oskar_Mem m_pp_lon0, m_pp_lat0, m_pp_rel_path;
    oskar_mem_init(&m_pp_lon0, type, loc, 1, owner, status);
    oskar_mem_init(&m_pp_lat0, type, loc, 1, owner, status);
    oskar_mem_init(&m_pp_rel_path, type, loc, 1, owner, status);

    // Pierce point of the observation phase centre - i.e. beam direction
    oskar_evaluate_pierce_points(&m_pp_lon0, &m_pp_lat0, &m_pp_rel_path,
            st_lon, st_lat, st_alt, st_x_ecef, st_y_ecef, st_z_ecef,
            settings->ionosphere.TID[0].height_km * 1000., 1, &hor_x, &hor_y,
            &hor_z, status);

    if (type == OSKAR_DOUBLE)
    {
        *pp_lon0 = ((double*)m_pp_lon0.data)[0];
        *pp_lat0 = ((double*)m_pp_lat0.data)[0];
    }
    else
    {
        *pp_lon0 = ((float*)m_pp_lon0.data)[0];
        *pp_lat0 = ((float*)m_pp_lat0.data)[0];
    }
}


void write_kml(const char* kml_file, const char* image_file, double* coords,
        oskar_Mem& pp_lon, oskar_Mem& pp_lat)
{
    FILE* file = fopen(kml_file, "w");
    fprintf(file,
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<kml xmlns=\"http://www.opengis.net/kml/2.2\" "
            "   xmlns:gx=\"http://www.google.com/kml/ext/2.2\">\"\n"
            "\n"
            "<Folder>\n"
            "   <name>OSKAR TEC</name>\n"
            "\n"
            "   <Style id=\"pp\">\n"
            "       <IconStyle>\n"
            "           <Icon><href>cross.png</href></Icon>\n"
            "           <scale>0.5</scale>\n"
            "       </IconStyle>\n"
            "   </Style>\n"
            "\n"
            "   <Style id=\"pp_vec\">\n"
            "       <LineStyle>\n"
            "           <color>64140096</color>\n"
            "           <width>6</width>\n"
            "       </LineStyle>\n"
            "       <PolyStyle>\n"
            "           <color>7f00ff00</color>\n"
            "       </PolyStyle>\n"
            "   </Style>\n"
    );

    write_kml_ground_overlay(file, coords, image_file);
    write_kml_pp_scatter(file, pp_lon, pp_lat);
    //    write_kml_pp_vectors(file);

    fprintf(file, "</Folder>\n");
    fprintf(file, "</kml>\n");
}

void write_kml_ground_overlay(FILE* file, const double* coords, const char* image)
{
    if (!file || !coords || !image)
        return;

    fprintf(file,
            "\n"
            "   <GroundOverlay>\n"
            "       <name>TEC screen 1</name>\n"
    );
    fprintf(file,
            "       <Icon><href>%s</href></Icon>\n"
            "       <altitudeMode>absolute</altitudeMode>\n"
            "       <altitude>300000.0</altitude>\n"
            "       <gx:LatLonQuad>\n"
            "       <coordinates>\n",
            image
    );
    fprintf(file,
            "           %f,%f\n"
            "           %f,%f\n"
            "           %f,%f\n"
            "           %f,%f\n",
            coords[0],coords[1],
            coords[2],coords[3],
            coords[4],coords[5],
            coords[6],coords[7]
    );
    fprintf(file,
            "       </coordinates>\n"
            "       </gx:LatLonQuad>\n"
            "   </GroundOverlay>\n"
            "\n"
    );
}

void write_kml_pp_scatter(FILE* file, oskar_Mem& pp_lon, oskar_Mem& pp_lat,
        double alt_m)
{
    int num_pp = pp_lon.num_elements;

    fprintf(file,
            "\n"
            "   <Placemark>\n"
            "       <styleUrl> #pp</styleUrl>\n"
            "       <MultiGeometry>\n"
    );
    double rad2deg = 180./M_PI;
    for (int i = 0; i < num_pp;++i)
    {
        double lon, lat;
        if (pp_lon.type == OSKAR_DOUBLE)
        {
            lon = ((double*)pp_lon.data)[i]*rad2deg;
            lat = ((double*)pp_lat.data)[i]*rad2deg;
        }
        else
        {
            lon = (double)((float*)pp_lon.data)[i]*rad2deg;
            lat = (double)((float*)pp_lat.data)[i]*rad2deg;
        }
        fprintf(file,
                "               <Point>\n"
                "                   <altitudeMode>absolute</altitudeMode>\n"
                "                   <coordinates>\n"
                "                       %f,%f,%f\n"
                "                   </coordinates>\n"
                "               </Point>\n",
                lon, lat, alt_m
        );
    }
    fprintf(file,
            "           </MultiGeometry>\n"
            "   </Placemark>\n"
            "\n"
    );
}

void write_kml_pp_vectors(FILE* file)
{
    fprintf(file,
            ""
            "   <Placemark>"
            "       <styleUrl> #pp_vec</styleUrl>"
            "       <LineString>"
            "           <extrude>0</extrude>"
            "           <tessellate>0</tessellate>"
            "           <altitudeMode>absolute</altitudeMode>"
            "           <coordinates>"
            "               0.0,-50.0,0.0"
            "               0.00061715,-49.999,300000.0"
            "           </coordinates>"
            "       </LineString>"
            "   </Placemark>"
            ""
    );
}

inline void setRGB(png_byte *ptr, float val, float red_val, float blue_val)
{
    int v = (int)(1023 * (val - red_val) / (blue_val - red_val));
    if (v < 256)
    {
        ptr[0] = 255;
        ptr[1] = v;
        ptr[2] = 0;
    }
    else if (v < 512)
    {
        v -= 256;
        ptr[0] = 255-v;
        ptr[1] = 255;
        ptr[2] = 0;
    }
    else if (v < 768)
    {
        v -= 512;
        ptr[0] = 0;
        ptr[1] = 255;
        ptr[2] = v;
    }
    else
    {
        v -= 768;
        ptr[0] = 0;
        ptr[1] = 255-v;
        ptr[2] = 255;
    }
}


void abort_(const char* s, ...)
{
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}

void image_to_png(const char* filename, oskar_Image& img)
{
    int status = OSKAR_SUCCESS;
    int num_pixels = img.width*img.height;
    // Get a single slice of the image
    oskar_Mem img_slice;
    int t = 0;
    int p = 0;
    int c = 0;
    // XXX slice 0.0.0 (chan.time.pol)
    int slice_offset = ((c * img.num_times + t) * img.num_pols + p) * num_pixels;
    oskar_mem_get_pointer(&img_slice, &(img.data), slice_offset,
            num_pixels, &status);

    int x, y;
    png_byte bit_depth;
    png_structp png_ptr;
    png_infop info_ptr;

    // Create the file.
    FILE* fp = fopen(filename, "wb");
    if (!fp)
        abort_("Failed to open PNG file for writing");

    // Init.
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
        abort_("png_create_write_struct() failed");

    info_ptr = png_create_info_struct(png_ptr);
    if (!png_ptr)
        abort_("png_create_info_struct() failed");

    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("Error during init_io");

    png_init_io(png_ptr, fp);

    // Write header
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("Error during write header");

    bit_depth = 8;
    png_set_IHDR(png_ptr, info_ptr,
            img.width, img.height, bit_depth,
            PNG_COLOR_TYPE_RGB,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE,
            PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    // Write bytes
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("error during writing bytes");

    // Allocate memory for one row, 3 bytes per pixel - RGB.
    png_bytep row;
    row = (png_bytep) malloc(3 * img.width * sizeof(png_byte));

    float* img_data = (float*)img_slice.data;

    float red = -FLT_MAX; // max
    float blue = FLT_MAX; // min
    for (int i = 0; i < num_pixels; ++i)
    {
        blue = std::min(blue, img_data[i]);
        red = std::max(red, img_data[i]);
    }

    for (y = 0; y < img.height; y++)
    {
        for (x = 0; x < img.width; x++)
        {
            int idx = (img.height-y-1)*img.width + x;
            setRGB(&row[x*3], img_data[idx], red, blue);
        }
        png_write_row(png_ptr, row);
    }

    // End write
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("error during end of write");

    png_write_end(png_ptr, NULL);

    fclose(fp);
    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    free(row);
}


