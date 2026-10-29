// Distance sensor

#pragma once

#include <Arduino.h>
#include <cstdint>

// A very stateful non-blocking implementation for an HC-SR04
class DistanceSensor {
	private:
		int TrigPin;
		int EchoPin;

	public:
		void init(uint8_t TrigPin, uint8_t EchoPin) {
			this->TrigPin = TrigPin;
			this->EchoPin = EchoPin;
			pinMode(TrigPin, OUTPUT);
			pinMode(EchoPin, INPUT);
		}

		inline DistanceSensor(): nextTick(0), phase(1) {}

		constexpr static int NREADY = -1;
		constexpr static int ERR	= -2;

		/**
		 * Ticks the HC-SR04
		 * Returns a value when ready, NREADY when not ready
		 * or ERR on error
		 * Call as fast as possible
		 */
		int tick() {
			if (micros() < nextTick) return NREADY;

			switch (phase) {
				case 1: phase1(); return NREADY;
				case 2: phase2(); return NREADY;
				case 3:
					if (!phase3()) return ERR;
					return NREADY;
				case 4: return phase4();
				default:
					// How?
					phase = 1;
					return ERR;
			}
		}

	private:
		unsigned long nextTick;
		uint16_t phase;

		void phase1() {
			digitalWrite(TrigPin, HIGH);
			phase	 = 2;
			nextTick = micros() + 10;
		}

		void phase2() {
			digitalWrite(TrigPin, LOW);
			phase	 = 3;
			nextTick = micros();
		}

		bool phase3() {
			// Wait for pin to go high
			if (digitalRead(EchoPin) != HIGH) {
				// 1 second
				if (micros() - 1000000 > nextTick) {
					phase = 1;
					// timed out
					return false;
				}

				// true bc didn't error
				return true;
			}

			// Pin is high
			// Save time in nextTick

			nextTick = micros();
			phase	 = 4;

			return true;
		}

		int phase4() {
			// Wait for pin to go low
			if (digitalRead(EchoPin)) return NREADY;

			// Pin is low
			// Get time

			unsigned long time = micros() - nextTick;

			phase = 1;

			int cm = time / 29 / 2;

			if (cm > 1900) { return ERR; }
			return cm;
		}
};
