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
}

void ChartBase::clear() {
	stat.reset();
	pts.drop(pts.get_used());
	tft.fillRect(xmin, DISPLAY_HEIGHT_PX - ymax, xmax-xmin, ymax-ymin, BACKGROUND_COLOR);
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
		for (int x = xmin; x < xmax; x++) {
			int i = x - xmax + pts.get_used();
			if (i >= 0) {
				uint8_t p = *pts.get(i);
				plot_point(x, comr2coord(p),
						static_cast<uint8_t>(compr2color(p)));
			}
		}
	}
}

void CustomChart::draw_overlay() {
	const char x_label_yshift = -3;
	const char y_label_yshift = 10;
	char str_buf[16];
	tft.setFont();
	tft.setTextSize(1);
	tft.setTextColor(TEXT_COLOR);

	for (int x = xmin; x < xmax; x++) {
		int i = x - xmax;
		if (i % 60 == 0) {
			tft.drawFastVLine(x, DISPLAY_HEIGHT_PX - ymax, ymax - ymin, GRID_COLOR);

			tft.setCursor(x-8, DISPLAY_HEIGHT_PX - ymin - x_label_yshift);
			sprintf(str_buf, "%d h", i/60);
			tft.print(str_buf);
		}
	}

	tft.drawLine(xmin, DISPLAY_HEIGHT_PX - ymax - 1, xmax - 1, DISPLAY_HEIGHT_PX - ymax - 1, BORDER_TOP_COLOR);
	tft.setCursor(xmin + 2, DISPLAY_HEIGHT_PX - ymax - y_label_yshift);
	sprintf(str_buf, "%d ppm", MAX_SENSOR_VALUE);
	tft.print(str_buf);

	int y_middle=ppm2coord(MAX_SENSOR_VALUE/2);
	tft.drawLine(xmin, DISPLAY_HEIGHT_PX-y_middle, xmax - 1, DISPLAY_HEIGHT_PX-y_middle, GRID_COLOR);
	tft.setCursor(xmin + 2, DISPLAY_HEIGHT_PX-y_middle - y_label_yshift);
	sprintf(str_buf, "%d ppm", MAX_SENSOR_VALUE/2);
	tft.print(str_buf);

	tft.setCursor(xmax-6, DISPLAY_HEIGHT_PX - ymin - x_label_yshift);
	tft.print("0");
}

void CustomChart::plot_point(int x, int y, uint8_t value) {
	auto color = palette(value);

	tft.drawLine(x, DISPLAY_HEIGHT_PX - y - 1, x, DISPLAY_HEIGHT_PX - ymax, BACKGROUND_COLOR);
	tft.drawLine(x, DISPLAY_HEIGHT_PX - ymin-1, x, DISPLAY_HEIGHT_PX - y, color);
	//tft.drawFastVLine Much faster, but artifats seen
}
