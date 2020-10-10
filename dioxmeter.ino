#include "config.h"
#include "dioxmeter.hpp"
#include "sensor.hpp"
#include "dashboard.hpp"
#include "logging.hpp"

void setup(void) {
	setup_log();
	setup_co2();
	setup_dashboard();
}

void loop(void) {
	static Measurment co2_ppm;
	if (measue_co2(co2_ppm)) {
		update_dashboard(co2_ppm);
		log_measurement(co2_ppm);
	}
}
