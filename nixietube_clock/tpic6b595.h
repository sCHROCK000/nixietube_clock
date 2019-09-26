#pragma once
#include <Arduino.h>

class tpic6b595
{
public:
    tpic6b595(int dataPin, int sckPin, int rckPin, int clearPin);
    void write(byte bits);
    void begin();
    void clear();

private:
    int _dataPin;
    int _sckPin;
    int _rckPin;
    int _clearPin;
    void genCLK(int pin, int active);
};