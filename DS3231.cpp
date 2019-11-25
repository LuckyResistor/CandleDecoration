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
#include "DS3231.hpp"


#include <Wire.h>


namespace lr {
namespace DS3231 {


/// @internal
/// A struct to store all time related registers in one block.
///
struct DateTimeRegister {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t dayOfWeek;
    uint8_t day;
    uint8_t month;
    uint8_t year;
};

/// @internal
/// A struct with the temperature registers.
///
struct TemperatureRegister {
    int8_t high;
    uint8_t low;
};


/// @internal
/// The chip address in the I2C bus.
///
static const uint8_t cChipAddress = 0x68;

/// @internal
/// The year base.
///
static uint16_t gYearBase;


/// @internal
/// Function to convert BCD format into binary format.
///
static inline uint8_t convertBcdToBin(const uint8_t bcd)
{
    return (bcd&0xf)+((bcd>>4)*10);
}


/// @internal
/// Function to convert binary to BCD format.
///
static inline uint8_t convertBinToBcd(const uint8_t bin)
{
    return (bin%10)+((bin/10)<<4);
}


uint8_t readRegister(Register reg)
{
    // Address the register
    Wire.beginTransmission(cChipAddress);
    Wire.write(static_cast<uint8_t>(reg));
    Wire.endTransmission();
    // Read one byte from the register.
    Wire.requestFrom(cChipAddress, 1u);
    const uint8_t data = Wire.read();
    return data;
}


void readRegister(Register reg, uint8_t *valueOut, uint8_t count)
{
    // Address the register
    Wire.beginTransmission(cChipAddress);
    Wire.write(static_cast<uint8_t>(reg));
    Wire.endTransmission();
    // Read the register values
    Wire.requestFrom(cChipAddress, count);
    for (uint8_t i = 0; i < count; ++i) {
        valueOut[i] = Wire.read();
    }
}


bool readFlag(Register reg, uint8_t bitMask)
{
    return (readRegister(reg) & bitMask) != 0;
}


void writeRegister(Register reg, uint8_t value)
{
    // Address the register and write the value
    Wire.beginTransmission(cChipAddress);
    Wire.write(static_cast<uint8_t>(reg));
    Wire.write(value);
    Wire.endTransmission();
}


void writeRegister(Register reg, const uint8_t *valueIn, uint8_t count)
{
    // Address the register and write the values
    Wire.beginTransmission(cChipAddress);
    Wire.write(static_cast<uint8_t>(reg));
    for (uint8_t i = 0; i < count; ++i) {
        Wire.write(valueIn[i]);
    }
    Wire.endTransmission();
}


void writeRegister(Register reg, uint8_t value, uint8_t mask)
{
    value &= mask; // Remove not maked bits.
    uint8_t data = readRegister(reg); // Read the old value.
    data &= (~mask); // Remove the masked bits.
    data |= value; // Add the new value.
    writeRegister(reg, data); // Write the combined value.
}


void setFlag(Register reg, uint8_t bitMask)
{
    uint8_t data = readRegister(reg); // Read the old value.
    data |= bitMask; // Set the flag.
    writeRegister(reg, data); // Write the combined value.
}


void clearFlag(Register reg, uint8_t bitMask)
{
    uint8_t data = readRegister(reg); // Read the old value.
    data &= (~bitMask); // Clear the flag
    writeRegister(reg, data); // Write the combined value.
}


void writeFlag(Register reg, uint8_t bitMask, bool enabled)
{
    if (enabled) {
        setFlag(reg, bitMask);
    } else {
        clearFlag(reg, bitMask);
    }
}


void initialize(uint16_t yearBase)
{
    gYearBase = yearBase;
}


DateTime getDateTime()
{
    // Use the struct to read all registers in one batch.
    DateTimeRegister data;
    readRegister(Register::Seconds, reinterpret_cast<uint8_t*>(&data), sizeof(DateTimeRegister));
    // Convert these values into a date object.
    return DateTime::fromUncheckedValues(
        static_cast<uint16_t>(convertBcdToBin(data.year))+((data.month&(1<<7))!=0?(gYearBase+100):gYearBase),
        convertBcdToBin(data.month&0x1f),
        convertBcdToBin(data.day&0x3f),
        convertBcdToBin(data.hours&0x3f),
        convertBcdToBin(data.minutes&0x7f),
        convertBcdToBin(data.seconds&0x7f),
        data.dayOfWeek&0x7);
}


void setDateTime(const DateTime &dateTime)
{
    // Basic year check
    const uint16_t newYear = dateTime.getYear();
    if (newYear < gYearBase || newYear >= (gYearBase+200)) {
        return; // Ignore this call.
    }
    // Use a struct to write all registers in one batch.
    DateTimeRegister data;
    // Prepare all values.
    data.seconds = convertBinToBcd(dateTime.getSecond());
    data.minutes = convertBinToBcd(dateTime.getMinute());
    data.hours = convertBinToBcd(dateTime.getHour());
    data.dayOfWeek = dateTime.getDayOfWeek();
    data.day = convertBinToBcd(dateTime.getDay());
    data.month = convertBinToBcd(dateTime.getMonth()) |
        (dateTime.getYear()>=(gYearBase+100)?(1<<7):0);
    data.year = convertBinToBcd(dateTime.getYear()%100);
    // Write all registers.
    writeRegister(Register::Seconds, reinterpret_cast<uint8_t*>(&data), sizeof(DateTimeRegister));
    // Enable the clock, start the oscillator and reset any status flags.
    writeRegister(Register::Control, 0b00000000u);
    writeRegister(Register::Status, 0);
}


bool isRunning()
{
    return !readFlag(Status::OSF);
}


float getTemperature()
{
    // Read the temperature.
    TemperatureRegister data;
    readRegister(Register::TemperatureHigh, reinterpret_cast<uint8_t*>(&data), sizeof(TemperatureRegister));
    // Create a float from this values.
    float result = static_cast<float>(data.high);
    const float fraction = static_cast<float>(data.low >> 6) * 0.25f;
    if (result < 0) {
        result -= fraction;
    } else {
        result += fraction;
    }
    return result;
}


void printAllRegisterValues()
{
    const uint8_t rtcRegisterCount = 0x13;
    uint8_t rtcRegister[rtcRegisterCount];
    readRegister(Register::Seconds, rtcRegister, rtcRegisterCount);
    for (uint8_t i = 0; i < rtcRegisterCount; ++i) {
        if (i < 0x10) {
           Serial.print('0');
        }
        Serial.print(i, HEX);
        Serial.print(':');
        const uint8_t value = rtcRegister[i];
        for (uint8_t j = 0; j < 8; ++j) {
            Serial.print(((value&(1<<(7-j)))!=0)?'1':'0');
        }
        Serial.print(':');
        if (value < 0x10) {
           Serial.print('0');
        }
        Serial.println(value, HEX);
    }
}


}
}
