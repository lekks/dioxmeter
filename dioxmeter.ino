#define VER "1.3"
#include "config.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>

#include "static_ring.hpp"

#define MAX_STRING_LENGHT 32 //Doesn`t work with big values, I can`t understand why

/*
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
#include <TftSpfd5408.h> // Hardware-specific library
*/

#ifdef DHTPIN
#include "DHT.h"
DHT dht(DHTPIN, DHTTYPE);
#endif

MCUFRIEND_kbv tft;

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define BROWN   0x8000

unsigned long PLOT_DELAY=60000;
unsigned long HEATING_DELAY=180000;
unsigned long DHT_DELAY=10000;

char str_buf[MAX_STRING_LENGHT+1];

template <const int16_t smin,const int16_t smax,const int16_t cmin,const int16_t cmax>
class Transform {  
public:
  int16_t operator() (int16_t s) const {
      if (s<=smin)return cmin;
      if (s>=smax)return cmax;
      return cmin+(int32_t)(s-smin)*(cmax-cmin)/(smax-smin);
  }
};

static const Transform <400,2000,0,255>ppm2compr;
static const Transform <-100,2000,0,255>ppm2color;
static const Transform <0,255,60,255>compr2color;

class Stat {
  uint16_t cnt;
  int32_t accum;
  int16_t min;
  int16_t max;
public:
  Stat():cnt(0),accum(0){};
  void reset() {
    cnt = 0;
    accum = 0;
  }

  void add(int16_t x) {
    if (!cnt) {
      min=max=x;
    } else {
      if(x>max)max=x;
      if(x<min)min=x;
    }
    accum+=x;
    ++cnt;
  }

  bool valid() {return cnt>0;}

  int16_t get_mean() {
    if(cnt) return accum/cnt;
    else return 0;
  }
  
  int16_t get_min() {
    if(cnt) return min;
    else return 0;
  }

  int16_t get_max() {
    if(cnt) return max;
    else return 0;
  }
};

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

int  makeNmeaSentense(char* buffer, int size, const char * sentense, ...)
{
  int ret;
  if (*sentense != '$') return -1;
  va_list arg; 
  va_start(arg, sentense);
  ret = vsnprintf(buffer, size, sentense,arg);
  va_end(arg);
  if(buffer[ret-1] != '*') return -1;
  if(ret+5>size) return -1;
  unsigned char crc=0;
  char *data = buffer+1;
  while (*data && *data!='*') {
    crc ^=*data++;
  }
  ret+=snprintf(&buffer[ret],5,"%02X\r\n",crc);
  return ret;
}

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
#ifdef DHTPIN
  dht.begin();
#endif
}

static inline uint16_t RGB(uint8_t r,uint8_t g,uint8_t b) {
  return tft.color565(r,g,b);
}

static uint16_t gr_gradient(uint8_t x)
{
      uint16_t c=x*2;
      if(c<256)
        return RGB(c,255,0);
      else if(c<512)
        return RGB(255,511-c,0);
      else 
        return RGB(255,255,255);
}

static void testColors()
{
  static Transform<0,319,0,255> xtoc;
  for (int i=0;i<320;++i) {
    uint16_t c = gr_gradient(xtoc(i));
    tft.drawFastVLine(i,0, 20, c);
  }
}


class Plotter {
    struct PlotPoint {
      uint8_t mean;
      uint8_t min;
      uint8_t max;
    };
    static const int16_t xstart=0;
    static const int16_t xend=320;
    static const int16_t yres=240;
    static const int16_t ymin=0;
    static const int16_t ymax=180;
    static const int16_t pmin=(ymax-ymin)*400L/2000+ymin;

    static const Transform<0,255,pmin,ymax> comr2coord;
    
    StaticRing<PlotPoint,uint16_t,320> pts;
    
public:

  void add(uint8_t mean,uint8_t min,uint8_t max) {
    pts.push(PlotPoint{mean,min,max});
  }

  void plot_compr_point(int x,const PlotPoint& p) {
    int yp=comr2coord(p.mean);
    uint16_t color=gr_gradient(compr2color(p.mean));

    tft.drawLine(x, yres-yp-1, x, yres-ymax, BLACK);
    tft.drawLine(x, yres-ymin, x, yres-yp, color);
    //Much faster, but artifats seen
//    tft.drawFastVLine(x, yres-ymax, ymax-yp-1, BLACK);
//    tft.drawFastVLine(x, yres-yp, yres-yp-1, color);
  }

  void draw_box() {
    tft.setTextSize(1);  
    tft.setTextColor(CYAN);  
    tft.drawLine(xstart, yres-ymax-1, xend-1, yres-ymax-1, BLUE);
    tft.setCursor(xend-46,yres-ymax-10);
    tft.print("2000ppm");
    tft.drawLine(xstart, yres-ymax/2-1, xend-1, yres-ymax/2-1, BROWN);

    tft.setCursor(xstart+1,yres-ymin-10);
    tft.print("5h:20min");

    for (int x=0;x<xend;x++) {
      int i = x-xend;
      if(i%60 == 0) {
          tft.drawFastVLine(x, yres-ymax, ymax-ymin, BROWN);
      }
    }

  }

  void plot() {
    int points = pts.get_used();
    for (int x=0;x<xend;x++) {
      int i = x-xend+pts.get_used();
      if(i>=0) {
        plot_compr_point(x,*pts.get(i));
      }
    }
    draw_box();
  }

};


static unsigned long time_diff(long unsigned time1, long unsigned time2)
{
  if(time2>=time2)
    return time2-time1;
  else
    return (ULONG_MAX-time1)+time2+1;
}

static int calc_ppm(unsigned long period,unsigned long imp)
{
    int dead_time = period/251;
    return 5000*(imp-dead_time/2)/(period-dead_time);
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

Plotter plotter;
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

#ifdef DHTPIN
  TftPrint ppm_printer(65,5,6);
  TftPrint hum_printer(220,15,3);
#else
  TftPrint ppm_printer(80,0,8);
#endif
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
#ifdef DHTPIN
      //Right after measure of CO2, HDT may use software delay measurements and IRQ may occure
      if( current_time >= next_dht_time ) {
        float hm = dht.readHumidity();
        if(isnan(hm))
          dump_nmea_value("ERR","ERR_HUM");
        hum = isnan(hm)? -1: int(hm+0.5);
        next_dht_time += DHT_DELAY;
      }
#endif
      ppm=calc_ppm(period,imp);
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
#ifdef DHTPIN
    if(_hum != hum) {
      if (hum >= 0) {
        sprintf(str_buf,"H=%i%%",hum);
        hum_printer.print(str_buf,WHITE);        
        dump_nmea_value("HUM",hum);
      }
      _hum = hum;
    }
#endif
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


