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

#define _DEBUG 1

#define SensorInputPin A0 // input pin number

EMGFilters myFilter;

// Set the input frequency.
//
// The filters work only with fixed sample frequency of
// `SAMPLE_FREQ_500HZ` or `SAMPLE_FREQ_1000HZ`.
// Inputs at other sample rates will bypass
SAMPLE_FREQUENCY sampleRate = SAMPLE_FREQ_1000HZ;

// Set the frequency of power line hum to filter out.
//
// For countries with 60Hz power line, change to "NOTCH_FREQ_60HZ"
NOTCH_FREQUENCY humFreq = NOTCH_FREQ_50HZ;

// Calibrating the baseline sEMG signals.
//
// After wiring the sEMG sensors to the Arduino board, wear the
// sEMG sensors. Release your muscles for a few seconds, you
// will be able to see a series of sEMG signals get printed on
// your serial terminal. Choose the max one and squre it as the baseline.
// Any value under the baseline will be treated as zero.
//
// It is recommended that you do calibration every time you wear
// the sEMG sensor.
int baseline = 200;

// Time interval for processing the input signal.
unsigned long long interval = 1000000ul / sampleRate;

void setup() {
    /* add setup code here */
    myFilter.init(sampleRate, humFreq, true, true, true);

    // open serial
    Serial.begin(115200);

    // Setup time interval to
    interval = 1000000ul / sampleRate;
}

void loop() {
    // Note: `micros()` will overflow and reset every about 70 minutes.
    unsigned long long timeStamp = micros();

    int data = analogRead(SensorInputPin);

    // filter processing
    int dataAfterFilter = myFilter.update(data);

    int envlope = sq(dataAfterFilter);
    // Any value below the `baseline` value will be treated as zero
    envlope = (envlope > baseline) ? envlope : 0;

    timeStamp = micros() - timeStamp;
    if (_DEBUG) {
        Serial.print("Squared Data: ");
        Serial.println(envlope);
        // Serial.print("Filters cost time: "); Serial.println(timeStamp);
        // the filter cost average around 520 us
    }

    // Usually, you should still have (interval - timeStamp) to do other work.
    // Otherwise, it mean you would have to lower down the `sampleRate`.
    delay((interval - timeStamp) / 1000);
}
