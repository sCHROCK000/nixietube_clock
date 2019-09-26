//#include <Arduino.h>
#include "tpic6b595.h"
#include <Wire.h>
#include <RtcDS3231.h>

#define countnum(a) (sizeof(a) / sizeof(a[0]))

#define ENABLE_HV   1

#define     CS      27
#define     DATA    26
#define     SCK     14
#define     RCK     12
#define     LED     2
#define     EN      13

#define     SCL     22
#define     SDA     21

hw_timer_t * timer0 = NULL;
hw_timer_t * timer1 = NULL;
portMUX_TYPE timerMux0 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux1 = portMUX_INITIALIZER_UNLOCKED;

tpic6b595 tpic(DATA, SCK, RCK, CS);
RtcDS3231<TwoWire> rtc(Wire);

uint8_t number[6];
uint8_t needProtect = 0;

byte num[] = {
    B00000001,
    B00000010,
    B00000100,
    B00001000,
    B00010000,
    B00100000,
    B01000000,
    B10000000,

};

byte nixietube[][10][9] = {
    //n6
    {{0,0,0x80,0,0,0,0,0,0},
    {0x01,0,0,0,0,0,0,0,0},
    {0,0x80,0,0,0,0,0,0,0},
    {0,0x40,0,0,0,0,0,0,0},
    {0,0x20,0,0,0,0,0,0,0},
    {0,0x10,0,0,0,0,0,0,0},
    {0,0x08,0,0,0,0,0,0,0},
    {0,0x04,0,0,0,0,0,0,0},
    {0,0x02,0,0,0,0,0,0,0},
    {0,0x01,0,0,0,0,0,0,0}},
    //n5
    {{0,0,0,0x10,0,0,0,0,0},
    {0,0,0x20,0,0,0,0,0,0},
    {0,0,0x10,0,0,0,0,0,0},
    {0,0,0x08,0,0,0,0,0,0},
    {0,0,0x04,0,0,0,0,0,0},
    {0,0,0x02,0,0,0,0,0,0},
    {0,0,0x01,0,0,0,0,0,0},
    {0,0,0,0x80,0,0,0,0,0},
    {0,0,0,0x40,0,0,0,0,0},
    {0,0,0,0x20,0,0,0,0,0}},
    //n4
    {{0,0,0,0,0x02,0,0,0,0},
    {0,0,0,0x04,0,0,0,0,0},
    {0,0,0,0x02,0,0,0,0,0},
    {0,0,0,0x01,0,0,0,0,0},
    {0,0,0,0,0x80,0,0,0,0},
    {0,0,0,0,0x40,0,0,0,0},
    {0,0,0,0,0x20,0,0,0,0},
    {0,0,0,0,0x10,0,0,0,0},
    {0,0,0,0,0x08,0,0,0,0},
    {0,0,0,0,0x04,0,0,0,0}},

    //n3
    {{0,0,0,0,0,0,0x40,0,0},
    {0,0,0,0,0,0x80,0,0,0},
    {0,0,0,0,0,0x40,0,0,0},
    {0,0,0,0,0,0x20,0,0,0},
    {0,0,0,0,0,0x10,0,0,0},
    {0,0,0,0,0,0x08,0,0,0},
    {0,0,0,0,0,0x04,0,0,0},
    {0,0,0,0,0,0x02,0,0,0},
    {0,0,0,0,0,0x01,0,0,0},
    {0,0,0,0,0,0,0x80,0,0}},

    //n2
    {{0,0,0,0,0,0,0,0x08,0},
    {0,0,0,0,0,0,0x10,0,0},
    {0,0,0,0,0,0,0x08,0,0},
    {0,0,0,0,0,0,0x04,0,0},
    {0,0,0,0,0,0,0x02,0,0},
    {0,0,0,0,0,0,0x01,0,0},
    {0,0,0,0,0,0,0,0x80,0},
    {0,0,0,0,0,0,0,0x40,0},
    {0,0,0,0,0,0,0,0x20,0},
    {0,0,0,0,0,0,0,0x10,0}},

    //n1
    {{0,0,0,0,0,0,0,0,0x01},
    {0,0,0,0,0,0,0,0x02,0},
    {0,0,0,0,0,0,0,0x01,0},
    {0,0,0,0,0,0,0,0,0x80},
    {0,0,0,0,0,0,0,0,0x40},
    {0,0,0,0,0,0,0,0,0x20},
    {0,0,0,0,0,0,0,0,0x10},
    {0,0,0,0,0,0,0,0,0x08},
    {0,0,0,0,0,0,0,0,0x04},
    {0,0,0,0,0,0,0,0,0x02}},

};
byte dot[][9] = {
    {0x02,0,0,0,0,0,0,0,0},
    {0,0,0x40,0,0,0,0,0,0},
    {0,0,0,0x08,0,0,0,0,0},
    {0,0,0,0,0x01,0,0,0,0},
    {0,0,0,0,0,0,0x20,0,0},
    {0,0,0,0,0,0,0,0x04,0}
};

