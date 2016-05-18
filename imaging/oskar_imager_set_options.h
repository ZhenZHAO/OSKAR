/*
 * Copyright (c) 2016, The University of Oxford
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

#ifndef OSKAR_IMAGER_SET_OPTIONS_H_
#define OSKAR_IMAGER_SET_OPTIONS_H_

/**
 * @file oskar_imager_set_options.h
 */

#include <oskar_global.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 * Sets the algorithm used by the imager.
 *
 * @details
 * Sets the algorithm used by the imager.
 *
 * The \p type string can be:
 * - "FFT" to use gridding followed by a FFT.
 * - "DFT 2D" to use a 2D Direct Fourier Transform, without gridding.
 * - "DFT 3D" to use a 3D Direct Fourier Transform, without gridding.
 *
 * @param[in,out] h          Handle to imager.
 * @param[in]     type       The algorithm to use (see description).
 * @param[in,out] status     Status return code.
 */
OSKAR_EXPORT
void oskar_imager_set_algorithm(oskar_Imager* h, const char* type,
        int* status);

/**
 * @brief
 * Sets the visibility channel range used by the imager.
 *
 * @details
 * Sets the visibility channel range used by the imager,
 * and whether frequency-synthesis should be used.
 *
 * @param[in,out] h          Handle to imager.
 * @param[in]     start      Start channel index.
 * @param[in]     end        End channel index (-1 for all channels).
 * @param[in]     snapshots  If true, image each channel separately;
 *                           if false, use frequency synthesis.
 */
OSKAR_EXPORT
void oskar_imager_set_channel_range(oskar_Imager* h, int start, int end,
        int snapshots);

/**
 * @brief
 * Clears any direction override.
 *
 * @details
 * Clears any direction override set using oskar_imager_set_direction().
 *
 * @param[in,out] h          Handle to imager.
 */
OSKAR_EXPORT
void oskar_imager_set_default_direction(oskar_Imager* h);

/**
 * @brief
 * Sets the image centre to be different to the observation phase centre.
 *
 * @details
 * Sets the image centre to be different to the observation phase centre.
 *
 * @param[in,out] h          Handle to imager.
 * @param[in]     ra_deg     The new image Right Ascension, in degrees.
 * @param[in]     dec_deg    The new image Declination, in degrees.
 */
OSKAR_EXPORT
void oskar_imager_set_direction(oskar_Imager* h, double ra_deg, double dec_deg);

/**
 * @brief
 * Sets the image field of view.
 *
 * @details
 * Sets the field of view of the output images.
 *
 * @param[in,out] h          Handle to imager.
 * @param[in]     fov_deg    Field of view, in degrees.
 */
OSKAR_EXPORT
void oskar_imager_set_fov(oskar_Imager* h, double fov_deg);

/**
 * @brief
 * Sets whether to use the GPU for FFTs.
 *
 * @details
 * Sets whether to use the GPU for FFTs.
 *
 * @param[in,out] h          Handle to imager.
 * @param[in]     value      If true, use the GPU; if false, use the CPU.
 */
OSKAR_EXPORT
void oskar_imager_set_fft_on_gpu(oskar_Imager* h, int value);

/**
 * @brief
 * Sets which GPUs will be used by the imager.
 *
 * @details
 * Sets which GPUs will be used by the imager.
 *
 * @param[in,out] h          Handle to imager.
 * @param[in]     num        The number of GPUs to use.
 * @param[in]     ids        An array containing the GPU IDs to use.
 * @param[in,out] status     Status return code.
 */
OSKAR_EXPORT
void oskar_imager_set_gpus(oskar_Imager* h, int num, const int* ids,
        int* status);

/**
 * @brief
 * Sets the image (polarisation) type
 *
 * @details
 * Sets the polarisation made by the imager.
 *
 * The \p type string can be:
 * - "STOKES" for all four Stokes parameters.
 * - "I" for Stokes I only.
 * - "Q" for Stokes Q only.
 * - "U" for Stokes U only.
 * - "V" for Stokes V only.
 * - "LINEAR" for all four linear polarisations.
 * - "XX" for XX only.
 * - "XY" for XY only.
 * - "YX" for YX only.
 * - "YY" for YY only.
 * - "PSF" for the point spread function.
 *
 * @param[in,out] h          Handle to imager.
 * @param[in]     type       Image type; see description.
 * @param[in,out] status     Status return code.
 */
OSKAR_EXPORT
void oskar_imager_set_image_type(oskar_Imager* h, const char* type,
        int* status);

/**
 * @brief
 * Sets the convolution kernel used for gridding visibilities.
 *
 * @details
 * Sets the convolution kernel used for gridding visibilities.
 *
 * The \p type string can be:
 * - "Spheroidal" to use the spheroidal kernel from CASA.
 *
 * @param[in,out] h          Handle to imager.
 * @param[in]     type       Type of kernel to use.
 * @param[in]     support    Support size of kernel.
 * @param[in]     oversample Oversampling factor used for look-up table.
 * @param[in,out] status     Status return code.
 */
OSKAR_EXPORT
void oskar_imager_set_grid_kernel(oskar_Imager* h, const char* type,
        int support, int oversample, int* status);

