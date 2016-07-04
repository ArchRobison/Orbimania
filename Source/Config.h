/* Copyright 2016 Arch D. Robison

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 */

/******************************************************************************
 Configuration parameters for Orbimania
*******************************************************************************/

#pragma once
#ifndef Config_H
#define Config_H

#include <cstddef>

//! Maximum supported display width
/** Must be multiple of 4.
    Seem to get slightly better performance if it is not a multiple of 16 */
const int DISPLAY_WIDTH_MAX = 2560;

//! Minimum display width
const int DISPLAY_WIDTH_MIN = 1024;

//! Minimum display height
const int DISPLAY_HEIGHT_MIN = 768;

//! Log2 of size of a color lookup array for converting samples to colors.
const int CLUT_LG_SIZE = 10;

//! Size of a color lookup array for converting samples to colors.
const int CLUT_SIZE = 1<<CLUT_LG_SIZE;

//! Maximum number of particles
const std::size_t N_PARTICLE_MAX = 1000;

#endif /*Config_H*/
