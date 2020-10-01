/*
 * chart.hpp
 *
 *  Created on: Sep 30, 2020
 *      Author: ldir
 */

#ifndef CHART_HPP_
#define CHART_HPP_

#include "utils.hpp"
#include "named_colors.h"
#include "static_ring.hpp"

#include <Adafruit_GFX.h>    // Core graphics library

const int MIN_SENSOR_VALUE = 400;
const int MAX_SENSOR_VALUE = 2000;

typedef uint16_t (*Palette)(uint8_t x);

class ChartBase { // @suppress("Class has a virtual method and non-virtual destructor")
protected:
	static const int16_t xstart = 0;
	static const int16_t xend = 320;
	static const int16_t yres = 240;
	static const int16_t ymin = 0;
	static const int16_t ymax = 180;
	static const int16_t pmin = (ymax - ymin) * 400L / 2000 + ymin;

	Adafruit_GFX &tft;

public:
	ChartBase(Adafruit_GFX &tft);
	void add_measuement(int value);
	void mk_point();
	void update();
	bool valid();

private:
	Averager stat;
	typedef StaticRing<uint8_t, uint16_t, xend - xstart> PointsBuffer;
	static PointsBuffer pts;
	static const Transform<MIN_SENSOR_VALUE, MAX_SENSOR_VALUE, 0, 255> ppm2compr;
	static const Transform<0, 255, pmin, ymax> comr2coord;
	static const Transform<0, 255, 60, 255> compr2color;

	virtual void plot_point(int x, int y, uint8_t value) =0;
	virtual void draw_underlay() {
	}
	virtual void draw_overlay() {
	}

	void draw_chart();
};

class Chart1: public ChartBase { // @suppress("Class has a virtual method and non-virtual destructor")

	const uint16_t BACKGROUND_COLOR = BLACK;
	const uint16_t BORDER_TOP_COLOR = BLUE;
	const uint16_t GRID_COLOR = BROWN;

public:
	Chart1(Adafruit_GFX &tft, Palette palette) :
			ChartBase(tft), palette(palette) {
	}
private:
	Palette palette;
	void plot_point(int x, int y, uint8_t value) override;
	void draw_overlay() override;

};

#endif /* CHART_HPP_ */
