#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <Arduino.h>

class Ultrasonic {
	private:
		uint8_t _trigPin;
		uint8_t _echoPin;
		unsigned long _timeout;

	public:
		// Configures the pin modes (Call this inside your setup() function)
		void begin(uint8_t trigPin,
				   uint8_t echoPin,
				   unsigned long timeout = 20000) {
			_trigPin = trigPin;
			_echoPin = echoPin;
			_timeout = timeout;
			pinMode(_trigPin, OUTPUT);
			pinMode(_echoPin, INPUT);
			digitalWrite(_trigPin, LOW);
		}

		// Measures and returns the distance in centimeters
		float readDistanceCM() {
			// Ensure trigger pin is low first
			digitalWrite(_trigPin, LOW);
			delayMicroseconds(2);

			// Send a 10-microsecond high pulse to trigger the sensor
			digitalWrite(_trigPin, HIGH);
			delayMicroseconds(10);
			digitalWrite(_trigPin, LOW);

			// Measure the high pulse duration on the echo pin
			unsigned long duration = pulseIn(_echoPin, HIGH, _timeout);

			// If pulseIn times out (returns 0), return -1.0 to indicate out of
			// range
			if (duration == 0) { return -1.0f; }

			// Calculate distance: (speed of sound is ~343 m/s or 0.0343 cm/us)
			// Distance = (duration * 0.0343) / 2
			return (duration * 0.0343f) / 2.0f;
		}
};

#endif	// ULTRASONIC_H