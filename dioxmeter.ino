#include "config.h"
#include "dioxmeter.hpp"
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

void setup(void) {
	Serial.begin(9600);
	setup_pcint();
	dashboard_impl()->setup();
}

void loop(void) {
	static unsigned int _cnt;
	static int _ppm;
	static bool heated = false;
	unsigned long current_time = millis();

	if (!heated  && current_time > HEATING_DELAY) {
		heated = true;
	}
	if (_cnt != cnt) {
		int ppm = calc_ppm(period, imp, 5000);
		dashboard_impl()->update(ppm, heated);
		_cnt = cnt;
	}
}
