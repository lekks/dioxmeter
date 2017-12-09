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

TftSpfd5408 tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

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

static unsigned long time_diff(long unsigned time1, long unsigned time2)
{
  if(time2>=time2)
    return time2-time1;
  else
    return (ULONG_MAX-time1)+time2+1;
}


static int calc_q(unsigned long period,unsigned long imp)
{
    int dead_time = period/251;
    return 5000*(imp-dead_time/2)/(period-dead_time);
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


void loop(void) {
  static unsigned int _cnt;
  static int _ppm;

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


