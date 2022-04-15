/*
 * Copyright 2017, OYMotion Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 */

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "EMGFilters.h"

#define _DEBUG      0

#define SensorInputPin A0 // input pin number

// Define the `CALIBRATE` macro as 1 to calibrate the baseline value
// of input sEMG signals.
//
// After wiring the sEMG sensors to the Arduino board, wear the
// sEMG sensors. Relax your muscles for a few seconds, you
// will be able to see a series of squared sEMG signals values get printed on
// your serial terminal. Choose the maximal one as the baseline by setting
// the `baseline` variable.
//
// After calibriting, change the `CALIBRATE` macro to 0, and rebuild this
// project. The `envelope`, which is the squared sEMG signal data, will be
// printed to the serial line. The developer can plot it using the Arduino
// SerialPlotter.
//
// Note:
//      After calibration, Any squared value of sEMG sigal below the
//      baseline will be treated as zero.
//
//      It is recommended that you do calibration every time you wear
//      the sEMG sensor.
#define CALIBRATE 1

int baseline = 200;

EMGFilters myFilter;

// Set the input frequency.
//
// The filters work only with fixed sample frequency of
// `SAMPLE_FREQ_500HZ` or `SAMPLE_FREQ_1000HZ`.
// Inputs at other sample rates will bypass
SAMPLE_FREQUENCY sampleRate = SAMPLE_FREQ_1000HZ;

// Time interval for processing the input signal.
unsigned long long interval = 1000000ul / sampleRate;

// Set the frequency of power line hum to filter out.
//
// For countries with 60Hz power line, change to "NOTCH_FREQ_60HZ"
NOTCH_FREQUENCY humFreq = NOTCH_FREQ_50HZ;


void setup() {
    /* add setup code here */
    myFilter.init(sampleRate, humFreq, true, true, true);

    // open serial
    Serial.begin(115200);
}

void loop() {
    // Note: `micros()` will overflow and reset every about 70 minutes.
    unsigned long long timeStamp = micros();

    int data = analogRead(SensorInputPin);

    // filter processing
    long dataAfterFilter = myFilter.update(data);

    // Get envelope by squaring the input
    long envelope = sq(dataAfterFilter);

    if (CALIBRATE) {
        Serial.print("Squared Data: ");
        Serial.println(envelope);
    }
    else {
        // Any value below the `baseline` value will be treated as zero
        if (envelope < baseline) {
            dataAfterFilter = 0;
            envelope = 0;
        }
        // You may plot the data using Arduino SerialPlotter.
        Serial.print(envelope);
    }

    // Usually, you should still have (interval - timeElapsed) to do other work.
    // Otherwise, you would have to lower down the `sampleRate`.
    unsigned long long timeElapsed = micros() - timeStamp;
#if _DEBUG
    Serial.print("Filters cost time: ");
    Serial.println(timeElapsed);
#else
    delay((interval - timeElapsed) / 1000);
#endif
}
