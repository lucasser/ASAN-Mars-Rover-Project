// 27

#include <Servo.h>
#include "Arduino.h"
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
			digitalWrite(fwd, (speedval > 10));
			digitalWrite(rev, (speedval < -10));
			analogWrite(speed, abs(speedval));
		};

		void getSpeed();

		void start(bool rev) {

		};

		void stop();
};

struct ServoMotorData {
		int signal;
		int rev;
};

class ServoMotor {
	private:
		Servo servo;
		int rev;  // -1 or 1

	public:
		void init(ServoMotorData data) {
			this->rev = data.rev;
			servo.attach(data.signal);
			servo.write(90);
		};

		void setSpeed(int speedval) {
			if (-2 < speedval && speedval < 2) {
				servo.write(90);
				return;
			};
			servo.write(map(speedval * rev, -90, 90, 0, 180));
		};
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
		int Jx, Jy, Throttle = 0;  // -90 < < 90, 0, 90
		bool Auton = 0;
};

class Rover {
	private:
		const int TURN_TIME = 600;
		const int WALL_DIST = 28;

		int turntime;

		ServoMotor motors[2];
		USensor sensor;
		RoverData data;

		int state = 0;

		int remapSpeed(int Jx, int Jy) {  // -90 < < 90
			return constrain(Jx + Jy, -data.Throttle, data.Throttle);
		}

	public:
		void init(ServoMotorData motorData[2], SensorData sensorData) {
			motors[0].init(motorData[0]);
			motors[1].init(motorData[1]);
			sensor.init(sensorData);
		}

		unsigned long prevrun;

		void tick() {
			sensor.tick();
			// if (millis() - prevrun < 10) return;
			if (data.Auton) {
				if (state == 6) { return; }
				state = auton(state);
			} else {
				state = 0;
				if (data.Jx >= 0) {
					moveSlow(data.Jx + data.Jy, data.Jx - data.Jy);
				} else {
					moveSlow(data.Jx - data.Jy, data.Jx + data.Jy);
				}
			}
			prevrun = millis();
		}

		int auton(int state) {
			int dist = sensor.getDist();
			switch (state) {
				case 0:	 // First straight
					{
						move(90, 90);
						turntime = millis();
						return state + 1;
					}
				case 1:	 // First turn
					{
						if (dist <= WALL_DIST) {
							move(15, 90);
							turntime = millis();
							return state + 1;
						}
						return state;
					}
				case 2:	 // Second straight
					{
						if (millis() - turntime >= TURN_TIME) {
							move(90, 90);
							turntime = millis();
							return state + 1;
						}

						return state;
					}
				case 3:	 // Second turn
					{
						if (millis() - turntime >= 150) {
							move(90, 15);
							turntime = millis();
							return state + 1;
						}

						return state;
					}
				case 4:	 // Final straight
					{
						if (millis() - turntime >= TURN_TIME) {
							move(90, 90);
							return state + 1;
						}

						return state;
					}
				case 5:	 // done
					{
						if (dist <= 12) {
							move(0, 0);
							return state + 1;
						}
						return state;
					}
			}
		}

		void sendData(RoverData data) { this->data = data; }

		int getDist() { return sensor.getDist(); }

		int getState() { return state; }

		void move(int m1, int m2) {
			motors[0].setSpeed(constrain(m1, -data.Throttle, data.Throttle));
			motors[1].setSpeed(constrain(m2, -data.Throttle, data.Throttle));
		}

		void moveSlow(int m1, int m2) {
			if (m1 != m2) {
				m1 = m1 / 3;
				m2 = m2 / 3;
			}
			motors[0].setSpeed(constrain(m1, -data.Throttle, data.Throttle));
			motors[1].setSpeed(constrain(m2, -data.Throttle, data.Throttle));
		}
};