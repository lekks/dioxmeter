/*
 * tft_utils.hpp
 *
 *  Created on: Sep 27, 2020
 *      Author: ldir
 */

#ifndef TFT_UTILS_HPP_
#define TFT_UTILS_HPP_

#include <Adafruit_GFX.h>    // Core graphics library
#include <string.h>


class TftLabel {
	int x, y;
	char sz;
	bool printed;
	char last[64];
	Adafruit_GFX& tft;
	unsigned int background;
public:
	TftLabel(Adafruit_GFX& tft, uint16_t background, int x, int y, char sz) :
			tft(tft), background(background), x(x), y(y), sz(sz), printed(false) {
	}
	;
	void clear() {
		if (printed) {
			tft.setCursor(x, y);
			tft.setTextSize(sz);
			tft.setTextColor(background);
			tft.print(last);
			printed = false;
		}
	}

	void print(const char *txt, unsigned int color) {
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



#endif /* TFT_UTILS_HPP_ */
