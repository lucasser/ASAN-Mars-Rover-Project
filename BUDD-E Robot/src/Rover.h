#include "USensor.h"
#include "api/Common.h"

struct MotorData {
		int fwd, rev, speed;
};

class HBMotor {	 // H-Bridge Motor
	private:
		int fwd;
		int rev;
		int speed;

	public:
		void init(MotorData data) {
			fwd	  = data.fwd;
			rev	  = data.rev;
			speed = data.speed;

			pinMode(fwd, OUTPUT);
			pinMode(rev, OUTPUT);
			pinMode(speed, OUTPUT);
		};

		int setSpeed(int speedval) {
			Serial.println(speedval);
			digitalWrite(fwd, (speedval > 10));
			digitalWrite(rev, (speedval < -10));
			analogWrite(speed, abs(speedval));
		};

		void getSpeed();

		void start(bool rev) {

		};

		void stop();
};

struct SensorData {
		int echo, trig;
};

class USensor {
	private:
		Ultrasonic sensor;
		int dist;

	public:
		void init(SensorData data) { sensor.begin(data.trig, data.echo); }

		unsigned long prevrun;

		void tick() {
			if (millis() - prevrun < 50) return;
			prevrun = millis();
			dist	= sensor.readDistanceCM();
			prevrun = millis();
		}

		int getDist() { return dist; }
};

struct RoverData {
		int Jx, Jy, Throttle;
		bool Auton;
};

class Rover {
	private:
		HBMotor motors[2];
		USensor sensor;
		RoverData data;

	public:
		void init(MotorData motorData[2], SensorData sensorData) {
			motors[0].init(motorData[0]);
			motors[1].init(motorData[1]);
			sensor.init(sensorData);
		}

		unsigned long prevrun;

		void tick() {
			sensor.tick();
			if (millis() - prevrun < 50) return;
			if (data.Auton) {
				if (sensor.getDist() < 10) {
					motors[0].setSpeed(-100);
					motors[1].setSpeed(-100);
				} else {
					motors[0].setSpeed(0);
					motors[1].setSpeed(0);
				}
			} else {
				int Jx = map(data.Jx, 0, 1023, -255, 255);
				int Jy = map(data.Jy, 0, 1023, -255, 255);
				motors[0].setSpeed(constrain(Jx - Jy, -255, 255));
				motors[1].setSpeed(constrain(Jx + Jy, -255, 255));
			}
			prevrun = millis();
		}

		void sendData(RoverData data) { this->data = data; }

		int getDist() { return sensor.getDist(); }

		void move() {}	// TODO: movement logic
};