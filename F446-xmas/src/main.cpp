#include <Arduino.h>

#include "./lib/IO-Kit.h"
#include "./lib/RTOS-Kit.h"

HardwareSerial uart1(PA10, PA9);
HardwareSerial uart2(PA3, PA2);
HardwareSerial uart4(PA1, PA0);
HardwareSerial uart5(PD2, PC12);
HardwareSerial uart6(PC7, PC6);

RTOS_Kit app;

#include "./lib/bno055.h"
#include "./lib/floorSensor.h"
#include "./lib/mlt8530.h"
#include "./lib/switchUI.h"
#include "./lib/vl53l0x.h"
#include "./lib/ws2812b.h"
#include "./lib/floorSensor.h"
#include "./lib/unitV.h"

Adafruit_NeoPixel stripL = Adafruit_NeoPixel(7, PA15, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripR = Adafruit_NeoPixel(7, PB13, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripUI = Adafruit_NeoPixel(24, PB14, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripTop = Adafruit_NeoPixel(24, PC1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripFloor = Adafruit_NeoPixel(3, PB15, NEO_GRB + NEO_KHZ800);

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

VL53L0X distanceSensor(&uart4);
BNO055 gyro(&bno);
WS2812B led(50);
// MLT8530 speaker;
SWITCHUI ui;
FLOOR_SENSOR floorSensor;
UNITV cameraLeft(&uart6);
UNITV cameraRight(&uart2);

#include "./lib/sts3032.h"
STS3032 servo(&uart5);

#include "./app/sensorApp.h"
#include "pitches.h"

#define melodyPin PB6

int melody[] = {NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
                NOTE_G5, NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_F5, NOTE_F5,
                NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
                NOTE_D5, NOTE_D5, NOTE_E5, NOTE_D5, NOTE_G5};

int tempo[] = {8, 8, 4, 8, 8, 4,  8,  8, 8, 8, 2, 8, 8,
               8, 8, 8, 8, 8, 16, 16, 8, 8, 8, 8, 4, 4};

// We wish you a merry Christmas

int wish_melody[] = {NOTE_B3, NOTE_F4, NOTE_F4, NOTE_G4, NOTE_F4, NOTE_E4,
                     NOTE_D4, NOTE_D4, NOTE_D4, NOTE_G4, NOTE_G4, NOTE_A4,
                     NOTE_G4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_E4, NOTE_A4,
                     NOTE_A4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_F4, NOTE_D4,
                     NOTE_B3, NOTE_B3, NOTE_D4, NOTE_G4, NOTE_E4, NOTE_F4};

int wish_tempo[] = {4, 4, 8, 8, 8, 8, 4, 4, 4, 4, 8, 8, 8, 8, 4,
                    4, 4, 4, 8, 8, 8, 8, 4, 4, 8, 8, 4, 4, 4, 2};

// Santa Claus is coming to town

int santa_melody[] = {NOTE_G4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_G4, NOTE_G4,
                      NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, NOTE_C5, NOTE_E4,
                      NOTE_F4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4,
                      NOTE_F4, NOTE_F4, NOTE_E4, NOTE_G4, NOTE_C4, NOTE_E4,
                      NOTE_D4, NOTE_F4, NOTE_B3, NOTE_C4};

int santa_tempo[] = {8, 8, 8, 4, 4, 4, 8, 8, 4, 4, 4, 8, 8, 4,
                     4, 4, 8, 8, 4, 2, 4, 4, 4, 4, 4, 2, 4, 1};

int song = 0;

void buzz(int targetPin, long frequency, long length) {
    long delayValue = 1000000 / frequency /
                      2;  // calculate the delay value between transitions
    //// 1 second's worth of microseconds, divided by the frequency, then split
    /// in half since / there are two phases to each cycle
    long numCycles = frequency * length /
                     1000;  // calculate the number of cycles for proper timing
    //// multiply frequency, which is really cycles per second, by the number of
    /// seconds to / get the total number of cycles to produce
    for (long i = 0; i < numCycles;
         i++) {  // for the calculated length of time...
        digitalWrite(
            targetPin,
            HIGH);  // write the buzzer pin high to push out the diaphram
        delayMicroseconds(delayValue);  // wait for the calculated delay value
        digitalWrite(
            targetPin,
            LOW);  // write the buzzer pin low to pull back the diaphram
        delayMicroseconds(
            delayValue);  // wait again or the calculated delay value
    }
}

void sing(int s) {
    // iterate over the notes of the melody:
    song = s;
    if (song == 3) {
        uart1.println(" 'We wish you a Merry Christmas'");
        int size = sizeof(wish_melody) / sizeof(int);
        for (int thisNote = 0; thisNote < size; thisNote++) {
            // to calculate the note duration, take one second
            // divided by the note type.
            // e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
            int noteDuration = 1000 / wish_tempo[thisNote];

            buzz(melodyPin, wish_melody[thisNote], noteDuration);

            // to distinguish the notes, set a minimum time between them.
            // the note's duration + 30% seems to work well:
            int pauseBetweenNotes = noteDuration * 0.50;
            delay(pauseBetweenNotes);

            // stop the tone playing:
            buzz(melodyPin, 0, noteDuration);
        }
    } else if (song == 2) {
        uart1.println(" 'Santa Claus is coming to town'");
        int size = sizeof(santa_melody) / sizeof(int);
        for (int thisNote = 0; thisNote < size; thisNote++) {
            // to calculate the note duration, take one second
            // divided by the note type.
            // e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
            int noteDuration = 900 / santa_tempo[thisNote];

            buzz(melodyPin, santa_melody[thisNote], noteDuration);

            // to distinguish the notes, set a minimum time between them.
            // the note's duration + 30% seems to work well:
            int pauseBetweenNotes = noteDuration * 1.30;
            delay(pauseBetweenNotes);

            // stop the tone playing:
            buzz(melodyPin, 0, noteDuration);
        }
    } else {
        uart1.println(" 'Jingle Bells'");
        int size = sizeof(melody) / sizeof(int);
        for (int thisNote = 0; thisNote < size; thisNote++) {
            // to calculate the note duration, take one second
            // divided by the note type.
            // e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
            int noteDuration = 1000 / tempo[thisNote];

            buzz(melodyPin, melody[thisNote], noteDuration);

            // to distinguish the notes, set a minimum time between them.
            // the note's duration + 30% seems to work well:
            int pauseBetweenNotes = noteDuration * 0.50;
            delay(pauseBetweenNotes);

            // stop the tone playing:
            buzz(melodyPin, 0, noteDuration);
        }
    }
}

void setup() {
    uart1.setRx(PA10);
    uart1.setTx(PA9);
    uart1.begin(115200);

    uart2.setRx(PA3);
    uart2.setTx(PA2);
    uart2.begin(115200);

    uart6.setRx(PC7);
    uart6.setTx(PC6);
    uart6.begin(115200);

    led.setLeftColor(led.yellow);
    led.setRightColor(led.yellow);
    led.setUIColor(led.yellow);
    led.show();

    // speaker.bootSound();
    led.bootIllumination();

    Wire.setSDA(PB9);
    Wire.setSCL(PB8);
    Wire.begin();

    gyro.init();

    // app.create(mainApp, firstPriority);
    // app.create(VictimDetectionLED);
    // app.create(inputMonitoringApp, firstPriority);
    // app.create(largeDrive);
    // app.create(onlyRight);
    // app.create(onlyLeft);
    // app.create(isOnBlack);
    // app.create(isOnBlue);
    // app.create(oooon);
    // app.create(right);
    // app.create(random);
    // app.create(VictimDetectionLED);

    // app.start(mainApp);
    // app.start(inputMonitoringApp);
    // app.start(VictimDetectionLED);
    // app.startRTOS();
}

void loop() {
    pinMode(PB6, OUTPUT);
    sing(3);
} 