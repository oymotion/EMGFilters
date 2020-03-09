/*
   Copyright 2019, OYMotion Inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
   AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
   THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
   DAMAGE.

*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "EMGFilters.h"


#define _DEBUG      0

#define SerialToUSB Serial


int SensorInputPins[] = {A0, A1};

#define INITIAL_DC_VAL 1000


#define ARR_SIZE(a) (sizeof(a) / sizeof(a[0]))

typedef struct
{
  uint8_t index;
  uint16_t buf[64]; /* ADC AC分量整流后的值，用于计算包络线 */
  uint32_t sum;
} CycleBuf_t;

/* Append to cycle buffer */
#define CYCLE_BUF_ADD(cb, val)                    \
  {                                               \
    cb.sum -= cb.buf[cb.index];                   \
    cb.sum += (val);                              \
    cb.buf[cb.index] = (val);                     \
    cb.index = (cb.index + 1) % ARR_SIZE(cb.buf); \
  }


/* Get mean value of cycle buffer */
#define CYCLE_BUF_MEAN(cb) (cb.sum / ARR_SIZE(cb.buf))



// IMPORTANT NOTE: Please unplug any power adapters when using sEMG sensors.
// Only battery powered systems an be connected directly/indirectly to sEMG
// sensors.
//
// Connect sensor to arduino board then compile & upload this sketch.
// When running, open SerilaPlot window (Menu->Tools->Serial Plotter) then
// you can see the curves.


EMGFilters myFilter[ARR_SIZE(SensorInputPins)];

// Set the input frequency.
//
// The filters work only with fixed sample frequency of
// `SAMPLE_FREQ_500HZ` or `SAMPLE_FREQ_1000HZ`.
// Inputs at other sample rates will bypass
SAMPLE_FREQUENCY sampleRate = SAMPLE_FREQ_500HZ;

// Time interval for processing the input signal. 
unsigned long long interval = 1000000ul / sampleRate;

// Set the frequency of power line hum to filter out.
//
// For countries with 60Hz power line, change to "NOTCH_FREQ_60HZ"
NOTCH_FREQUENCY humFreq = NOTCH_FREQ_50HZ;

CycleBuf_t rectifiedAcBuf[ARR_SIZE(SensorInputPins)];


void setup() {
  /* add setup code here */
  
  //initialization
  for (int i = 0; i < ARR_SIZE(SensorInputPins); i++) {
    myFilter[i].init(sampleRate, humFreq, true, true, true);
    
    rectifiedAcBuf[i].sum = 0;
    rectifiedAcBuf[i].index = 0;

    for (int j = 0; j < ARR_SIZE(rectifiedAcBuf[i].buf); j++)
    {
      rectifiedAcBuf[i].buf[j] = 0;
    }
  }

  // open serial
  SerialToUSB.begin(115200);
}


void loop() {
  // Note: `micros()` will overflow and reset every about 70 minutes.
  unsigned long long timeStamp = micros();

  // filter processing
  int data = 0, dataAfterFilter = 0;

  for (int i = 0; i <  ARR_SIZE(SensorInputPins); i++) {
    data = analogRead(SensorInputPins[i]);
    dataAfterFilter = myFilter[i].update(data);

    // Rectification
    CYCLE_BUF_ADD(rectifiedAcBuf[i], abs(dataAfterFilter) * 4);

    uint16_t envelope = CYCLE_BUF_MEAN(rectifiedAcBuf[i]);

#if !_DEBUG
    SerialToUSB.print(128 + dataAfterFilter); // offset = 128
    SerialToUSB.print(" ");
    SerialToUSB.print(envelope);
    SerialToUSB.print(" ");
#endif
  }
  
  unsigned long timeElapsed = micros() - timeStamp;


#if _DEBUG
  SerialToUSB.print("Filters cost time: ");
  SerialToUSB.println(timeElapsed);
#else
  if (interval > timeElapsed){
    delay((interval - timeElapsed) / 1000);
  }
#endif

  SerialToUSB.println();
}
