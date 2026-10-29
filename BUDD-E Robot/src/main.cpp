#include <Arduino.h>
#include "Rover.h"
#include "Wifi.h"

MotorData motors[2] = {
	{2, 4, 3},
	{5, 7, 6}
};
SensorData sensor = {8, 9};

Rover rover;

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

void setup() {
	Serial.begin(9600);
	rover.init(motors, sensor);
	wifi.begin(ssid, pwd, WifiControllerType::Client);
}

unsigned long prevrun;

void tick() {
	if (millis() - prevrun < 20) return;
	prevrun = millis();
	if (wifi.active()) wifi.setData({PacketType::State, 0, 0, rover.getDist()});
}

void loop() {
	wifi.tick();
	rover.tick();
	if (wifi.available()) {
		data = wifi.getData();
		Serial.print("Jx: ");
		Serial.println(data.Jx);
		Serial.print("Jy: ");
		Serial.println(data.Jy);
	}
	rover.tick();
	tick();
}