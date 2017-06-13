# EMG Filters

## Brief
This library provides the following filters for processing OYMotion's sEMG
signals:

1. an anti-hum notch filter to filter out 50HZ or 60HZ power line noise.
2. a lowpass filter to filter out noises above 150HZ.
3. a highpass filter to filter out noises below 20HZ.

You can turn on or off these filters by the init function.

**Note**:
Input frequencies of 500HZ and 1000HZ are supported only!

## Usage
The library is written in standard C++ without dependency of any other library,
so it should be easy to embed it into any other C++ project.

A simple sample code for using on Arduino is provided in the `example` folder.
