#include <Arduino.h>
#include <WString.h>

int joystick[2] = {A0, A1}; //x, y

int button = 2;

int pot = A2;

void setup() {
	Serial.begin(9600);
	pinMode(button, INPUT_PULLUP);
}

void loop() {
  	Serial.print("Joystick X: ");
  	Serial.println(analogRead(joystick[0]));
  	Serial.print("Joystick Y: ");
  	Serial.println(analogRead(joystick[1]));
	Serial.print("Auton?: ");
  	Serial.println((digitalRead(button)) ? "No" : "Yes");
	Serial.print("Throttle: ");
  	Serial.println(analogRead(pot));
	delay(100);
}