/*
 * tft_utils.hpp
 *
 *  Created on: Sep 27, 2020
 *      Author: ldir
 */

#ifndef TFT_UTILS_HPP_
#define TFT_UTILS_HPP_

#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <string.h>


//Fast for small and rough fonts
class TftLabelRepaint {
	Adafruit_GFX& tft;
	unsigned int background;
	int x, y;
	char sz;
	const GFXfont *font;
	bool printed;
	char last[64];

public:
	TftLabelRepaint(Adafruit_GFX& tft, uint16_t background, int x, int y, char sz, const GFXfont *font = nullptr) :
			tft(tft), background(background), x(x), y(y), sz(sz), font(font), printed(false) {
	}
	;
	void clear() {
		if (printed) {
			tft.setFont(font);
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
			tft.setFont(font);
			tft.setCursor(x, y);
			tft.setTextSize(sz);
			tft.setTextColor(color);
			tft.print(txt);
			printed = true;
			strcpy(last, txt);
		}
	}
};

//Could be faster for fine fonts
class TftLabelFill {
	Adafruit_GFX& tft;
	unsigned int background;
	int x, y;
	char sz;
	const GFXfont *font;
	bool printed;
	int16_t  x1, y1;
	uint16_t w, h;

public:
	TftLabelFill(Adafruit_GFX& tft, uint16_t background, int x, int y, char sz, const GFXfont *font = nullptr) :
			tft(tft), background(background), x(x), y(y), sz(sz), font(font), printed(false) {
	}

	void print(const char *txt, unsigned int color) {
		tft.setFont(font);
		tft.setCursor(x, y);
		tft.setTextSize(sz);
		tft.setTextColor(color);
		if (printed) {
			tft.fillRect(x1, y1, w+1, h+1, background);
		}
		// Some delay is needed between fillRect and print or artifacts appeared. Let it be getTextBounds
		// If delay is not enough use delay(1);
		tft.getTextBounds(txt, x, y, &x1, &y1, &w, &h);
		tft.print(txt);
		printed = true;
	}
};


#endif /* TFT_UTILS_HPP_ */
