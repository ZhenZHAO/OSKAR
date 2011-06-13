/*
 * Copyright (c) 2011, The University of Oxford
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

#ifndef OSKAR_SKY_CUDAK_HA_DEC_TO_HOR_LMN_H_
#define OSKAR_SKY_CUDAK_HA_DEC_TO_HOR_LMN_H_

/**
 * @file oskar_sky_cudak_ha_dec_to_hor_lmn.h
 */

#include "utility/oskar_util_cuda_eclipse.h"

/**
 * @brief
 * CUDA kernel to compute local source direction cosines (single precision).
 *
 * @details
 * This CUDA kernel transforms sources specified in a local equatorial
 * system (HA, Dec) to direction cosines l,m,n in the horizontal system.
 *
 * The directions are:
 * <li> l is parallel to x (pointing East), </li>
 * <li> m is parallel to y (pointing North), </li>
 * <li> n is parallel to z (pointing to the zenith). </li>
 *
 * If the source n-value is less than 0, then the source is below the horizon.
 *
 * @param[in] ns The number of source positions.
 * @param[in] ha The source Hour Angles in radians.
 * @param[in] dec The source Declinations in radians.
 * @param[in] cosLat The cosine of the geographic latitude.
 * @param[in] sinLat The sine of the geographic latitude.
 * @param[out] l The source l-coordinates (see above).
 * @param[out] m The source m-coordinates (see above).
 * @param[out] n The source n-coordinates (see above).
 */
__global__
void oskar_sky_cudakf_ha_dec_to_hor_lmn(int ns, const float* ha,
        const float* dec, float cosLat, float sinLat, float* l, float* m,
        float* n);

/**
 * @brief
 * CUDA kernel to compute local source direction cosines (double precision).
 *
 * @details
 * This CUDA kernel transforms sources specified in a local equatorial
 * system (HA, Dec) to direction cosines l,m,n in the horizontal system.
 *
 * The directions are:
 * <li> l is parallel to x (pointing East), </li>
 * <li> m is parallel to y (pointing North), </li>
 * <li> n is parallel to z (pointing to the zenith). </li>
 *
 * If the source n-value is less than 0, then the source is below the horizon.
 *
 * @param[in] ns The number of source positions.
 * @param[in] ha The source Hour Angles in radians.
 * @param[in] dec The source Declinations in radians.
 * @param[in] cosLat The cosine of the geographic latitude.
 * @param[in] sinLat The sine of the geographic latitude.
 * @param[out] l The source l-coordinates (see above).
 * @param[out] m The source m-coordinates (see above).
 * @param[out] n The source n-coordinates (see above).
 */
__global__
void oskar_sky_cudakd_ha_dec_to_hor_lmn(int ns, const double2* radec,
        double cosLat, double sinLat, double2* azel);

#endif // OSKAR_SKY_CUDAK_HA_DEC_TO_HOR_LMN_H_
