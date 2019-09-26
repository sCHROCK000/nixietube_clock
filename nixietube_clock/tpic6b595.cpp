#include "tpic6b595.h"

tpic6b595::tpic6b595(int dataPin, int sckPin, int rckPin, int clearPin) {
    _dataPin = dataPin;
    _sckPin = sckPin;
    _rckPin = rckPin;
    _clearPin = clearPin;
}

void tpic6b595::begin(void) {
    pinMode(_dataPin, OUTPUT);
    pinMode(_sckPin, OUTPUT);
    pinMode(_rckPin, OUTPUT);
    pinMode(_clearPin, OUTPUT);

    digitalWrite(_clearPin, !LOW);
    delay(2);
    digitalWrite(_clearPin, !HIGH);
    digitalWrite(_dataPin, !HIGH);
    digitalWrite(_sckPin, !HIGH);
    digitalWrite(_rckPin, !HIGH);

}

void tpic6b595::write(byte data) {
    shiftOut(_dataPin, _sckPin, MSBFIRST, ~data);
    genCLK(_rckPin, !HIGH);
}

void tpic6b595::clear() {
    digitalWrite(_clearPin, !LOW);
    delay(2);
    digitalWrite(_clearPin, !HIGH);
}

void tpic6b595::genCLK(int pin, int active) {
    if (active == HIGH)
    {
        digitalWrite(pin, !HIGH);
        delayMicroseconds(20);
        digitalWrite(pin, !LOW);
    }
    else
    {
        digitalWrite(pin, !LOW);
        delayMicroseconds(20);
        digitalWrite(pin, !HIGH);
    }
}