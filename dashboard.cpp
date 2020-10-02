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
//#include <Fonts/FreeMonoBoldOblique12pt7b.h>

const unsigned long PLOT_DELAY = 60000;

/*
 #define LCD_CS A3 // Chip Select goes to Analog 3
 #define LCD_CD A2 // Command/Data goes to Analog 2
 #define LCD_WR A1 // LCD Write goes to Analog 1
 #define LCD_RD A0 // LCD Read goes to Analog 0

 #define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
 #include <TftSpfd5408.h> // Hardware-specific library
 */

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

static void test_chart() {
	const static Transform<0, DISPLAY_WIDTH_PX-1, MIN_SENSOR_VALUE, MAX_SENSOR_VALUE> pos2value;
	for (int i = 0; i < DISPLAY_WIDTH_PX; ++i) {
		plotter.add_measuement(pos2value(i));
		plotter.mk_point();
	}
	plotter.update();
}

static void setup_display() {
	tft.reset();
	tft.begin(TFT_ID);
	tft.setRotation(TFT_ROTATE);
}

#include <Fonts/FreeSans12pt7b.h>
void setup_ttf() {
	tft.fillScreen(BACKGROUND_COLOR);

	tft.setFont(&FreeSans12pt7b);
	tft.setTextSize(1);
	tft.setTextColor(GREY);
	tft.setCursor(5, 25);
	tft.print("CO2");
	tft.setCursor(5, 48);
	tft.print("ppm");
}

void setup_dashboard(void) {
	setup_display();
	setup_ttf();
	plotter.update();
}


#include <Fonts/FreeSans24pt7b.h>
void update_label(int ppm) {
	char str_buf[16];
	static const Transform<MIN_SENSOR_VALUE-500, MAX_SENSOR_VALUE, 0, 255> ppm2color;
	static TftLabel ppm_printer {tft, BACKGROUND_COLOR, 100, 70, 2, &FreeSans24pt7b };
	static int _ppm = -1;

	if (_ppm != ppm) {
		uint16_t color = palette_8bit(ppm2color(ppm));
		sprintf(str_buf, "%d", ppm);
		ppm_printer.print(str_buf, color);
		_ppm = ppm;
	}
}

void update_chart(const Measurment &measurement){
	static unsigned long next_plot_time = millis() + PLOT_DELAY;
	static bool test_shown = false;

	if (!test_shown && !measurement.valid) {
		test_chart();
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


void update_dashboard(const Measurment &measurement) {
//	test_chart();
	update_label(measurement.value);
	update_chart(measurement);
}

