#include "config.h"
#include "dioxmeter.hpp"

void setup(void) {
	Serial.begin(9600);
	setup_co2();
	dashboard_impl()->setup();
}

void loop(void) {
	static Measurment co2_ppm;
	if (measue_co2(co2_ppm)) {
		dashboard_impl()->update(co2_ppm);
	}
}
