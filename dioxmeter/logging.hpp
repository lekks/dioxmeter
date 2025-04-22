/*
 * logging.hpp
 *
 *  Created on: Sep 28, 2020
 *      Author: ldir
 */

#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include "dioxmeter.hpp"

void setup_log(void);
void log_measurement(const Measurment& measurement);
void log_debug(const char*);

#endif /* LOGGING_HPP_ */
