/*
 * dioxmeter.hpp
 *
 *  Created on: Sep 19, 2020
 *      Author: ldir
 */

#ifndef DIOXMETER_HPP_
#define DIOXMETER_HPP_

#define VER "1.3"

struct Measurment {
	int value;
	bool valid;
};

void setup_co2();
bool measue_co2(Measurment&);

void setup_dashboard(void);
void update_dashboard(const Measurment& measurement);

void setup_log(void);
void log_measurement(const Measurment& measurement);
void log_debug(const char*);

#endif /* DIOXMETER_HPP_ */
