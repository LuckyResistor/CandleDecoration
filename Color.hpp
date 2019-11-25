#pragma once
//
// (c)2019 by Lucky Resistor. See LICENSE for details.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//


#include <Arduino.h>

#include <cstdint>


/// A color class to simplify calculations.
///
class Color
{
public:
    /// Create black color.
    ///
    inline Color()
        : r(0), g(0), b(0), w(0) {      
    }
    
    /// Create a color with the given RGBW values.
    ///
    inline Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
        : r(r), g(g), b(b), w(w) {
    }
    
    /// Create a color with the given 16bit value. WBGR
    ///
    inline Color(uint16_t value) {
        r = ((value & 0x000f) >> 0) * 0x10;
        g = ((value & 0x00f0) >> 4) * 0x10;
        b = ((value & 0x0f00) >> 8) * 0x10;
        w = ((value & 0xf000) >> 12) * 0x10;
    }
    
    /// Mix two colors
    ///
    inline Color mix(const Color &other, uint8_t shift) const {
        const uint16_t aShift = 0x100-(uint16_t)shift;
        const uint16_t bShift = shift;
        const uint8_t newR = (((uint16_t)r * aShift) + ((uint16_t)other.r * bShift))/0x100;
        const uint8_t newG = (((uint16_t)g * aShift) + ((uint16_t)other.g * bShift))/0x100;
        const uint8_t newB = (((uint16_t)b * aShift) + ((uint16_t)other.b * bShift))/0x100;
        const uint8_t newW = (((uint16_t)w * aShift) + ((uint16_t)other.w * bShift))/0x100;
        return Color(newR, newG, newB, newW);
    }
    
    /// Set the brightness
    ///
    inline Color dim(uint8_t level) const {
        const uint16_t level16 = (uint16_t)level+1;
        const uint8_t newR = ((uint16_t)r * level16)/0x100;
        const uint8_t newG = ((uint16_t)g * level16)/0x100;
        const uint8_t newB = ((uint16_t)b * level16)/0x100;
        const uint8_t newW = ((uint16_t)w * level16)/0x100;
        return Color(newR, newG, newB, newW);
    }
        
    /// Calculate the 32bit value for the NeoPixel library.
    ///
    inline uint32_t getValue() const {
        uint32_t result = 0;
        result |= (uint32_t)cGamma[b];
        result |= (uint32_t)cGamma[g] << 8;
        result |= (uint32_t)cGamma[r] << 16;
        result |= (uint32_t)cGamma[w] << 24;
        return result;
    }

public:    
    /// Get a color value. color = 0-191
    ///
    inline static Color wheel(uint8_t color, uint8_t white) {
        if (color < 64) {
            const uint8_t aShift = 63-(uint16_t)color;
            const uint8_t bShift = color;
            const uint8_t newR = aShift * 4;
            const uint8_t newG = bShift * 4;
            const uint8_t newB = 0;
            return Color(newR, newG, newB, white);
        } else if (color < 128) {
            const uint8_t aShift = 63-(uint16_t)(color - 64);
            const uint8_t bShift = (color - 64);
            const uint8_t newR = 0;
            const uint8_t newG = aShift * 4;
            const uint8_t newB = bShift * 4;
            return Color(newR, newG, newB, white);          
        } else {
            const uint8_t aShift = 63-(uint16_t)(color - 128);
            const uint8_t bShift = (color - 128);
            const uint8_t newR = bShift * 4;
            const uint8_t newG = 0;
            const uint8_t newB = aShift * 4;
            return Color(newR, newG, newB, white);
        }
    }

public:
    /// Gamme correction table.
    ///
    static const uint8_t cGamma[];
    
public:
    uint8_t r; ///< The red amount.
    uint8_t g; ///< The green amount.
    uint8_t b; ///< The blue amount.
    uint8_t w; ///< The white amount.
};


