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

#ifndef OSKAR_SETTINGS_ELEMENT_PATTERN_H_
#define OSKAR_SETTINGS_ELEMENT_PATTERN_H_

/**
 * @file oskar_SettingsElementPattern.h
 */

#include "oskar_global.h"
#include "station/oskar_SettingsElementFit.h"
#include "station/oskar_SettingsElementTaper.h"

/**
 * @struct oskar_SettingsElementPattern
 *
 * @brief Structure to hold settings for the station element pattern evaluation.
 *
 * @details
 * The structure holds settings for the station element pattern evaluation.
 */
struct OSKAR_EXPORT oskar_SettingsElementPattern
{
    int enable_numerical_patterns;
    oskar_SettingsElementFit fit;
    int functional_type;
    oskar_SettingsElementTaper taper;
};
typedef struct oskar_SettingsElementPattern oskar_SettingsElementPattern;

#endif /* OSKAR_SETTINGS_ELEMENT_PATTERN_H_ */
