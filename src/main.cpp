/*INSTRUCTIONS:

First: ***Uncomment and run "eraseBTbonds.cpp"***

Second: To put the Xbox controller into BLE pairing mode

    1. Turn controller off
    2. Press Xbox button → turn it on
    3. Hold the pair button (near LB/RB) for ~3 seconds
    4. Xbox logo starts rapid flashing

Only Xbox One (BLE models) and Series X/S controllers work.*/


#include <Arduino.h>
#include <Bluepad32.h>
#include <ESP32Servo.h>
#include <cmath>
#include <algorithm>

GamepadPtr myGamepad = nullptr;
Servo escR;
Servo escL;

int escPinR = 22; 
int escPinL = 23; 
bool wasConnected = false;
bool bleStarted = false;
int nothing = 0;


// Called when a controller connects
void onConnectedGamepad(GamepadPtr gp) {
    Serial.println("Gamepad connected");
    myGamepad = gp;
}

// Called when a controller disconnects
void onDisconnectedGamepad(GamepadPtr gp) {
    Serial.println("Gamepad disconnected");
    if (myGamepad == gp) {
        myGamepad = nullptr;
    }
}

struct Vec
{
    double x;
    double y;
};

// Transforms the controller input vector to right/left motor speeds
Vec transform(const Vec& v)
{
    // Step 1: rotate by -45 degrees
    const double invSqrt2 = 1.0 / sqrt(2.0);

    double xr =  (v.x + v.y) * invSqrt2;
    double yr = (-v.x + v.y) * invSqrt2;

    // Step 2: normalize by max absolute component
    double maxAbs = max(abs(xr), fabs(yr));

    // Guard against zero (just in case)
    if (maxAbs == 0.0)
    {
        return {0.0, 0.0};
    }

    return { xr / maxAbs, yr / maxAbs };
}

void setup() {
    
    Serial.begin(115200);
    delay(500);

    Serial.println("Starting Bluepad32...");

    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

    Serial.println();
    escR.attach(escPinR, 1000, 2000);
    escL.attach(escPinL, 1000, 2000);
    escR.writeMicroseconds(1500);
    escL.writeMicroseconds(1500);
    delay(1000);
}

void loop() {
    BP32.update();

    if (myGamepad && myGamepad->isConnected()) {
        Serial.println("Xbox controller connected");

        
        int rx = myGamepad->axisRX();
        int ry = myGamepad->axisRY();



        int rStickX = static_cast<double>(rx)*(100/512);
        int rStickY = -static_cast<double>(rx)*(100/512);


        if (rStickX > -10 && rStickX < 10) {
            rStickX = 0;}
        if (rStickY > -10 && rStickY < 10) {
            rStickY = 0;}


        //Clamp speeds to prevent potential errors
        (rStickX > 100) ? (rStickX = 100):(rStickX = rStickX);
        (rStickX < -100) ? (rStickX = -100):(rStickX = rStickX);
        (rStickY > 100) ? (rStickY = 100):(rStickY = rStickY);
        (rStickY < -100) ? (rStickY = -100):(rStickY = rStickY);

        Vec v = {static_cast<double>(rStickX), static_cast<double>(rStickY)};
        Vec motorSpeeds = transform(v);

        escR.writeMicroseconds(1000+motorSpeeds.x*500);
        escL.writeMicroseconds(1000+motorSpeeds.y*500);
        
    } 
    else {
        if (wasConnected) {
            Serial.println("Xbox controller disconnected");
            wasConnected = false;
            
            // STOP motors on disconnect
            escR.writeMicroseconds(1500);
            escL.writeMicroseconds(1500);
        }

        Serial.println("Waiting for Xbox controller...");
        delay(250);
    }

    delay(5);
}