# EMG filter

## Brief
This library provides an anti-hum notch filter to filter out 50HZ or 60HZ power
line noise, a lowpass filter to filter out signals above 150HZ, and a highpass
filter to filter out noise below 20HZ.

You can turn on or off these filters by the init function.

**Note**:
Input frequencies of 500HZ and 1000HZ are supported only!

## Revision history

## Usage
The library is written in standard C++ without dependency of any other library,
so it can be embedded into any other C++ project.

A simple sample code for using on Arduino is provided in the `example` folder.
