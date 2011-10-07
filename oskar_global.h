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

#ifndef OSKAR_GLOBAL_H_
#define OSKAR_GLOBAL_H_

/**
 * @macro OSKAR_VERSION
 *
 * @brief Macro used to determine the version of OSKAR.
 *
 * @details
 * This macro expands to a numeric value of the form 0xMMNNPP (MM = major,
 * NN = minor, PP = patch) that specifies the version number of the OSKAR
 * library. For example, in OSKAR version 2.1.3 this would expand to
 * 0x020103.
 */
#define OSKAR_VERSION 0x015000

/**
 * @macro OSKAR_VERSION_STR
 *
 * @brief Macro used to return the version of OSKAR as a text string.
 *
 * @details
 * This macro expands to a string that specifies the OSKAR version number
 * (for example, "2.1.3").
 */
#define OSKAR_VERSION_STR "1.50.0_pre-alpha"

/**
 * @macro OSKAR_EXPORT
 *
 * @brief
 * Macro used to export public functions.
 *
 * @details
 * Macro used for creating the Windows library.
 * Note: should only be needed in header files.
 *
 * The __declspec(dllexport) modifier enables the method to
 * be exported by the DLL so that it can be used by other applications.
 *
 * Usage examples:
 *   OSKAR_EXPORT void foo();
 *   static OSKAR_EXPORT double add(double a, double b);
 *
 * For more information see:
 *   http://msdn.microsoft.com/en-us/library/a90k134d(v=VS.90).aspx
 */
#if (defined(_WIN32) || defined(__WIN32__))
    #ifdef oskar_EXPORTS
        #ifndef OSKAR_EXPORT
            #define OSKAR_EXPORT __declspec(dllexport)
        #endif
    #else
        #ifndef OSKAR_EXPORT
            #define OSKAR_EXPORT
        #endif
    #endif
#else
    #ifndef OSKAR_EXPORT
        #define OSKAR_EXPORT
    #endif
#endif

// Macros used to prevent Eclipse from complaining about unknown CUDA syntax.
#ifdef __CDT_PARSER__
    #define __global__
    #define __device__
    #define __host__
    #define __shared__
    #define __constant__
    #define __forceinline__
    #define OSKAR_CUDAK_CONF(...)
#else
    #define OSKAR_CUDAK_CONF(...) <<< __VA_ARGS__ >>>
#endif

#endif // OSKAR_GLOBAL_H_