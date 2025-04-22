/*
 * sensors.ino
 *
 *  Created on: Sep 19, 2020
 *      Author: ldir
 */

#include "config.h"
#include "sensor.hpp"
#include <Arduino.h>
#include "utils.hpp"
#include "logging.hpp"


static volatile unsigned long period_us;
static volatile unsigned long pulse_time_us;
static volatile unsigned int mesurements_cnt;

static void setup_pcint() {
	DDRC &= ~bit(PC5); //AIN
	PORTC |= bit(PC5);
	PCMSK1 |= bit(PCINT13);
	PCICR |= bit(PCIE1);
}

SIGNAL(PCINT1_vect) //PCINT13 PC5 ADC5
{
	static volatile bool valid=false;
	static volatile unsigned long imp_int;
	static volatile bool last_level;
	static volatile unsigned long last_on;

	bool level = PINC & _BV(PC5);
	if(last_level != level) {
		unsigned long time = micros();
		if(level) {
			if(valid) {
				period_us = time_diff(last_on,time);
				pulse_time_us = imp_int;
				++mesurements_cnt;
			}
			valid = true;
			last_on = time;
		} else {
			imp_int = time_diff(last_on,time);
		}
		last_level = level;
	}
}

int calc_ppm(unsigned long period, unsigned long imp, int max_ppm) {
	int dead_time = period / 251;
	return max_ppm * (imp - dead_time / 2) / (period - dead_time);
}

void setup_co2() {
	setup_pcint();
}

bool measue_co2(Measurment &result) {
	static unsigned int _cnt;
	static int ppm;
	static bool heated = false;
	unsigned long current_time = millis();

	if (!heated && current_time > HEATING_DELAY) {
		heated = true;
	}
	result.valid = heated;
	if (_cnt != mesurements_cnt) {
		 ppm = calc_ppm(period_us, pulse_time_us, MAX_PPM);
		_cnt = mesurements_cnt;
		result.value = ppm;
		return true;
	} else {
		return false;
	}
}
