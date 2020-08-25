#define VER "1.3"
#include "config.h"
#include "utils.hpp"
#include "plotter.hpp"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>
//#include <Fonts/FreeMonoBoldOblique12pt7b.h>
//#include <Fonts/FreeSerif9pt7b.h>


#define MAX_STRING_LENGHT 32 //Doesn`t work with big values, I can`t understand why

/*
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
#include <TftSpfd5408.h> // Hardware-specific library
*/

MCUFRIEND_kbv tft;


unsigned long PLOT_DELAY=60000;
unsigned long HEATING_DELAY=180000;
unsigned long DHT_DELAY=10000;

char str_buf[MAX_STRING_LENGHT+1];

static const Transform <400,2000,0,255>ppm2compr;
static const Transform <-100,2000,0,255>ppm2color;


class TftPrint {
  int x,y;
  char sz;
  bool printed;
  char last[64];
public:
  TftPrint(int x,int y,char sz):x(x),y(y),sz(sz),printed(false){};
  void clear() {
    if(printed) {
      tft.setCursor(x, y);
      tft.setTextSize(sz);  
      tft.setTextColor(BLACK);  
      tft.print(last);
      printed = false; 
    }
  }

  void print(const char *txt,uint16_t color) {
    if(strcmp(txt,last) || !printed) {
      clear();
      tft.setCursor(x, y);
      tft.setTextSize(sz);  
      tft.setTextColor(color);  
      tft.print(txt);
      printed = true;
      strcpy(last,txt);
    }
  }

};


static void setup_pcint()
{
  DDRC &= ~_BV(PC5); //AIN
  //PORTC |= _BV(PC5);
  PCMSK1 |= _BV(PCINT13);
  PCICR |= _BV(PCIE1);
}

static void setup_display()
{
  tft.reset();
  tft.begin(TFT_ID); 
  tft.setRotation(TFT_ROTATE);
}


void setup(void) {
  Serial.begin(9600);
  setup_display();
  setup_pcint();
}


static void testColors()
{
  static Transform<0,319,0,255> xtoc;
  for (int i=0;i<320;++i) {
    uint16_t c = gr_gradient(xtoc(i));
    tft.drawFastVLine(i,0, 20, c);
  }
}

inline uint16_t RGB(uint8_t r,uint8_t g,uint8_t b) {
  return tft.color565(r,g,b);
}

static unsigned long time_diff(long unsigned time1, long unsigned time2)
{
  if(time2>=time2)
    return time2-time1;
  else
    return (ULONG_MAX-time1)+time2+1;
}


static volatile unsigned long period;
static volatile unsigned long imp;
static volatile unsigned int cnt;

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

static void dump(int ppm,unsigned long period,unsigned long imp,unsigned int cnt)
{
  sprintf(str_buf,"%d:%lu/%lu/%u",ppm,period,imp,cnt);
  Serial.println(str_buf);
}

static void dump(int mean,int min, int max)
{
  sprintf(str_buf,"%d:%d-%du",mean,min,max);
  Serial.println(str_buf);
}

Plotter plotter(tft);
void test_plot(){
  static uint8_t mock=0;
  plotter.add(mock,mock,mock);
  mock++;
  plotter.plot();  
}

void dump_nmea_value(const char* name, int value) {
  makeNmeaSentense(str_buf,MAX_STRING_LENGHT,"$%s,%d*",name,value);
  Serial.print(str_buf);
}

void dump_nmea_value(const char* name, const char* value) {
  makeNmeaSentense(str_buf,MAX_STRING_LENGHT,"$%s,%s*",name,value);
  Serial.print(str_buf);
}

char msg_buf[MAX_STRING_LENGHT];
void loop(void) {
  dump_nmea_value("VER",VER);
  tft.fillScreen(BLACK);

  static unsigned int _cnt;  
  static int _ppm;
  static int _hum=-1;
  //testColors();
  char buf[32];

  TftPrint ppm_printer(80,0,8);
//  tft.setFont(&FreeMonoBoldOblique12pt7b);
  tft.setTextSize(3);  
  tft.setTextColor(GREEN);  
  tft.setCursor(5,2);
  tft.print("CO2");
  tft.setCursor(5,24);
  tft.print("ppm");
 
  unsigned long next_plot_time = millis()+PLOT_DELAY;
  unsigned long next_dht_time = millis()+DHT_DELAY;
  bool heated = false;
  
  int ppm = 0;
  int hum=-1;
  
  Stat stat;
  plotter.plot();  
  for (;;) {
    unsigned long current_time = millis();

    if(_cnt != cnt) {
      ppm=calc_ppm(period,imp, 5000);
      stat.add(ppm);
      //dump(ppm,period,imp,cnt);
      _cnt = cnt;
    }
    if(_ppm != ppm) {
      uint16_t color=gr_gradient(ppm2color(ppm));
      sprintf(str_buf,"%d",ppm);
      ppm_printer.print(str_buf,color);
      dump_nmea_value("CO2",ppm);
      _ppm = ppm;
    }

    if(!heated) {
      if ( current_time < HEATING_DELAY ) {
        stat.reset();
        next_plot_time = current_time;
      } else 
        heated = true;
    } else if( current_time >= next_plot_time && stat.valid() ) {
      //dump(stat.get_mean(),stat.get_min(),stat.get_max());
      plotter.add(ppm2compr(stat.get_mean()),ppm2compr(stat.get_min()),ppm2compr(stat.get_max()));
      plotter.plot();  
      stat.reset();
      next_plot_time+=PLOT_DELAY;
    }
    //test_plot();
   }
}
