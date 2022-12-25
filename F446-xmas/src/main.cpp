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
MLT8530 speaker;
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

int wish_wish[] = {NOTE_C5, NOTE_F5,  NOTE_F5, NOTE_G5, NOTE_F5, NOTE_E5,
                   NOTE_D5, NOTE_D5,  NOTE_D5, NOTE_G5, NOTE_G5, NOTE_A5,
                   NOTE_G5, NOTE_F5,  NOTE_E5, NOTE_C5, NOTE_C5, NOTE_A5,
                   NOTE_A5, NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F5, NOTE_D5,
                   NOTE_C5, NOTE_C5,  NOTE_D5, NOTE_G5, NOTE_E5, NOTE_F5};

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

void sing(App) {
    while (1) {
        int size = sizeof(wish_wish) / sizeof(int);
        for (int thisNote = 0; thisNote < size; thisNote++) {
            int noteDuration = 1500 / wish_tempo[thisNote];

            speaker.setFrequncy(wish_wish[thisNote]);
            app.delay(noteDuration);

            int pauseBetweenNotes = noteDuration * 0.10;
            speaker.mute();
            app.delay(pauseBetweenNotes);
        }
    }
}

void illuminate(App) {
    int counter = 0;
    randomSeed(micros());
    while (1) {
        switch (counter % 3) {
            case 0:
                led.setLeftColor(led.white);
                led.setRightColor(led.white);
                break;

            case 1:
                led.setLeftColor(led.red);
                led.setRightColor(led.red);
                break;

            case 2:
                led.setLeftColor(led.green);
                led.setRightColor(led.green);
                break;
        }

        static int color;
        for (int i = 0; i < 6; i++) {
            color += ((millis() % 2) ^ (micros() % 2)) + 1;
            for (int j = 0; j < 4; j++) {
                switch (color % 3) {
                    case 0:
                        stripUI.setPixelColor(i * 4 + j, led.white);
                        break;

                    case 1:
                        stripUI.setPixelColor(i * 4 + j, led.red);
                        break;

                    case 2:
                        stripUI.setPixelColor(i * 4 + j, led.green);
                        break;
                }
            }
        }
        stripUI.show();
        stripL.show();
        stripR.show();

        counter++;
        app.delay(200);
    }
}

void topIlluminate(App) {
    while (1) {
        uart1.println("くりぼっちおめでとう！！！！");
        double k_speed = 0.2;
        int color = (int)((millis() * k_speed - 270 )/ 360) % 3;

        int brightness =
            sin(radians((int)(millis() * k_speed) % 360)) * 127 + 128;

        switch (color) {
            case 0:
                led.setTopColor(led.white);
                break;

            case 1:
                led.setTopColor(led.red);
                break;

            case 2:
                led.setTopColor(led.green);
                break;
        }

        led.setTopBrightness(brightness);

        stripTop.show();

        app.delay(2);
    }
}

void setup() {
    uart1.begin(115200);

    led.setLeftColor(led.red);
    led.setRightColor(led.red);
    led.setUIColor(led.green);
    led.show();

    speaker.bootSound();
    led.bootIllumination();

    Wire.setSDA(PB9);
    Wire.setSCL(PB8);
    Wire.begin();

    gyro.init();

    app.create(sing, lowPriority);
    app.create(illuminate);
    app.create(topIlluminate);
    app.start(illuminate);
    app.start(topIlluminate);
    app.start(sing);
    app.startRTOS();
}

void loop() {
    // sing(3);
}