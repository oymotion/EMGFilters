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

// Author: matthew gan
// Data: 20170326
// 

#include "EMG_Filter.h"

//#define	NOTCH_FREQ_50HZ		50
//#define	NOTCH_FREQ_60HZ		60
//#define	SAMPLE_FREQ_500HZ	500
//#define	SAMPLE_FREQ_1000HZ	1000

//coefficients of transfer function of LPF
//coef[sampleFreqInd][order]
static float lpf_numerator_coef[2][3] = { { 0.3913, 0.7827, 0.3913 },{ 0.1311,0.2622,0.1311 } };
static float lpf_denominator_coef[2][3] = { { 1.0000,0.3695,0.1958 },{ 1.0000,-0.7478,0.2722 } };
//coefficients of transfer function of HPF
static float hpf_numerator_coef[2][3] = { {0.8371,-1.6742,0.8371},{ 0.9150,-1.8299,0.9150 } };
static float hpf_denominator_coef[2][3] = { { 1.0000,-1.6475,0.7009 },{ 1.0000,-1.8227,0.8372 } };
//coefficients of transfer function of anti-hum filter
//coef[sampleFreqInd][order] for 50Hz
static float ahf_numerator_coef_50Hz[2][6] = { {0.9522, -1.5407, 0.9522, 0.8158, -0.8045, 0.0855  },{ 0.5869, -1.1146, 0.5869, 1.0499, -2.0000, 1.0499 } };
static float ahf_denominator_coef_50Hz[2][6] = { {1.0000, -1.5395, 0.9056, 1.0000 -1.1187, 0.3129  },{ 1.0000, -1.8844, 0.9893, 1.0000, -1.8991, 0.9892 } };
static float ahf_output_gain_coef_50Hz[2] = {1.3422, 1.4399};
//coef[sampleFreqInd][order] for 60Hz
static float ahf_numerator_coef_60Hz[2][6] = { {0.9528, -1.3891, 0.9528, 0.8272, -0.7225, 0.0264  },{0.5824, -1.0810, 0.5824, 1.0736, -2.0000, 1.0736 } };
static float ahf_denominator_coef_60Hz[2][6] = { {1.0000, -1.3880, 0.9066, 1.0000, -0.9739, 0.2371  },{1.0000, -1.8407, 0.9894, 1.0000, -1.8584, 0.9891 } };
static float ahf_output_gain_coef_60Hz[2] = {1.3430, 1.4206  };

enum FILTER_TYPE
{
	FILTER_TYPE_LOWPASS = 0,
	FILTER_TYPE_HIGHPASS,
};

class FILTER_2nd
{
private:
	//second-order filter
	float states[2];
	float num[3];
	float den[3];
public:
	void init(FILTER_TYPE ftype, int sampleFreq)
	{
		states[0] = 0;
		states[1] = 0;
		if (ftype == FILTER_TYPE_LOWPASS)
		{
			//2th order butterworth lowpass filter
			//cutoff frequency 150Hz
			if (sampleFreq == SAMPLE_FREQ_500HZ)
			{
				for (int i = 0; i < 3; i++)
				{
					num[i] = lpf_numerator_coef[0][i];
					den[i] = lpf_denominator_coef[0][i];
				}
			}
			else if (sampleFreq == SAMPLE_FREQ_1000HZ)
			{
				for (int i = 0; i < 3; i++)
				{
					num[i] = lpf_numerator_coef[1][i];
					den[i] = lpf_denominator_coef[1][i];
				}
			}			
		}
		else if (ftype == FILTER_TYPE_HIGHPASS)
		{
			//2th order butterworth
			//cutoff frequency 20Hz
			if (sampleFreq == SAMPLE_FREQ_500HZ)
			{
				for (int i = 0; i < 3; i++)
				{
					num[i] = hpf_numerator_coef[0][i];
					den[i] = hpf_denominator_coef[0][i];
				}
			}
			else if (sampleFreq == SAMPLE_FREQ_1000HZ)
			{
				for (int i = 0; i < 3; i++)
				{
					num[i] = hpf_numerator_coef[1][i];
					den[i] = hpf_denominator_coef[1][i];
				}
			}
		}
	}

	float update(float input)
	{
		float tmp = (input - den[1] * states[0] - den[2] * states[1]) / den[0];
		float output = num[0] * tmp + num[1] * states[0] + num[2] * states[1];
		//save last states
		states[1] = states[0];
		states[0] = tmp;
		return output;
	}
};

