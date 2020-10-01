/*
 * chart.cpp
 *
 *  Created on: Oct 1, 2020
 *      Author: ldir
 */

#include "chart.hpp"

ChartBase::PointsBuffer ChartBase::pts;

ChartBase::ChartBase(Adafruit_GFX &tft) :
		tft(tft) {
	stat.reset();
}

void ChartBase::add_measuement(int value) {
	stat.add(value);
}

void ChartBase::mk_point() {
	pts.push(ppm2compr(stat.get_mean()));
	stat.reset();
	update();
}

void ChartBase::update() {
	draw_underlay();
	draw_chart();
	draw_overlay();
}

bool ChartBase::valid() {
	return stat.valid();
}
void ChartBase::draw_chart() {
	{
		for (int x = 0; x < xend; x++) {
			int i = x - xend + pts.get_used();
			if (i >= 0) {
				uint8_t p = *pts.get(i);
				plot_point(x, comr2coord(p),
						static_cast<uint8_t>(compr2color(p)));
			}
		}
	}
}

void Chart1::draw_overlay() {
	tft.setTextSize(1);
	tft.setTextColor(CYAN);
	tft.drawLine(xstart, yres - ymax - 1, xend - 1, yres - ymax - 1,
			BORDER_TOP_COLOR);
	tft.setCursor(xend - 46, yres - ymax - 10);
	tft.print("2000ppm");
	tft.drawLine(xstart, yres - ymax / 2 - 1, xend - 1, yres - ymax / 2 - 1,
			GRID_COLOR);

	tft.setCursor(xstart + 1, yres - ymin - 10);
	tft.print("5h:20min");

	for (int x = 0; x < xend; x++) {
		int i = x - xend;
		if (i % 60 == 0) {
			tft.drawFastVLine(x, yres - ymax, ymax - ymin, GRID_COLOR);
		}
	}

}

void Chart1::plot_point(int x, int y, uint8_t value) {
	auto color = palette(value);

	tft.drawLine(x, yres - y - 1, x, yres - ymax, BACKGROUND_COLOR);
	tft.drawLine(x, yres - ymin, x, yres - y, color);
	//Much faster, but artifats seen
//    tft.drawFastVLine(x, yres-ymax, ymax-y-1, BACKGROUND_COLOR);
//    tft.drawFastVLine(x, yres-y, yres-y-1, color);
}
