#include "DistanceSensor.h"

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
		};

		void getSpeed(int newSpeed);
		int setSpeed();
		void start();
		void stop();
};

struct SensorData {
		int echo, trig;
};

class USensor {
	private:
		DistanceSensor sensor;
		int dist;

	public:
		void init(SensorData data) { sensor.init(data.trig, data.echo); }

		void tick() {
			int result = sensor.tick();

			// tick() returns NREADY if not ready, or
			// ERR on error (like timeout)
			if (result == sensor.NREADY) return;

			if (result < -1) return;
			dist = result;
		}

		int getDist() { return dist; }
};

class Rover {
	private:
		HBMotor motors[2];
		USensor sensor;

	public:
		void init(MotorData motorData[2], SensorData sensorData) {
			motors[0].init(motorData[0]);
			motors[1].init(motorData[1]);
			sensor.init(sensorData);
		}

		void tick() {
			// if (millis() - prevrun < 10) return; // Guard to not spam tick
			sensor.tick();
		}

		void move() {}	// TODO: movement logic
};