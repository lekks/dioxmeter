/*
 * chart.hpp
 *
 *  Created on: Sep 30, 2020
 *      Author: ldir
 */

#ifndef CHART_HPP_
#define CHART_HPP_

#include "utils.hpp"
#include <Adafruit_GFX.h>    // Core graphics library


typedef	uint16_t (*Palette)(uint8_t x);

class Chart {
	Palette palette;
	Adafruit_GFX &tft;
	static const int16_t xstart = 0;
	static const int16_t xend = 320;
	static const int16_t yres = 240;
	static const int16_t ymin = 0;
	static const int16_t ymax = 180;
	static const int16_t pmin = (ymax - ymin) * 400L / 2000 + ymin;

	static const Transform<0, 255, pmin, ymax> comr2coord;
	static const Transform<0, 255, 60, 255> compr2color;
	Averager stat;

	typedef uint8_t PlotPoint;
	StaticRing<PlotPoint, uint16_t, xend - xstart> pts;
	static const Transform<400, 2000, 0, 255> ppm2compr;


	void plot_compr_point(int x, const PlotPoint &p) {
		int yp = comr2coord(p);
		auto color = palette(compr2color(p));

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

	void draw_chart() {
		for (int x = 0; x < xend; x++) {
			int i = x - xend + pts.get_used();
			if (i >= 0) {
				plot_compr_point(x, *pts.get(i));
			}
		}
	}


public:
	Chart(Adafruit_GFX &tft, Palette palette) :
			tft(tft),palette(palette) {
		stat.reset();
	}

	void add_measuement(int16_t value) {
		stat.add(value);
	}

	void mk_point() {
		pts.push( ppm2compr(stat.get_mean()) );
		stat.reset();
		update();
	}

	void update() {
		draw_chart();
		draw_box();
	}

	bool valid() {
		return stat.valid();
	}
};




#endif /* CHART_HPP_ */
