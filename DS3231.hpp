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


#include "DateTime.hpp"


/// @namespace lr::DS3231
///
/// A simple driver for the DS3231 real time clock.
///
/// @note This is an old implementation for the Arduino environment.
/// See https://github.com/LuckyResistor/HAL-ds3231 for the latest implementation.


namespace lr {
namespace DS3231 {


/// Initialize the real time clock driver.
///
/// @param yearBase The year base which is used for the RTC. The RTC stores the year
///    only with two digits, plus one additional bit for the next century. If you set
//     the year base to `2000`, the RTC will hold the correct time for 200 years,
///    starting from `2000-01-01 00:00:00`.
///
void initialize(uint16_t yearBase = 2000);

/// Get the current date/time.
///
DateTime getDateTime();

/// Set the date/time.
///
void setDateTime(const DateTime &dateTime);

/// Check if the RTC is running.
///
bool isRunning();

/// Get the temperature in degrees celsius.
///
float getTemperature();

/// @name Low Level Functions
/// Low level functions to directly access all registers of the chip or
/// to print useful information for debugging.
/// @{

/// Print the status of all register values.
///
void printAllRegisterValues();

/// All registers available in the chip.
///
enum class Register : uint8_t {
    Seconds = 0x00,
    Minutes = 0x01,
    Hours = 0x02,
    DayOfWeek = 0x03,
    Day = 0x04,
    MonthCentury = 0x05,
    Year = 0x06,
    Alarm1Seconds = 0x07,
    Alarm1Minutes = 0x08,
    Alarm1Hours = 0x09,
    Alarm1DayDate = 0x0a,
    Alarm2Minutes = 0x0b,
    Alarm2Hours = 0x0c,
    Alarm2DayDate = 0x0d,
    Control = 0x0e,
    Status = 0x0f,
    AgingOffset = 0x10,
    TemperatureHigh = 0x11,
    TemperatureLow = 0x12
};

/// All flags for the control register
///
enum class Control : uint8_t {
    A1IE = (1<<0),
    A2IE = (1<<1),
    INTCN = (1<<2),
    RS1 = (1<<3),
    RS2 = (1<<4),
    CONV = (1<<5),
    BBSQW = (1<<6),
    EOSC = (1<<7)
};

/// All flags for the status register
///
enum class Status : uint8_t {
    A1F = (1<<0),
    A2F = (1<<1),
    BSY = (1<<2),
    EN32kHz = (1<<3),
    OSF = (1<<7)
};

/// Read a single register from the chip.
///
/// @param reg The register to read.
/// @return The value from the register.
///
uint8_t readRegister(Register reg);

/// Read multiple registers from the chip.
///
/// @param reg The first register to read.
/// @param valueOut An array of bytes to write the register values to.
/// @param count The number of registers to read.
///
void readRegister(Register reg, uint8_t *valueOut, uint8_t count);

/// Read a flag from a register.
///
/// @param reg The register for the flag.
/// @param bitMask The bit mask for the flag.
/// @return true if the flag is set.
///
bool readFlag(Register reg, uint8_t bitMask);

/// Write a single register to the chip.
///
/// @param reg The register to write into.
/// @param value The new value.
///
void writeRegister(Register reg, uint8_t value);

/// Write a multiple register values to the chip.
///
/// @param reg The start register for the write.
/// @param valueIn A pointer to the array of values to write.
/// @param count The number of registers to write.
///
void writeRegister(Register reg, const uint8_t *valueIn, uint8_t count);

/// Write a few bits in a single register.
///
/// This reads the register first, masks the result and writes the register.
///
/// @param reg The register to write into.
/// @param value The new value.
/// @param mask The mask. Each 1 bit in the mask set is written.
///
void writeRegister(Register reg, uint8_t value, uint8_t mask);

/// Write a flag to a register.
///
/// This will only do a write to the chip if the flag changes.
///
/// @param reg The register to change.
/// @param bitMask The bit mask for the flag.
/// @param enabled If the flag should be set `true` or cleared `false`.
///
void writeFlag(Register reg, uint8_t bitMask, bool enabled);

/// Set a flag in a register.
///
/// @param reg The register to change.
/// @param bitMask The bit mask for the flag.
///
void setFlag(Register reg, uint8_t bitMask);

/// Clear a flag in a register.
///
/// @param reg The register to change.
/// @param bitMask The bit mask for the flag.
///
void clearFlag(Register reg, uint8_t bitMask);


// Helper methods to simplify the code.
inline bool readFlag(Control flag) { return readFlag(Register::Control, static_cast<uint8_t>(flag)); }
inline bool readFlag(Status flag) { return readFlag(Register::Status, static_cast<uint8_t>(flag)); }
inline void setFlag(Control flag) { setFlag(Register::Control, static_cast<uint8_t>(flag)); }
inline void setFlag(Status flag) { setFlag(Register::Status, static_cast<uint8_t>(flag)); }
inline void clearFlag(Control flag) { clearFlag(Register::Control, static_cast<uint8_t>(flag)); }
inline void clearFlag(Status flag) { clearFlag(Register::Status, static_cast<uint8_t>(flag)); }
inline void writeFlag(Control flag, bool enabled) { writeFlag(Register::Control, static_cast<uint8_t>(flag), enabled); }
inline void writeFlag(Status flag, bool enabled) { writeFlag(Register::Status, static_cast<uint8_t>(flag), enabled); }


/// @}


}
}
