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

#ifndef OSKAR_CUDAK_DFTW_2D_SEQ_IN_H_
#define OSKAR_CUDAK_DFTW_2D_SEQ_IN_H_

/**
 * @file oskar_cudak_dftw_2d_seq_in.h
 */

#ifdef __CDT_PARSER__
    #define __global__
    #define __device__
    #define __shared__
    #define __constant__
#endif

/**
 * @brief
 * CUDA kernel to generate un-normalised 2D DFT weights (single precision).
 *
 * @details
 * This CUDA kernel produces the complex 2D DFT weights for the
 * given inputs and outputs, and stores them in device memory.
 *
 * Each thread generates the complex weight for a single input and a single
 * output.
 *
 * The kernel requires (blockDim.x + blockDim.y) * sizeof(float2)
 * bytes of shared memory.
 *
 * @param[in] n_in     Number of input points.
 * @param[in] x_in     Array of input x positions.
 * @param[in] y_in     Array of input y positions.
 * @param[in] n_out    Number of output points.
 * @param[in] x_out    Array of output 1/x positions.
 * @param[in] y_out    Array of output 1/y positions.
 * @param[out] weights Matrix of complex DFT weights (n_in columns, n_out rows).
 */
__global__
void oskar_cudakf_dftw_2d_seq_in(const int n_in, const float* x_in,
        const float* y_in, const int n_out, const float* x_out,
        const float* y_out, float2* weights);

/**
 * @brief
 * CUDA kernel to generate un-normalised 2D DFT weights (double precision).
 *
 * @details
 * This CUDA kernel produces the complex 2D DFT weights for the
 * given inputs and outputs, and stores them in device memory.
 *
 * Each thread generates the complex weight for a single input and a single
 * output.
 *
 * The kernel requires (blockDim.x + blockDim.y) * sizeof(double2)
 * bytes of shared memory.
 *
 * @param[in] n_in     Number of input points.
 * @param[in] x_in     Array of input x positions.
 * @param[in] y_in     Array of input y positions.
 * @param[in] n_out    Number of output points.
 * @param[in] x_out    Array of output 1/x positions.
 * @param[in] y_out    Array of output 1/y positions.
 * @param[out] weights Matrix of complex DFT weights (n_in columns, n_out rows).
 */
__global__
void oskar_cudakd_dftw_2d_seq_in(const int n_in, const double* x_in,
        const double* y_in, const int n_out, const double* x_out,
        const double* y_out, double2* weights);

#endif // OSKAR_CUDAK_DFTW_2D_SEQ_IN_H_