void IRAM_ATTR onTimer0() {
    portENTER_CRITICAL_ISR(&timerMux0);
    static int time_stamp = 0;
    if (time_stamp >= 60)
    {
        needProtect = 1;
        time_stamp = 0;
    }
    time_stamp++;
    portEXIT_CRITICAL_ISR(&timerMux0);
}

void IRAM_ATTR onTimer1() {
    portENTER_CRITICAL_ISR(&timerMux1);
    //put your codes to do something
    
    portEXIT_CRITICAL_ISR(&timerMux1);
}

void Timer0_Init(uint16_t freq = 1000000) {
    timer0 = timerBegin(0, 80, true);
    timerAttachInterrupt(timer0, &onTimer0, true);
    timerAlarmWrite(timer0, freq*1000, true);
    timerAlarmEnable(timer0);
}

void Timer1_Init(uint16_t freq = 1000000) {
    timer1 = timerBegin(0, 80, true);
    timerAttachInterrupt(timer1, &onTimer1, true);
    timerAlarmWrite(timer1, freq * 1000, true);
    timerAlarmEnable(timer1);
}

void GPIO_Init(void) {
    pinMode(LED, OUTPUT);
    pinMode(EN, OUTPUT);
    digitalWrite(LED, LOW);
    
#if ENABLE_HV
    digitalWrite(EN, !HIGH);
#else
    digitalWrite(EN, HIGH);
#endif
    
}

void RTC_Init(void) {
    rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    if (!rtc.IsDateTimeValid())
    {
        if (rtc.LastError() != 0)
        {
            Serial.print("RTC communications error = ");
            Serial.println(rtc.LastError());
        }
        else
        {
            Serial.println("RTC lost confidence in the DateTime!");
            Serial.println("Common Causes:");
            Serial.println("  1. first time you ran and the device wasn't running yet");
            Serial.println("  2. the battery on the device is low or even missing");
            rtc.SetDateTime(compiled);
        }
    }
    if (!rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        rtc.SetIsRunning(true);
    }

    RtcDateTime now = rtc.GetDateTime();

    if (now < compiled)
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        rtc.SetDateTime(compiled);
    }
    else if (now > compiled)
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled)
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }

    rtc.Enable32kHzPin(false);
    rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

}

void Binary2BCD(unsigned long num){
    
    memset(number, 0, sizeof(number));

    while (num >= 100000)
    {
        number[5]++;
        num -= 100000;
    };
    while (num >= 10000)
    {
        number[4]++;
        num -= 10000;
    };
    while (num >= 1000)
    {
        number[3]++;
        num -= 1000;
    };
    while (num >= 100)
    {
        number[2]++;
        num -= 100;
    };
    while (num >= 10)
    {
        number[1]++;
        num -= 10;
    };
    while (num >= 1)
    {
        number[0]++;
        num -= 1;
    };

}

void writeNixie(unsigned long num) {
    byte tmp[1][1][9];
    int i, j,k;
    Binary2BCD(num);

        for (j = 0; j < 9; j++)
            tmp[0][0][j] = nixietube[0][number[0]][j] + nixietube[1][number[1]][j]  \
                         + nixietube[2][number[2]][j] + nixietube[3][number[3]][j]  \
                         + nixietube[4][number[4]][j] + nixietube[5][number[5]][j];
        for (j = 0; j < 9; j++)
            tpic.write(tmp[0][0][j]);
        
}


void nixie_clock(const RtcDateTime& dt) {
    char DateString[7];
    int Date = 0;
    snprintf_P(DateString,
        countnum(DateString),
        PSTR("%02u%02u%02u"),
        dt.Hour(),
        dt.Minute(),
        dt.Second());
    Serial.println(DateString);
    sscanf(DateString, "%d", &Date);
    writeNixie(Date);
}

void CathodeProtect(void) {
    int i,j,k;

    for (i = 0; i < 6; i++)
    {
        for (j = 0; j < 10; j++)
        {
            for (k = 0; k < 9; k++)
                tpic.write(nixietube[i][j][k]);
            delay(20); 
        }
        for (k = 0; k < 9; k++)
            tpic.write(dot[i][k]);
        delay(20);

    }
}

void setup()
{
    Serial.begin(115200);
    tpic.begin();
    GPIO_Init();
    delay(500);

    Serial.print("compiled: ");
    Serial.println(__DATE__);
    Serial.println(__TIME__);
    RTC_Init();
    CathodeProtect();
    Timer0_Init(1000);
//    Timer1_Init(300);
}

void loop()
{
    Serial.println();
    RtcDateTime now = rtc.GetDateTime();
    //printDateTime(now);
    if (needProtect == 0)
    {
        nixie_clock(now);
        delay(100);
    }
    else
    {
        CathodeProtect();
        needProtect = 0;
    }
    
}

void printDateTime(const RtcDateTime& dt) {
    char DateString[20];

    snprintf_P(DateString,
        countnum(DateString),
        PSTR("%04u/%02u/%02u %02u:%02u:%02u"),
        dt.Year(),
        dt.Month(),
        dt.Day(),
        dt.Hour(),
        dt.Minute(),
        dt.Second());
    Serial.print(DateString);
}
