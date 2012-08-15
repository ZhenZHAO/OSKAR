/*
 * Copyright (c) 2012, The University of Oxford
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


#include <mex.h>

#include "oskar_global.h"
#include "interferometry/oskar_TelescopeModel.h"
#include "interferometry/oskar_telescope_model_load_station_coords.h"
#include "interferometry/oskar_evaluate_uvw_baseline.h"
#include "utility/oskar_get_error_string.h"

#include <math.h>

// MATLAB Entry function.
void mexFunction(int num_out, mxArray** out, int num_in, const mxArray** in)
{
    // Read options from MATLAB

    if (num_in != 9 || num_out > 1)
    {
        mexErrMsgTxt(
                "Usage:\n"
                "\tuvw = oskar.visibilities.evaluate_baseline_uvw(...args...)\n\n"
                "Arguments:\n"
                "\t1) layout_file: OSKAR Station layout file.\n"
                "\t2) lon:         Telescope longitude, in degrees.\n"
                "\t3) lat:         Telescope latitude, in degrees.\n"
                "\t4) alt:         Telescope altitude, in metres.\n"
                "\t5) RA:          Observation Right Ascension, in degrees.\n"
                "\t6) Dec:         Observation Declination, in degrees.\n"
                "\t7) start time:  Observation start time, in MJD UTC.\n"
                "\t8) num_times:   Number of time integration steps.\n"
                "\t9) dt:          Integration length of each time step, in seconds.\n"
                );
    }

    // Read inputs.
    const char* filename  = mxArrayToString(in[0]);
    double lon            = (double)mxGetScalar(in[1]);
    double lat            = (double)mxGetScalar(in[2]);
    double alt            = (double)mxGetScalar(in[3]);
    double ra             = (double)mxGetScalar(in[4]);
    double dec            = (double)mxGetScalar(in[5]);
    double start_mjd_utc  = (double)mxGetScalar(in[6]);
    int num_times         = (int)mxGetScalar(in[7]);
    double dt             = (double)mxGetScalar(in[8]);

    // Convert to units required by OSKAR functions.
    lon *= M_PI / 180.0;
    lat *= M_PI / 180.0;
    ra *= M_PI / 180.0;
    dec *= M_PI / 180.0;
    dt /= 86400.0;

    // Load the telescope model station layout file
    int err = OSKAR_SUCCESS;
    oskar_TelescopeModel telescope(OSKAR_DOUBLE, OSKAR_LOCATION_CPU, 0);
    err = oskar_telescope_model_load_station_coords(&telescope, filename, lon, lat, alt);
    if (err)
    {
        mexErrMsgIdAndTxt("OSKAR:error",
                "\nError reading OSKAR station layout file file: '%s'.\nERROR: %s.",
                filename, oskar_get_error_string(err));
    }


    // Create data arrays to hold uvw baseline coordinates.
    int num_baselines = telescope.num_baselines();
    mwSize dims[] = { num_baselines, num_times };
    int num_coords = num_baselines * num_times;
    mxArray* uu_ = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    mxArray* vv_ = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    mxArray* ww_ = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    oskar_Mem uu(OSKAR_DOUBLE, OSKAR_LOCATION_CPU, num_coords, OSKAR_FALSE);
    oskar_Mem vv(OSKAR_DOUBLE, OSKAR_LOCATION_CPU, num_coords, OSKAR_FALSE);
    oskar_Mem ww(OSKAR_DOUBLE, OSKAR_LOCATION_CPU, num_coords, OSKAR_FALSE);
    uu.data = mxGetData(uu_);
    vv.data = mxGetData(vv_);
    ww.data = mxGetData(ww_);

    // Allocate work array
    oskar_Mem work_uvw(OSKAR_DOUBLE, OSKAR_LOCATION_CPU, 3 * telescope.num_stations, OSKAR_TRUE);

    oskar_evaluate_uvw_baseline(&uu, &vv, &ww, telescope.num_stations,
            &telescope.station_x, &telescope.station_y, &telescope.station_z,
            ra, dec, num_times, start_mjd_utc, dt, &work_uvw,
            &err);
    if (err)
    {
        mexErrMsgIdAndTxt("OSKAR:error",
                "\nError evaluating baseline uvw coordinates.\nERROR: %s.",
                filename, oskar_get_error_string(err));
    }

    const char* fields[] = { "uu", "vv", "ww" };
    int num_fields = 3;
    out[0] = mxCreateStructMatrix(1, 1, num_fields, fields);
    mxSetField(out[0], 0, "uu", uu_);
    mxSetField(out[0], 0, "vv", vv_);
    mxSetField(out[0], 0, "ww", ww_);
}

