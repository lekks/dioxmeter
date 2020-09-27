#include "config.h"
#include "dioxmeter.hpp"
#include "named_colors.h"
#include "utils.hpp"
#include "static_ring.hpp"

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

static const Transform<400, 2000, 0, 255> ppm2compr;
static const Transform<-100, 2000, 0, 255> ppm2color;

static inline uint16_t RGB(uint8_t r, uint8_t g, uint8_t b) {
	return tft.color565(r, g, b);
}

static uint16_t gr_gradient(uint8_t x) {
	uint16_t c = x * 2;
	if (c < 256)
		return RGB(c, 255, 0);
	else if (c < 512)
		return RGB(255, 511 - c, 0);
	else
		return RGB(255, 255, 255);
}

class Plotter {
	struct PlotPoint {
		uint8_t mean;
		uint8_t min;
		uint8_t max;
	};
	Adafruit_GFX &tft;
	static const int16_t xstart = 0;
	static const int16_t xend = 320;
	static const int16_t yres = 240;
	static const int16_t ymin = 0;
	static const int16_t ymax = 180;
	static const int16_t pmin = (ymax - ymin) * 400L / 2000 + ymin;

	static const Transform<0, 255, pmin, ymax> comr2coord;
	static const Transform<0, 255, 60, 255> compr2color;

	StaticRing<PlotPoint, uint16_t, xend - xstart> pts;

public:
	Plotter(Adafruit_GFX &tft) :
			tft(tft) {
	}
	;

	void add(uint8_t mean, uint8_t min, uint8_t max) {
		pts.push(PlotPoint { mean, min, max });
	}

	void plot_compr_point(int x, const PlotPoint &p) {
		int yp = comr2coord(p.mean);
		uint16_t color = gr_gradient(compr2color(p.mean));

		tft.drawLine(x, yres - yp - 1, x, yres - ymax, BLACK);
		tft.drawLine(x, yres - ymin, x, yres - yp, color);
		//Much faster, but artifats seen
//    tft.drawFastVLine(x, yres-ymax, ymax-yp-1, BLACK);
//    tft.drawFastVLine(x, yres-yp, yres-yp-1, color);
	}

	void draw_box() {
		tft.setTextSize(1);
		tft.setTextColor(CYAN);
		tft.drawLine(xstart, yres - ymax - 1, xend - 1, yres - ymax - 1, BLUE);
		tft.setCursor(xend - 46, yres - ymax - 10);
		tft.print("2000ppm");
		tft.drawLine(xstart, yres - ymax / 2 - 1, xend - 1, yres - ymax / 2 - 1,
				BROWN);

		tft.setCursor(xstart + 1, yres - ymin - 10);
		tft.print("5h:20min");

		for (int x = 0; x < xend; x++) {
			int i = x - xend;
			if (i % 60 == 0) {
				tft.drawFastVLine(x, yres - ymax, ymax - ymin, BROWN);
			}
		}

	}

	void update() {
		int points = pts.get_used();
		for (int x = 0; x < xend; x++) {
			int i = x - xend + pts.get_used();
			if (i >= 0) {
				plot_compr_point(x, *pts.get(i));
			}
		}
		draw_box();
	}

};

class TftLabel {
	int x, y;
	char sz;
	bool printed;
	char last[64];
public:
	TftLabel(int x, int y, char sz) :
			x(x), y(y), sz(sz), printed(false) {
	}
	;
	void clear() {
		if (printed) {
			tft.setCursor(x, y);
			tft.setTextSize(sz);
			tft.setTextColor(BLACK);
			tft.print(last);
			printed = false;
		}
	}

	void print(const char *txt, uint16_t color) {
		if (strcmp(txt, last) || !printed) {
			clear();
			tft.setCursor(x, y);
			tft.setTextSize(sz);
			tft.setTextColor(color);
			tft.print(txt);
			printed = true;
			strcpy(last, txt);
		}
	}

};

static void setup_display() {
	tft.reset();
	tft.begin(TFT_ID);
	tft.setRotation(TFT_ROTATE);
}

static void testColors() {
	static Transform<0, 319, 0, 255> xtoc;
	for (int i = 0; i < 320; ++i) {
		uint16_t c = gr_gradient(xtoc(i));
		tft.drawFastVLine(i, 0, 20, c);
	}
}

void dump(int ppm, unsigned long period, unsigned long imp, unsigned int cnt) {
	sprintf(str_buf, "%d:%lu/%lu/%u", ppm, period, imp, cnt);
	log_debug(str_buf);
}

static void dump(int mean, int min, int max) {
	sprintf(str_buf, "%d:%d-%du", mean, min, max);
	log_debug(str_buf);
}

Plotter plotter(tft);
void test_plot() {
	static uint8_t mock = 0;
	plotter.add(mock, mock, mock);
	mock++;
	plotter.update();
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

static Stat stat;

void setup_dashboard(void) {
	setup_display();
	setup_ttf();
	plotter.update();
	stat.reset();
}

void update_dashboard(const Measurment &measurement) {
	static TftLabel ppm_printer { 80, 0, 8 };
	static unsigned long next_plot_time = millis() + PLOT_DELAY;
	static int _ppm = -1;

	const int ppm = measurement.value;
	//testColors();
	//dump(ppm,period,imp,cnt);
	if (_ppm != ppm) {
		uint16_t color = gr_gradient(ppm2color(ppm));
		sprintf(str_buf, "%d", ppm);
		ppm_printer.print(str_buf, color);
		_ppm = ppm;
	}

	if (measurement.valid) {
		stat.add(ppm);
		unsigned long current_time = millis();
		if (current_time >= next_plot_time && stat.valid()) {
			//dump(stat.get_mean(),stat.get_min(),stat.get_max());
			plotter.add(ppm2compr(stat.get_mean()), ppm2compr(stat.get_min()),
					ppm2compr(stat.get_max()));
			plotter.update();
			stat.reset();
			next_plot_time += PLOT_DELAY;
		}
	} else {
		stat.reset();
	}
	//test_plot();
}