/**
 * @brief
 * Sets the log to use for the imager.
 *
 * @details
 * Sets the log to use for the imager.
 *
 * @param[in,out] h          Handle to imager.
 * @param[in]     log        Handle to log.
 * @param[in,out] status     Status return code.
 */
OSKAR_EXPORT
void oskar_imager_set_log(oskar_Imager* h, oskar_Log* log);

/**
 * @brief
 * Sets the data column to use from a Measurement Set.
 *
 * @details
 * Sets the data column to use from a Measurement Set.
 *
 * @param[in,out] h          Handle to imager.
 * @param[in]     column     Name of the column to use.
 * @param[in,out] status     Status return code.
 */
OSKAR_EXPORT
void oskar_imager_set_ms_column(oskar_Imager* h, const char* column,
        int* status);

/**
 * @brief
 * Sets the root path of output images.
 *
 * @details
 * Sets the root path of output images.
 * FITS images files will be created with appropriate extensions.
 *
 * @param[in,out] h          Handle to imager.
 * @param[in]     filename   Root path.
 * @param[in,out] status     Status return code.
 */
OSKAR_EXPORT
void oskar_imager_set_output_root(oskar_Imager* h, const char* filename,
        int* status);

/**
 * @brief
 * Sets image side length.
 *
 * @details
 * Sets the image side length in pixels.
 *
 * @param[in,out] h          Handle to imager.
 * @param[in]     size       Image side length in pixels.
 */
OSKAR_EXPORT
void oskar_imager_set_size(oskar_Imager* h, int size);

/**
 * @brief
 * Sets the visibility time range used by the imager.
 *
 * @details
 * Sets the visibility time range used by the imager,
 * and whether time-synthesis should be used.
 *
 * @param[in,out] h          Handle to imager.
 * @param[in]     start      Start time index.
 * @param[in]     end        End time index (-1 for all times).
 * @param[in]     snapshots  If true, image each time slice separately;
 *                           if false, use time synthesis.
 */
OSKAR_EXPORT
void oskar_imager_set_time_range(oskar_Imager* h, int start, int end,
        int snapshots);

/**
 * @brief
 * Sets the visibility start frequency.
 *
 * @details
 * Sets the frequency of channel index 0, and the frequency increment.
 * This is required even if not applying phase rotation or channel selection,
 * to ensure the FITS headers are written correctly.
 *
 * @param[in,out] h       Handle to imager.
 * @param[in]     ref_hz  Frequency of index 0, in Hz.
 * @param[in]     inc_hz  Frequency increment, in Hz.
 * @param[in]     num     Number of channels in visibility data.
 * @param[in,out] status  Status return code.
 */
OSKAR_EXPORT
void oskar_imager_set_vis_frequency(oskar_Imager* h,
        double ref_hz, double inc_hz, int num, int* status);

/**
 * @brief
 * Sets the coordinates of the visibility phase centre.
 *
 * @details
 * Sets the coordinates of the visibility phase centre.
 * This is required even if not applying phase rotation, to ensure the
 * FITS headers are written correctly.
 *
 * @param[in,out] h          Handle to imager.
 * @param[in]     ra_deg     Right Ascension of phase centre, in degrees.
 * @param[in]     dec_deg    Declination of phase centre, in degrees.
 */
OSKAR_EXPORT
void oskar_imager_set_vis_phase_centre(oskar_Imager* h,
        double ra_deg, double dec_deg);

/**
 * @brief
 * Sets the visibility start time.
 *
 * @details
 * Sets the time of time index 0, and the time increment.
 * This is required even if not applying phase rotation or time selection,
 * to ensure the FITS headers are written correctly.
 *
 * @param[in,out] h            Handle to imager.
 * @param[in]     ref_mjd_utc  Time of index 0, as MJD(UTC).
 * @param[in]     inc_sec      Time increment, in seconds.
 * @param[in]     num          Number of time steps in visibility data.
 * @param[in,out] status       Status return code.
 */
OSKAR_EXPORT
void oskar_imager_set_vis_time(oskar_Imager* h,
        double ref_mjd_utc, double inc_sec, int num, int* status);

/**
 * @brief
 * Sets the number of W planes to use.
 *
 * @details
 * Sets the number of W planes, used only for W-projection.
 * A value of 0 or less means 'automatic'.
 *
 * @param[in,out] h            Handle to imager.
 * @param[in] value            Number of W planes to use.
 */
OSKAR_EXPORT
void oskar_imager_set_w_planes(oskar_Imager* h, int value);

/**
 * @brief
 * Sets the visibility W-coordinate range.
 *
 * @details
 * Sets the visibility W-coordinate range, used only for W-projection.
 * Note that the values must be in wavelengths.
 *
 * @param[in,out] h            Handle to imager.
 * @param[in] w_min            Minimum value of W, in wavelengths.
 * @param[in] w_max            Maximum value of W, in wavelengths.
 * @param[in] w_rms            RMS value of W, in wavelengths.
 */
OSKAR_EXPORT
void oskar_imager_set_w_range(oskar_Imager* h,
        double w_min, double w_max, double w_rms);

#ifdef __cplusplus
}
#endif

#endif /* OSKAR_IMAGER_SET_OPTIONS_H_ */
