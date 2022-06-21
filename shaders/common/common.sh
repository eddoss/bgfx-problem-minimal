/*
 * Copyright 2011-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "bgfx_shader.sh"
#include "shaderlib.sh"

float fit(float value, float omin, float omax, float nmin, float nmax)
{
    return (value - omin) / (omax - omin) * (nmax - nmin) + nmin;
}