class FILTER_4th
{
private:
	//fourth-order filter
	//cascade two 2nd-order filters
	float states[4];
	float num[6];
	float den[6];
	float gain;
public:
	void init(int sampleFreq, int humFreq)
	{
		gain = 0;
		for (int i = 0; i < 4; i++)
		{
			states[i] = 0;
		}
		if (humFreq == NOTCH_FREQ_50HZ)
		{
			if (sampleFreq == SAMPLE_FREQ_500HZ)
			{
				for (int i = 0; i < 6; i++)
				{
					num[i] = ahf_numerator_coef_50Hz[0][i];
					den[i] = ahf_denominator_coef_50Hz[0][i];
				}
				gain = ahf_output_gain_coef_50Hz[0];
			}
			else if (sampleFreq == SAMPLE_FREQ_1000HZ)
			{
				for (int i = 0; i < 6; i++)
				{
					num[i] = ahf_numerator_coef_50Hz[1][i];
					den[i] = ahf_denominator_coef_50Hz[1][i];
				}
				gain = ahf_output_gain_coef_50Hz[1];
			}
		}
		else if (humFreq == NOTCH_FREQ_60HZ)
		{
			if (sampleFreq == SAMPLE_FREQ_500HZ)
			{
				for (int i = 0; i < 6; i++)
				{
					num[i] = ahf_numerator_coef_60Hz[0][i];
					den[i] = ahf_denominator_coef_60Hz[0][i];
				}
				gain = ahf_output_gain_coef_60Hz[0];
			}
			else if (sampleFreq == SAMPLE_FREQ_1000HZ)
			{
				for (int i = 0; i < 6; i++)
				{
					num[i] = ahf_numerator_coef_60Hz[1][i];
					den[i] = ahf_denominator_coef_60Hz[1][i];
				}
				gain = ahf_output_gain_coef_60Hz[1];
			}
		}
	}

	float update(float input)
	{
		float output;
		float stageIn;
		float stageOut;

		stageOut = num[0] * input + states[0];
		states[0] = (num[1] * input + states[1]) - den[1] * stageOut;
		states[1] = num[2] * input - den[2] * stageOut;
		stageIn = stageOut;
		stageOut = num[3] * stageOut + states[2];
		states[2] = (num[4] * stageIn + states[3]) - den[4] * stageOut;
		states[3] = num[5] * stageIn - den[5] * stageOut;

		output = gain * stageOut;

		return output;
	}
};

FILTER_2nd LPF;
FILTER_2nd HPF;
FILTER_4th AHF;

void EMG_FILTER::begin(int sampleFreq, int notchFreq)
{
	_sampleFreq = sampleFreq;
	_notchFreq = notchFreq;
	_bypassEnalbe = true;
	if ((sampleFreq == SAMPLE_FREQ_500HZ) || (sampleFreq == SAMPLE_FREQ_1000HZ))
	{
		if ((notchFreq == NOTCH_FREQ_50HZ) || (notchFreq == NOTCH_FREQ_50HZ))
		{
			_bypassEnalbe = false;
		}
	}

	_notchFilterEnable = true;
	_lowPassFilterEnable = true;
	_highPassFilterEnable = true;

	LPF.init(FILTER_TYPE_LOWPASS, _sampleFreq);
	HPF.init(FILTER_TYPE_HIGHPASS, _sampleFreq);
	AHF.init(_sampleFreq, _notchFreq);
}

void EMG_FILTER::config(FILTER_SWITCH notchFilter, FILTER_SWITCH lowpassfilter, FILTER_SWITCH highpassfilter)
{
	_notchFilterEnable = (notchFilter == FILTER_ON);
	_lowPassFilterEnable = (lowpassfilter == FILTER_ON);
	_highPassFilterEnable = (highpassfilter == FILTER_ON);
}

int EMG_FILTER::update(int inputValue)
{
	int output = 0;
	if (_bypassEnalbe)
	{
		return output = inputValue;
	}

	//first notch filter
	if (_notchFilterEnable)
	{
		//output = NTF.update(inputValue);
		output = AHF.update(inputValue);
	}
	else
	{
		//notch filter bypass
		output = inputValue;
	}

	//second low pass filter
	if (_lowPassFilterEnable)
	{
		output = LPF.update(output);
	}

	//third high pass filter
	if(_highPassFilterEnable)
	{
		output = HPF.update(output);
	}

	return output;
}
