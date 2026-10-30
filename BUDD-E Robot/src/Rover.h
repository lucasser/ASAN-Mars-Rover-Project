#include "Arduino.h"
#include "USensor.h"

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
		int dist = 0;

	public:
		void init(SensorData data) { sensor.begin(data.trig, data.echo); }

		unsigned long prevrun;

		void tick() {
			if (millis() - prevrun < 10) return;
			prevrun		 = millis();
			int tempDist = sensor.readDistanceCM();
			dist		 = (tempDist >= 0) ? tempDist : dist;
			prevrun		 = millis();
		}

		int getDist() { return dist; }
};

struct RoverData {
		int Jx, Jy, Throttle = 0;
		bool Auton = 0;
};

class Rover {
	private:
		const int TURN_TIME = 2000;
		const int WALL_DIST = 10;

		int turntime;

		HBMotor motors[2];
		USensor sensor;
		RoverData data;

		int state = 0;

	public:
		void init(MotorData motorData[2], SensorData sensorData) {
			motors[0].init(motorData[0]);
			motors[1].init(motorData[1]);
			sensor.init(sensorData);
		}

		unsigned long prevrun;

		void tick() {
			sensor.tick();
			// if (millis() - prevrun < 10) return;
			if (data.Auton) {
				if (state == 5) { return; }
				state = auton(state);
			} else {
				state  = 0;
				int Jx = map(data.Jx, 0, 1023, -255, 255);
				int Jy = map(data.Jy, 0, 1023, -255, 255);
				move(constrain(Jx + Jy, -255, 255),
					 constrain(Jx - Jy, -255, 255));
			}
			prevrun = millis();
		}

		int auton(int state) {
			int dist = sensor.getDist();
			switch (state) {
				case 0:	 // First straight
					{
						if (dist > WALL_DIST) {
							move(255, 255);
							return state;
						}
						move(0, 0);
						turntime = millis();
						return state + 1;
					}
				case 1:	 // First turn
					{
						if (millis() - turntime < TURN_TIME) {
							move(-255, 255);
							return state;
						}
						return state + 1;
					}
				case 2:	 // Second straight
					{
						if (dist > WALL_DIST) {
							move(255, 255);
							return state;
						}
						move(0, 0);
						turntime = millis();
						return state + 1;
					}
				case 3:	 // Second turn
					{
						if (millis() - turntime < TURN_TIME) {
							move(255, -255);
							return state;
						}
						return state + 1;
					}
				case 4:	 // Final straight
					{
						if (dist > WALL_DIST) {
							move(255, 255);
							return state;
						}
						move(0, 0);
						return state + 1;
					}
			}
		}

		void sendData(RoverData data) { this->data = data; }

		int getDist() { return sensor.getDist(); }

		int getState() { return state; }

		void move(int m1, int m2) {
			motors[0].setSpeed(m1);
			motors[1].setSpeed(m2);
		}
};