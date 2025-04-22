/*
 * logging.cpp
 *
 *  Created on: Sep 27, 2020
 *      Author: ldir
 */
#include "dioxmeter.hpp"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#define MAX_STRING_LENGHT 32 //Doesn`t work with big values, I can`t understand why
static char str_buf[MAX_STRING_LENGHT + 1];

static int makeNmeaSentense(char *buffer, int size, const char *sentense, ...) {
	int ret;
	if (*sentense != '$')
		return -1;
	va_list arg;
	va_start(arg, sentense);
	ret = vsnprintf(buffer, size, sentense, arg);
	va_end(arg);
	if (buffer[ret - 1] != '*')
		return -1;
	if (ret + 5 > size)
		return -1;
	unsigned char crc = 0;
	char *data = buffer + 1;
	while (*data && *data != '*') {
		crc ^= *data++;
	}
	ret += snprintf(&buffer[ret], 5, "%02X\r\n", crc);
	return ret;
}

static inline void print_str(const char *str) {
	Serial.println(str);
}


static void dump_nmea_value(const char *name, int value, bool valid) {
	makeNmeaSentense(str_buf, MAX_STRING_LENGHT, "$%s,%d,%i*", name, value, static_cast<int>(valid));
	print_str(str_buf);
}

static void dump_nmea_value(const char *name, const char *value) {
	makeNmeaSentense(str_buf, MAX_STRING_LENGHT, "$%s,%s*", name, value);
	print_str(str_buf);
}

void setup_log(void) {
	Serial.begin(9600);
	dump_nmea_value("VER", VER);
}

void log_debug(const char *str) {
	print_str(str);
}

void log_measurement(const Measurment &measurement) {
	static int _ppm=-1;
	const int ppm = measurement.value;
	if (_ppm != ppm) {
		dump_nmea_value("CO2", ppm, measurement.valid);
		_ppm = ppm;
	}
}
