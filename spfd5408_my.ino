//#include <pin_magic.h>
#include <registers.h>
#include <SPFD5408_Adafruit_GFX.h>
#include <SPFD5408_Adafruit_TFTLCD.h>
#include <SPFD5408_TouchScreen.h>

#include <limits.h>
#include <stdio.h>

// IMPORTANT: Adafruit_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// SEE RELEVANT COMMENTS IN Adafruit_TFTLCD.h FOR SETUP.

// Modified for SPFD5408 Library by Joao Lopes
// Version 0.9.2 - Rotation for Mega and screen initial

// *** SPFD5408 change -- Begin
#include <SPFD5408_Adafruit_GFX.h>    // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>
// *** SPFD5408 change -- End

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7
// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
// Adafruit_TFTLCD tft;

// -- Setup

void setup_pcint()
{
  DDRC &= ~_BV(PC5);
  //PORTC |= _BV(PC5);
  PCMSK1 |= _BV(PCINT13);
  PCICR |= _BV(PCIE1);
}

void setup_display()
{
  tft.reset();
 
  // *** SPFD5408 change -- Begin

// Original code commented

//  uint16_t identifier = tft.readID();
//
//  if(identifier == 0x9325) {
//    Serial.println(F("Found ILI9325 LCD driver"));
//  } else if(identifier == 0x9328) {
//    Serial.println(F("Found ILI9328 LCD driver"));
//  } else if(identifier == 0x7575) {
//    Serial.println(F("Found HX8347G LCD driver"));
//  } else if(identifier == 0x9341) {
//    Serial.println(F("Found ILI9341 LCD driver"));
//  } else if(identifier == 0x8357) {
//    Serial.println(F("Found HX8357D LCD driver"));
//  } else {
//    Serial.print(F("Unknown LCD driver chip: "));
//    Serial.println(identifier, HEX);
//    Serial.println(F("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
//    Serial.println(F("  #define USE_ADAFRUIT_SHIELD_PINOUT"));
//    Serial.println(F("should appear in the library header (Adafruit_TFT.h)."));
//    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
//    Serial.println(F("Also if using the breakout, double-check that all wiring"));
//    Serial.println(F("matches the tutorial."));
//    return;
//  }
//
//  tft.begin(identifier);

  // Code changed to works 
  
  tft.begin(0x9341); // SDFP5408

  tft.setRotation(1); // Need for the Mega, please changed for your choice or rotation initial
  
}


void setup(void) {
  Serial.begin(9600);

  Serial.println("Hello!");

  setup_display();
  setup_pcint();

}

unsigned long time_diff(long unsigned time1, long unsigned time2)
{
  if(time2>=time2)
    return time2-time1;
  else
    return (ULONG_MAX-time1)+time2+1;
}


volatile unsigned long period;
volatile unsigned long imp;
volatile unsigned int cnt;

SIGNAL(PCINT1_vect) //PCINT13 PC5 ADC5 
{
  static volatile bool valid=false;
  static volatile unsigned long imp_int;
  static volatile bool last_sig;
  static volatile unsigned long last_on;
  
  bool sig=PINC&_BV(PC5);
  if(last_sig != sig) {
    //unsigned long time = millis();
    unsigned long time = micros();
    if(sig) {
      if(valid) {
        period = time_diff(last_on,time);
        imp = imp_int;
        ++cnt;
      }
      valid = true;
      last_on = time;
    } else {
      imp_int = time_diff(last_on,time);
    }
    last_sig = sig;
  }
}

int calc_q(unsigned long period,unsigned long imp)
{
    int dead_time = period/251;
    return 5000*(imp-dead_time/2)/(period-dead_time);
}

unsigned int _cnt;
int _ppm;

inline uint16_t RGB(uint8_t r,uint8_t g,uint8_t b) {
  return tft.color565(r,g,b);
}


uint16_t gr_gradient(float x)
{
      uint16_t c=x*511;
      if(c<256)
        return RGB(c,255,0);
      else if(c<512)
        return RGB(255,511-c,0);
      else 
        return RGB(255,255,255);
}


void testColors()
{
  for (int i=0;i<320;++i) {
    uint16_t c = gr_gradient((float)i/319.0);
    tft.drawFastVLine(i,0, 10, c);
  }
}

float alarm_color(int16_t ppm)
{
    const int16_t min=0;
    const int16_t max=2000;
    const float r_min=0;
    const float r_max=1;
    if (ppm<=min)return r_min;
    if (ppm>=max)return r_max;
    return r_min + (ppm-min)*(r_max-r_min)/(max-min);
}

void loop(void) {
  tft.fillScreen(BLACK);
  char buf[128];
  tft.setTextColor(YELLOW);
  tft.setTextSize(12);
  testColors();
  for (;;) {
  //Serial.println("Ping!");
    if(_cnt != cnt) {
      int ppm=calc_q(period,imp);
      sprintf(buf,"%d:%lu/%lu/%u",ppm,period,imp,cnt);
      Serial.println(buf);
      Serial.println(ppm);
      if(_ppm != ppm) {
        //tft.fillRect(0,0,100,21, BLACK);
//        tft.fillScreen(BLACK);
        tft.setCursor(10, 80);
        tft.setTextColor(BLACK);      
        tft.println(_ppm);
        tft.setCursor(10, 80);
        tft.setTextColor(gr_gradient(alarm_color(ppm)));      
        tft.println(ppm);
        _ppm = ppm;
      }
      _cnt = cnt;
    }
    // delay(1000);
   }
}


