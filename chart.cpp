/*
 * chart.cpp
 *
 *  Created on: Oct 1, 2020
 *      Author: ldir
 */

#include "logging.hpp"
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
	pts.push(ppm2compr(stat.get_trimmed_mean()));
	stat.reset();
}

void ChartBase::clear() {
	stat.reset();
	pts.drop(pts.get_used());
	tft.fillRect(left_pos, DISPLAY_HEIGHT_PX - top_pos, right_pos-left_pos, top_pos-bottom_pos, BACKGROUND_COLOR);
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
		for (int x = left_pos; x < right_pos; x++) {
			int i = x - right_pos + pts.get_used();
			if (i >= 0) {
				uint8_t p = *pts.get(i);
				plot_point(x, comr2coord(p),
						static_cast<uint8_t>(compr2color(p)));
			}
		}
	}
}

void ChartBase::fill_test() {
	const int values = right_pos - left_pos;
	const Transform<0, values, MIN_CHART_VALUE, MAX_CHART_VALUE> pos2value;
	for (int i = 0; i < values; ++i) {
		add_measuement(pos2value(i));
		mk_point();
	}
	update();
	char str_buf[16];
	sprintf(str_buf, "%d", min_chart_pos);
	log_debug(str_buf);
}


void CustomChart::draw_overlay() {
	const char x_label_yshift = -3;
	const char y_label_yshift = 10;
	char str_buf[16];
	tft.setFont();
	tft.setTextSize(1);
	tft.setTextColor(TEXT_COLOR);

	for (int x = left_pos; x < right_pos; x++) {
		int i = x - right_pos;
		if (i % 60 == 0) {
			tft.drawFastVLine(x, DISPLAY_HEIGHT_PX - top_pos, top_pos - bottom_pos, GRID_COLOR);

			tft.setCursor(x-8, DISPLAY_HEIGHT_PX - bottom_pos - x_label_yshift);
			sprintf(str_buf, "%d h", i/60);
			tft.print(str_buf);
		}
	}

	tft.drawLine(left_pos, DISPLAY_HEIGHT_PX - top_pos - 1, right_pos - 1, DISPLAY_HEIGHT_PX - top_pos - 1, BORDER_TOP_COLOR);
	tft.setCursor(left_pos + 2, DISPLAY_HEIGHT_PX - top_pos - y_label_yshift);
	sprintf(str_buf, "%d ppm", MAX_CHART_VALUE);
	tft.print(str_buf);

	int y_middle=ppm2coord(MAX_CHART_VALUE/2);
	tft.drawLine(left_pos, DISPLAY_HEIGHT_PX-y_middle, right_pos - 1, DISPLAY_HEIGHT_PX-y_middle, GRID_COLOR);
	tft.setCursor(left_pos + 2, DISPLAY_HEIGHT_PX-y_middle - y_label_yshift);
	sprintf(str_buf, "%d ppm", MAX_CHART_VALUE/2);
	tft.print(str_buf);

	tft.setCursor(right_pos-6, DISPLAY_HEIGHT_PX - bottom_pos - x_label_yshift);
	tft.print("0");
}

void CustomChart::plot_point(int x, int y, uint8_t value) {
	auto color = palette(value);

	tft.drawLine(x, DISPLAY_HEIGHT_PX - y - 1, x, DISPLAY_HEIGHT_PX - top_pos, BACKGROUND_COLOR);
	if(line_width) {
		const int y2 = max(y-20,bottom_pos);
		tft.drawLine(x, DISPLAY_HEIGHT_PX - y2, x, DISPLAY_HEIGHT_PX - y, color);
		tft.drawLine(x, DISPLAY_HEIGHT_PX - bottom_pos-1, x, DISPLAY_HEIGHT_PX - y2-1, BACKGROUND_COLOR);
	} else {
		tft.drawLine(x, DISPLAY_HEIGHT_PX - bottom_pos-1, x, DISPLAY_HEIGHT_PX - y, color);
	}

	//	tft.drawLine(x, DISPLAY_HEIGHT_PX - bottom_pos-1, x, DISPLAY_HEIGHT_PX - y, color);


	//tft.drawFastVLine Much faster, but artifats seen
}
