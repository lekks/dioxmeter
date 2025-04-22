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

#include <stdint.h>
#include <Adafruit_GFX.h>    // Core graphics library


typedef uint16_t (*Palette)(uint8_t x);

class ChartBase { // @suppress("Class has a virtual method and non-virtual destructor")
protected:
	static const int16_t left_pos = 0;
	static const int16_t right_pos = DISPLAY_WIDTH_PX;
	static const int16_t bottom_pos = 14;
	static const int16_t top_pos = 160;

	static const int16_t min_chart_pos = (top_pos - bottom_pos) * static_cast<long int>(MIN_CHART_VALUE)/ MAX_CHART_VALUE + bottom_pos;
	static const Transform<MIN_CHART_VALUE, MAX_CHART_VALUE, min_chart_pos, top_pos> ppm2coord;

	Adafruit_GFX &tft;

public:
	ChartBase(Adafruit_GFX &tft);
	void add_measuement(int value);
	void mk_point();
	void update();
	bool valid();
	void clear();
	void fill_test();
private:
	Averager stat;
	typedef StaticRing<uint8_t, uint16_t, right_pos - left_pos> PointsBuffer;
	static PointsBuffer pts;
	static const Transform<MIN_CHART_VALUE, MAX_CHART_VALUE, 0, UINT8_MAX> ppm2compr;
	static const Transform<0, UINT8_MAX, min_chart_pos, top_pos> comr2coord;
	static const Transform<0, UINT8_MAX, MIN_CHART_COLOR, MAX_CHART_COLOR> compr2color;

	virtual void plot_point(int x, int y, uint8_t value) =0;
	virtual void draw_underlay() {
	}
	virtual void draw_overlay() {
	}

	void draw_chart();
};

class CustomChart: public ChartBase { // @suppress("Class has a virtual method and non-virtual destructor")

	const int line_width=0; // 0 to flood
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
