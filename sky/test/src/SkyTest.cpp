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

#include "sky/test/SkyTest.h"
#include "sky/generate_random_sources.h"

#include <vector>
#include <cstdlib>
using namespace std;


/**
 * @details
 * Sets up the context before running each test method.
 */
void SkyTest::setUp()
{
}

/**
 * @details
 * Clean up routine called after each test is run.
 */
void SkyTest::tearDown()
{
}


/**
 * @details
 * Test of generating a random number in the range 0.0 to 1.0
 */
void SkyTest::test_rand()
{
    const unsigned n = 10;
    const unsigned seed = 0;
    srand(seed);
    for (unsigned i = 0; i < n; ++i)
    {
        const double r = (double)rand() / (double)RAND_MAX;
        //CPPUNIT_ASSERT_DOUBLES_EQUAL()
    }
}


/**
 * @details
 */
void SkyTest::test_method()
{
    const unsigned num_sources = 20;
    const double inner_radius = 1.0;
    const double outer_radius = 180.0;

    const double brightness_min = 1.0e-2;
    const double brightness_max = 1.0e4;
    const double distribution_power = -2.0;
    vector<double> ra(num_sources);
    vector<double> dec(num_sources);
    vector<double> brightness(num_sources);

    generate_random_sources(num_sources, inner_radius, outer_radius,
            brightness_min, brightness_max, distribution_power,
            &ra[0], &dec[0], &brightness[0]);
}