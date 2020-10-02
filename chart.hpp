/*
 * chart.hpp
 *
 *  Created on: Sep 30, 2020
 *      Author: ldir
 */

#ifndef CHART_HPP_
#define CHART_HPP_

#include "config.h"
#include "utils.hpp"
#include "named_colors.h"
#include "static_ring.hpp"

#include <Adafruit_GFX.h>    // Core graphics library


typedef uint16_t (*Palette)(uint8_t x);

class ChartBase { // @suppress("Class has a virtual method and non-virtual destructor")
protected:
	static const int16_t xmin = 0;
	static const int16_t xmax = DISPLAY_WIDTH_PX;
	static const int16_t ymin = 14;
	static const int16_t ymax = 160;

	static const int16_t pmin = (ymax - ymin) * static_cast<long int>(MIN_SENSOR_VALUE)/ MAX_SENSOR_VALUE + ymin;
	static const Transform<MIN_SENSOR_VALUE, MAX_SENSOR_VALUE, pmin, ymax> ppm2coord;

	Adafruit_GFX &tft;

public:
	ChartBase(Adafruit_GFX &tft);
	void add_measuement(int value);
	void mk_point();
	void update();
	bool valid();
	void clear();

private:
	Averager stat;
	typedef StaticRing<uint8_t, uint16_t, xmax - xmin> PointsBuffer;
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

class CustomChart: public ChartBase { // @suppress("Class has a virtual method and non-virtual destructor")

	const uint16_t BORDER_TOP_COLOR = DARK_GREY;
	const uint16_t GRID_COLOR = DARK_GREY;
	const uint16_t TEXT_COLOR = WHITE;

public:
	CustomChart(Adafruit_GFX &tft, Palette palette) :
			ChartBase(tft), palette(palette) {
	}
private:
	Palette palette;
	void plot_point(int x, int y, uint8_t value) override;
	void draw_overlay() override;

};

#endif /* CHART_HPP_ */
