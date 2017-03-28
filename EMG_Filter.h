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

// EMG_Filter.h
// Author: matthew gan
// Data: 20170326
/* Function Description: 
*	the EMG filter have a anti-hum filter in order to remove 50Hz or 60Hz interference from power supplier;
*	the EMG filter have a lowpass filter to get rid of noise above 150Hz;
*	the EMG filter have a highpass filter to get rid of noise below 20Hz;
*	you can turn off these filters by configuration function.
*/
#ifndef _EMG_FILTER_h
#define _EMG_FILTER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

enum FILTER_SWITCH
{
	FILTER_OFF =0,
	FILTER_ON,
};

#define	NOTCH_FREQ_50HZ		50
#define NOTCH_FREQ_60HZ		60
#define SAMPLE_FREQ_500HZ	500
#define	SAMPLE_FREQ_1000HZ	1000

class EMG_FILTER
{
public:
	//call in the setup to initialize all the filters
	//CAUTION:
	//INPUT sampleFreq only support SAMPLE_FREQ_500HZ & SAMPLE_FREQ_1000HZ
	//INPUT notchFreq only support NOTCH_FREQ_50HZ & NOTCH_FREQ_60HZ
	//otherwise the filter wont work, output = input
	void begin(int sampleFreq, int notchFreq);

	//by default, all the filter is set to FILTER_ON state when initialization.
	void config(FILTER_SWITCH notchFilter, FILTER_SWITCH lowpassfilter, FILTER_SWITCH highpassfilter);

	//call in the loop, input read analog value to get filtered value
	int update(int inputValue);

private:
	bool _bypassEnalbe;
	int _sampleFreq;
	int _notchFreq;
	bool _notchFilterEnable;
	bool _lowPassFilterEnable;
	bool _highPassFilterEnable;
};

#endif

