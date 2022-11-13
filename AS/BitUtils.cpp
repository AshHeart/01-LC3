/**
 * Copyright (c) 2022, the AshHeart/01-LC3 developers.
 */

#pragma once

#include <cstdint>
#include "BitUtils.h"

namespace AS {

    uint16_t swap16(uint16_t x)
    {
        return (x << 8) | (x >> 8);
    }

    uint16_t sign_extend(uint16_t x, int bit_count)
    {
        if ((x >> (bit_count - 1)) & 1) {
            x |= (0XFFF << bit_count);
        }

        return x;
    }
}

