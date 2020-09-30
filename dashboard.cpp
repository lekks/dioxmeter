#include "config.h"
#include "dashboard.hpp"
#include "named_colors.h"
#include "utils.hpp"
#include "static_ring.hpp"
#include "tft_utils.hpp"
#include "chart.hpp"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>
//#include <Fonts/FreeMonoBoldOblique12pt7b.h>
//#include <Fonts/FreeSerif9pt7b.h>

const int MAX_SENSOR_VALUE = 2000;
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

#define MAX_STRING_LENGHT 32 //Doesn`t work with big values, I can`t understand why
static char str_buf[MAX_STRING_LENGHT + 1];

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



static Chart plotter(tft, palette_8bit);


static void test_palette() {
	static Transform<0, 319, 0, 255> xtoc;
	for (int i = 0; i < 320; ++i) {
		uint16_t c = palette_8bit(xtoc(i));
		tft.drawFastVLine(i, 0, 20, c);
	}
}


void test_plot() {
	static uint8_t mock = 0;
	plotter.add_measuement(mock);
	mock++;
	plotter.mk_point();
}

static void setup_display() {
	tft.reset();
	tft.begin(TFT_ID);
	tft.setRotation(TFT_ROTATE);
}

void setup_ttf() {
	tft.fillScreen(BLACK);

	//  tft.setFont(&FreeMonoBoldOblique12pt7b);
	tft.setTextSize(3);
	tft.setTextColor(GREEN);
	tft.setCursor(5, 2);
	tft.print("CO2");
	tft.setCursor(5, 24);
	tft.print("ppm");

}

void setup_dashboard(void) {
	setup_display();
	setup_ttf();
	plotter.update();
}

void print_label(int ppm) {
	static const Transform<-100, 2000, 0, 255> ppm2color;
	static TftLabel ppm_printer {tft, BLACK, 80, 0, 8 };
	static int _ppm = -1;

	if (_ppm != ppm) {
		uint16_t color = palette_8bit(ppm2color(ppm));
		sprintf(str_buf, "%d", ppm);
		ppm_printer.print(str_buf, color);
		_ppm = ppm;
	}
}

void update_dashboard(const Measurment &measurement) {
	static unsigned long next_plot_time = millis() + PLOT_DELAY;

	print_label(measurement.value);

	if (measurement.valid) {
		plotter.add_measuement(measurement.value);
		unsigned long current_time = millis();
		if (current_time >= next_plot_time && plotter.valid()) {
			plotter.mk_point();
			next_plot_time += PLOT_DELAY;
		}
	}
}

