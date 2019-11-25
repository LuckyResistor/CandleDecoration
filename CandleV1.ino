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


#include "Color.hpp"
#include "DS3231.hpp"

#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_DotStar.h>

#include <cstdint>


// Configuration
// --------------------------------------------------------------------------


/// The number of gPixels
///
const uint8_t cNumberOfPixels = 24;

/// The pin for the data output.
///
const uint8_t cDataPin = 4;


// Hardware Access.
// --------------------------------------------------------------------------


/// The global object to access the NeoPixels
///
Adafruit_NeoPixel gPixels = Adafruit_NeoPixel(cNumberOfPixels, cDataPin, NEO_GRBW + NEO_KHZ800);

/// Access to the dot star LED on the board.
///
Adafruit_DotStar gDotStar(1, 7, 8, DOTSTAR_BRG);


// Global Variables
// --------------------------------------------------------------------------


/// The base colors for the effect.
///
Color gBaseColors[cNumberOfPixels];

/// The blend colors to mix with the base colors.
///
Color gBlendColors[cNumberOfPixels];

/// The begin for random color blends.
///
Color gRandomBegin;

/// The end for random color blends.
///
Color gRandomEnd;

/// The current phase for the random effect.
///
uint8_t gRandomPhase = 0;

/// The current speed for the phase of the random effect..
///
uint8_t gRandomSpeed = 0;

/// The next point in time to check the RTC.
///
uint32_t gNextTimeCheck;

/// Flag if the decoration is currently enabled.
///
bool gIsEnabled = false;


// Functions
// --------------------------------------------------------------------------


/// Update the neopixels with the current colors and phase.
///
void updateNeoPixels()
{
    for (int i = 0; i < cNumberOfPixels; ++i) {
        const Color result = gBaseColors[i].mix(gBlendColors[i], gRandomPhase);
        gPixels.setPixelColor(i, result.getValue());
    }
    gPixels.show();
}


/// Disable all neopixels by setting them to black.
///
void disableNeoPixels()
{
    for (int i = 0; i < cNumberOfPixels; ++i) {
        gPixels.setPixelColor(i, 0);
    }    
    gPixels.show();
}


/// Fill the base colors with one solid color.
///
void fillWithColor(const Color &color)
{
    for (int i = 0; i < cNumberOfPixels; ++i) {
        gBaseColors[i] = color;
    }
}


/// A very simple pseudo random number generator.
/// 
uint8_t getSimpleRandom()
{
    static uint16_t seed = 70;
    seed = 181 * seed + 359;
    return (uint8_t)(seed >> 8);
}


/// Generate a new random blend with the current colors.
///
void generateNewRandomBlend()
{
    for (int i = 0; i < cNumberOfPixels; ++i) {
        gBlendColors[i] = gRandomBegin.mix(gRandomEnd, getSimpleRandom());
    }
    gRandomSpeed = (getSimpleRandom() >> 3) + 8;
}


/// Set the new random blend colors.
///
void setRandomBlendColors(Color a, Color b)
{
    gRandomBegin = a;
    gRandomEnd = b;
    gRandomPhase = 0;
    gRandomSpeed = 16;
    fillWithColor(Color());
    generateNewRandomBlend(); 
}


/// Check if the decoration should be on.
///
/// Modify this function to setup your on and off times for the decoration.
///
bool isOnTime()
{
    auto now = lr::DS3231::getDateTime();
    Serial.println(now.toString(lr::DateTime::Format::ISO));
    auto hour = now.getHour();
    
    return (hour >= 19) || (hour > 5 && hour < 8);
}


// Main methods
// --------------------------------------------------------------------------


/// Setup everything.
///
void setup()
{
    // Set the pin with the built-in LED as output.
    pinMode(LED_BUILTIN, OUTPUT);
    
    // Initialise the Wire library which is needed to communicate with the RTC.
    Wire.begin();
    
    // Wait for things to settle down.
    delay(1000);
    
    // Initialise the RTC driver.
    lr::DS3231::initialize();

    // Uncomment this line and set the current date/time to set the RTC once.
    // Make sure to comment it out and re-upload the firmware after setting the RTC!
    //lr::DS3231::setDateTime(lr::DateTime(2020,1,1,20,0,0));
    
    // Check if the RTC is running and had no time issues.
    if (!lr::DS3231::isRunning()) {
        // Do not start the decoration if the RTC does not has the correct time.
        while(1) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(200);
            digitalWrite(LED_BUILTIN, LOW);
            delay(400);
        }
    }

    // Set the dot star LED to black.
    gDotStar.begin();
    gDotStar.setPixelColor(0, 0);
    gDotStar.show();
    
    // Initialise the NeoPixels driver.
    gPixels.begin();
    gPixels.clear();
    gPixels.show();
    
    // Set the colors for the random effect.
    setRandomBlendColors(Color(0x6200), Color(0x0024));

    // Make the first time check after one second after start.
    gNextTimeCheck = millis() + 1000;
}


/// The main loop.
///
void loop()
{
    // Check the RTC every minute and enable/disable the decoration.
    if (static_cast<int32_t>(gNextTimeCheck - millis()) < 0) {
        gNextTimeCheck = millis() + 60000; // Check every minute.
        bool onTime = isOnTime();
        if (gIsEnabled != onTime) {
            gIsEnabled = onTime;
            if (!gIsEnabled) {
                disableNeoPixels();
            }
        }
    }        

    // If the decoration is enabled, produce a random effect.
    if (gIsEnabled) {
        updateNeoPixels();
        delay(50);
        const uint8_t oldPhase = gRandomPhase;
        gRandomPhase += gRandomSpeed;
        if (oldPhase > gRandomPhase) {
            for (uint8_t i = 0; i < cNumberOfPixels; ++i) {
                gBaseColors[i] = gBlendColors[i];
            }
            generateNewRandomBlend();
        }    
    }
}


