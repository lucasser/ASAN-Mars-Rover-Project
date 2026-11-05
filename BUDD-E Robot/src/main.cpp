#include "IPAddress.h"
#include "Rover.h"
#include "Wifi.h"
#include <Arduino.h>
#include <cstdlib>

MotorData motors[2] = {{2, 4, 3}, {5, 7, 6}};

ServoMotorData smotors[2] = {{12, 1}, {13, -1}};

SensorData sensor = {4, 3};

Rover rover;

struct DataPacket {
  PacketType type;

  int Jx = 0;       // 0 < < 1023
  int Jy = 0;       // 0 < < 1023
  int throttle = 0; // 0 < < 1023

  bool auton = 0;

  int sensor;
  int autonState;

  DataPacket() {}

  DataPacket(int sensor, int autonState)
      : sensor(sensor), autonState(autonState) {
    type = PacketType::State;
  }

  DataPacket(PacketType packet) : type(packet) {}
};

DataPacket data;

WifiController<DataPacket> wifi;

const String ssid = "Team 111";
const String pwd = "password";

void setup() {
  Serial.begin(9600);
  rover.init(smotors, sensor);
  wifi.begin(ssid, pwd, WifiControllerType::Client);
}

unsigned long prevrun;

void tick() {
  if (millis() - prevrun < 20)
    return;
  prevrun = millis();
  if (wifi.active()) {
    wifi.setData(DataPacket(rover.getDist(), rover.getState()));
  }
}

int deadzone(int val) { return (-5 < val && val < 5) ? 0 : val; }

void loop() {
  wifi.tick();
  rover.tick();
  if (wifi.available()) {
    data = wifi.getData();
    rover.sendData({deadzone(map(data.Jx, 0, 1022, -90, 90)),
                    deadzone(map(data.Jy, 0, 1022, -90, 90)),
                    map(data.throttle, 0, 1023, 0, 90), data.auton});
  }
  tick();
}
