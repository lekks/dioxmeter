#include "config.h"
#include "dashboard.hpp"
#include "named_colors.h"
#include "utils.hpp"
#include "static_ring.hpp"
#include "tft_utils.hpp"

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>

#include "chart.hpp"
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

const unsigned long PLOT_DELAY = 60000;

MCUFRIEND_kbv tft;


static inline uint16_t RGB(uint8_t r, uint8_t g, uint8_t b) {
	return tft.color565(r, g, b);
}

static uint16_t palette_8bit(uint8_t x) {
	uint16_t c = x * 2;
	if (c < 256)
		return RGB(c, 255, 0);
	else if (c < 512)
		return RGB(255, 511 - c, 0);
	else
		return RGB(255, 255, 255);
}

static CustomChart plotter(tft, palette_8bit);

static void test_palette() {
	const static Transform<0, DISPLAY_WIDTH_PX-1, 0, 255> xtoc;
	for (int i = 0; i < DISPLAY_WIDTH_PX; ++i) {
		uint16_t c = palette_8bit(xtoc(i));
		tft.drawFastVLine(i, 0, 20, c);
	}
}

static void setup_display() {
	tft.reset();
	tft.begin(TFT_ID);
	tft.setRotation(TFT_ROTATE);
}

void setup_ttf() {
	tft.fillScreen(BACKGROUND_COLOR);

	tft.setFont(&FreeSans18pt7b);
	tft.setTextSize(1);
	tft.setTextColor(WHITE);
	tft.setCursor(5, 28);
	tft.print("CO2");
}

void setup_dashboard(void) {
	setup_display();
	setup_ttf();
	plotter.update();
}


void update_label(int ppm) {
	char str_buf[16];
	static const Transform<MIN_CHART_VALUE-500, MAX_CHART_VALUE, 0, MAX_CHART_COLOR> ppm2color;
	static TftLabelFill ppm_printer {tft, BACKGROUND_COLOR, 100, 68, 2, &FreeSans24pt7b };
	static int _ppm = -1;

	if (_ppm != ppm) {
		uint16_t color = palette_8bit(ppm2color(ppm));
		sprintf(str_buf, "%4d", ppm);
		ppm_printer.print(str_buf, color);
		_ppm = ppm;
	}
}

void update_chart(const Measurment &measurement){
	static unsigned long next_plot_time = millis() + PLOT_DELAY;
	static bool test_shown = false;

	if (!test_shown && !measurement.valid) {
		plotter.fill_test();
		test_shown = true;
	}

	if (measurement.valid) {
		if (test_shown) {
			plotter.clear();
			test_shown = false;
		}
		plotter.add_measuement(measurement.value);
		unsigned long current_time = millis();
		if (current_time >= next_plot_time && plotter.valid()) {
			plotter.mk_point();
			plotter.update();
			next_plot_time += PLOT_DELAY;
		}
	}
}


void update_calibration_status(){
		static TftLabelFill status_printer {tft, BACKGROUND_COLOR, 40, 120, 1, &FreeSans18pt7b };
		status_printer.print("Calibrating",BROWN );
}

void update_dashboard(const Measurment &measurement) {
	update_label(measurement.value);
	update_chart(measurement);
}

