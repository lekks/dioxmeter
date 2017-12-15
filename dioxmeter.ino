#include <Adafruit_GFX.h>    // Core graphics library
#include <TftSpfd5408.h> // Hardware-specific library

#include <limits.h>
#include <stdio.h>

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

unsigned long PLOT_DELAY=2000;

TftSpfd5408 tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

class TftPrint {
  int x,y;
  char sz;
  bool printed;
  int last_val;
public:
  TftPrint(int x,int y,char sz):x(x),y(y),sz(sz),printed(false){};
  void clear() {
    if(printed) {
      tft.setCursor(x, y);
      tft.setTextSize(sz);  
      tft.setTextColor(BLACK);  
      tft.print(last_val);
      printed = false; 
    }
  }
  void print(int val,uint16_t color) {
    if(val!=last_val || !printed) {
      clear();
      tft.setCursor(x, y);
      tft.setTextSize(sz);  
      tft.setTextColor(color);  
      tft.print(val);
      printed = true;
      last_val=val; 
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

static uint16_t gr_gradient(float x)
{
      uint16_t c=x*511;
      if(c<256)
        return RGB(c,255,0);
      else if(c<512)
        return RGB(255,511-c,0);
      else 
        return RGB(255,255,255);
}



static void testColors()
{
  for (int i=0;i<320;++i) {
    uint16_t c = gr_gradient((float)i/319.0);
    tft.drawFastVLine(i,0, 20, c);
  }
}

static float alarm_color(int16_t ppm)
{
    const int16_t min=-100;
    const int16_t max=2000;
    const float r_min=0;
    const float r_max=1;
    if (ppm<=min)return r_min;
    if (ppm>=max)return r_max;
    return r_min + (ppm-min)*(r_max-r_min)/(max-min);
}

class Plotter {
    const int16_t min=400;
    const int16_t max=2000;
    const int16_t pos_min=0;
    const int16_t pos_max=220;
    const int16_t xmin=300;
    const int16_t xmax=319;
    const int16_t ymax=240;
public:
  int16_t ypos (int16_t ppm) {
      if (ppm<=min)return pos_min;
      if (ppm>=max)return pos_max;
      return pos_min + (int32_t)(ppm-min)*(pos_max-pos_min)/(max-min);
  }
  void plot(int16_t ppm, uint16_t color) {
    for(int y=pos_min;y<pos_max;y++) {
      if(y>ypos(ppm))
        tft.drawLine(xmin, ymax-y, xmax, ymax-y, BLACK);
      else
        tft.drawLine(xmin, ymax-y, xmax, ymax-y, color);
    }
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


struct PlotPoint {
  uint8_t val;
  uint8_t min;
  uint8_t max;
};

class PlotPoints {
  PlotPoint queue[320];
public:
  PlotPoint get(int i) {
    return queue[i];
  }
  void put(PlotPoint &p) {
    queue[0]=p;
  }
};

PlotPoints pts;

void loop(void) {
  static unsigned int _cnt;
  
  static int _ppm;
  tft.fillScreen(BLACK);
  tft.setTextColor(YELLOW);
  tft.setTextSize(12);
  
  testColors();

  TftPrint y_printer(10,180,5);
  TftPrint ppm_printer(10,80,12);
  Plotter plotter;
  unsigned long next_plot_time = millis()+PLOT_DELAY;
  int ppm = 0;
  for (;;) {
    if(_cnt != cnt) {
      ppm=calc_ppm(period,imp);
      dump(ppm,period,imp,cnt);
      _cnt = cnt;
    }
    if(_ppm != ppm) {
      //tft.fillRect(10,80,200,300, BLACK);
      //tft.fillScreen(BLACK);
      uint16_t color=gr_gradient(alarm_color(ppm));
      ppm_printer.print(ppm,color);
      plotter.plot(ppm,color);
      _ppm = ppm;
      PlotPoint p={12};
      pts.put(p);
    }

    unsigned long current_time = millis();
    if(current_time >= next_plot_time) {
      Serial.println("Plot");
      next_plot_time+=PLOT_DELAY;
    }
    // delay(1000);
   }
}


