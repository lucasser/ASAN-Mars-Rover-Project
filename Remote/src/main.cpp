#include <Arduino.h>
#include <LiquidCrystal.h>
#include "Wifi.h"
#include "api/Common.h"

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);	 // initialize the library with the
// numbers of the interface pins

struct JoystickData {
		int x, y;
};

struct DataPacket {
		PacketType type;

		int Jx		 = 0;
		int Jy		 = 0;
		int throttle = 0;

		bool auton = 0;

		int sensor;
		int autonState;

		DataPacket() {}

		DataPacket(JoystickData joystick,
				   int throttle,
				   bool auton): Jx(joystick.x),
								Jy(joystick.y),
								throttle(throttle),
								auton(auton) {
			type = PacketType::Command;
		}

		DataPacket(PacketType packet): type(packet) {}
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

JoystickData readJoystick() {
	return {analogRead(joystick[0]), analogRead(joystick[1])};
}

void setup() {
	Serial.begin(9600);
	setPinmodes();
	wifi.begin(ssid, pwd, WifiControllerType::Server);
	lcd.begin(16, 2);  // set up the LCD's number of columns and rows:
	lcd.setCursor(0, 0);
}

unsigned long prevrun;

void tick() {
	if (millis() - prevrun < 10) return;
	prevrun = millis();
	if (wifi.active())
		wifi.setData(DataPacket(readJoystick(), 0, !digitalRead(autonSwitch)));
}

void loop() {
	wifi.tick();
	tick();
	if (wifi.available()) {
		data = wifi.getData();
		lcd.setCursor(0, 0);
		lcd.print("Distance: ");
		lcd.print(data.sensor);
		lcd.print("             ");
		lcd.setCursor(0, 1);
		lcd.print("Auton State: ");
		lcd.print(data.autonState);
		lcd.print("             ");
	}
}