#include <Arduino.h>
#include <WString.h>
#include "Rover.h"

MotorData motors[2] = {
	{2, 4, 3},
	{5, 7, 6}
};
SensorData sensor = {8, 9};

Rover rover;

void setup() {
	Serial.begin(9600);
	rover.init(motors, sensor);
}

void loop() {
	rover.tick();
}