#include <Adafruit_GFX.h>    // Core graphics library
#include <TftSpfd5408.h> // Hardware-specific library

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "static_ring.hpp"

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

unsigned long PLOT_DELAY=60000;

TftSpfd5408 tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

template <const int16_t smin,const int16_t smax,const int16_t cmin,const int16_t cmax>
class Transform {  
public:
  int16_t operator() (int16_t s) {
      if (s<=smin)return cmin;
      if (s>=smax)return cmax;
      return cmin+(int32_t)(s-smin)*(cmax-cmin)/(smax-smin);
  }
};

static Transform <400,2000,0,255>ppm2compr;
static Transform <-100,2000,0,255>ppm2color;
static Transform <0,255,60,255>compr2color;

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


static void setup_pcint()
{
  DDRC &= ~_BV(PC5);
  //PORTC |= _BV(PC5);
  PCMSK1 |= _BV(PCINT13);
  PCICR |= _BV(PCIE1);
}

static void setup_display()
{
  tft.reset();
  tft.begin(0x9341); // SDFP5408
  tft.setRotation(1); // Need for the Mega, please changed for your choice or rotation initial
}


void setup(void) {
  Serial.begin(9600);
  Serial.println("Hello!");
  setup_display();
  setup_pcint();
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

    Transform<400,2000,ymin,ymax> ppm2coord;
    Transform<0,255,ymin,ymax> comr2coord;
    
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
  }

  void draw_box() {
    tft.drawLine(xstart, yres-ymax-1, xend-1, yres-ymax-1, BLUE);
    tft.setTextSize(1);  
    tft.setTextColor(YELLOW);  
    tft.setCursor(xend-46,yres-ymax-10);
    tft.print("2000ppm");

    tft.setCursor(xstart+1,yres-ymin-10);
    tft.print("5h:20min");
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
  char buf[128];
  sprintf(buf,"%d:%lu/%lu/%u",ppm,period,imp,cnt);
  Serial.println(buf);
}

static void dump(int mean,int min, int max)
{
  char buf[128];
  sprintf(buf,"%d:%d-%du",mean,min,max);
  Serial.println(buf);
}

Plotter plotter;
void test_plot(){
  static uint8_t mock=0;
  plotter.add(mock,mock,mock);
  mock++;
  plotter.plot();  
}

void loop(void) {
  tft.fillScreen(BLACK);

  static unsigned int _cnt;  
  static int _ppm;
  //testColors();
  char buf[64];

  TftPrint ppm_printer(20,0,8);
  unsigned long next_plot_time = millis()+PLOT_DELAY;
  int ppm = 0;
  Stat stat;
  plotter.plot();  
  for (;;) {
    if(_cnt != cnt) {
      ppm=calc_ppm(period,imp);
      stat.add(ppm);
      //dump(ppm,period,imp,cnt);
      _cnt = cnt;
    }
    if(_ppm != ppm) {
      uint16_t color=gr_gradient(ppm2color(ppm));
      sprintf(buf,"%d",ppm);
      ppm_printer.print(buf,color);
      _ppm = ppm;
    }

    unsigned long current_time = millis();
    if(current_time >= next_plot_time) {
      //dump(stat.get_mean(),stat.get_min(),stat.get_max());
      plotter.add(ppm2compr(stat.get_mean()),ppm2compr(stat.get_min()),ppm2compr(stat.get_max()));
      plotter.plot();  
      stat.reset();
      next_plot_time+=PLOT_DELAY;
    }
    //test_plot();
   }
}


