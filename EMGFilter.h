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

#ifndef _EMGFILTER_H
#define _EMGFILTER_H

enum NOTCH_FREQUENCY { NOTCH_FREQ_50HZ = 50, NOTCH_FREQ_60HZ = 60 };

enum SAMPLE_FREQUENCY { SAMPLE_FREQ_500HZ = 500, SAMPLE_FREQ_1000HZ = 1000 };

// \brief EMGFilter provides an anti-hum notch filter to filter out 50HZ or
//        60HZ power line noise, a lowpass filter to filter out signals above
//        150HZ, and a highpass filter to filter out noise below 20HZ;
//        You can turn on or off these filters by the init function.
// \remark Input frequencies of 500HZ and 1000HZ are supported only!
class EMGFilter {
  public:
    // \brief Initializes the filter.
    // \param sampleFreq only supports SAMPLE_FREQ_500HZ &
    // SAMPLE_FREQ_1000HZ.
    // \param notchFreq only supports NOTCH_FREQ_50HZ & NOTCH_FREQ_60HZ.
    // \param enableNotchFilter enables the notch filter. True by default.
    // \param enableLowpassFilter enables the lowpass filter. True by
    // default.
    // \param enableHighpassFilter enables the highpass filter. True by
    // default
    // \remark The filter won't work and just bypass input if sampleFreq or
    //         notchFreq isn't specified properly.
    void init(SAMPLE_FREQUENCY sampleFreq,
              NOTCH_FREQUENCY  notchFreq,
              bool             enableNotchFilter    = true,
              bool             enableLowpassFilter  = true,
              bool             enableHighpassFilter = true);

    // \brief Called in the loop, input read analog value to get filtered
    // value
    int update(int inputValue);

  private:
    SAMPLE_FREQUENCY m_sampleFreq;
    NOTCH_FREQUENCY  m_notchFreq;
    bool             m_bypassEnalbe;
    bool             m_notchFilterEnable;
    bool             m_lowpassFilterEnable;
    bool             m_highpassFilterEnable;
};

#endif
