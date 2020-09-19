/*
 * sensors.ino
 *
 *  Created on: Sep 19, 2020
 *      Author: ldir
 */

#include "config.h"
#include "dioxmeter.hpp"
#include <Arduino.h>
#include "utils.hpp"

static volatile unsigned long period;
static volatile unsigned long imp;
static volatile unsigned int cnt;

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
	static volatile bool last_sig;
	static volatile unsigned long last_on;

	bool sig = PINC & _BV(PC5);
	if(last_sig != sig) {
		//unsigned long time = millis();
		unsigned long time = micros();
		if(sig) {
			if(valid) {
				period = time_diff(last_on,time);
				imp = imp_int;
				++cnt;
			}
			valid = true;
			last_on = time;
		} else {
			imp_int = time_diff(last_on,time);
		}
		last_sig = sig;
	}
}

int calc_ppm(unsigned long period, unsigned long imp, int MAX_PPM) {
	int dead_time = period / 251;
	return MAX_PPM * (imp - dead_time / 2) / (period - dead_time);
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
	if (_cnt != cnt) {
		 ppm = calc_ppm(period, imp, 5000);
		_cnt = cnt;
	}
	result.valid = heated;
	result.value = ppm;
	return true;
}
