#include <Arduino.h>
#include <LiquidCrystal.h>
#include "Wifi.h"

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);	 // initialize the library with the
// numbers of the interface pins

struct DataPacket {
		PacketType type;

		int Jx;
		int Jy;

		int sensor;
};

DataPacket data;

WifiController<DataPacket> wifi;

const String ssid = "Team 113";
const String pwd  = "password";

const int autonSwitch = 2;
const int joystick[2] = {A0, A1};

void setPinmodes() {
	pinMode(autonSwitch, INPUT_PULLUP);
	pinMode(joystick[0], INPUT);
	pinMode(joystick[1], INPUT);
}

struct JoystickData {
		int x, y;
};

JoystickData readJoystick() {
	return {analogRead(joystick[0]), analogRead(joystick[1])};
}

void setup() {
	Serial.begin(9600);
	setPinmodes();
	lcd.begin(16, 2);  // set up the LCD's number of columns and rows:
	wifi.begin(ssid, pwd, WifiControllerType::Server);
}

unsigned long prevrun;

void tick() {
	if (millis() - prevrun < 20) return;
	prevrun = millis();
	if (wifi.active())
		wifi.setData({PacketType::Command, readJoystick().x, readJoystick().y});
}

void loop() {
	wifi.tick();
	tick();
	if (wifi.available()) {
		data = wifi.getData();
		Serial.print("Sensor: ");
		Serial.println(data.sensor);
	}
}