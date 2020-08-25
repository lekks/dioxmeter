#include "utils.hpp"
#include "static_ring.hpp"
#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>


uint16_t gr_gradient(uint8_t x)
{
      uint16_t c=x*2;
      if(c<256)
        return RGB(c,255,0);
      else if(c<512)
        return RGB(255,511-c,0);
      else 
        return RGB(255,255,255);
}

class Plotter {
    struct PlotPoint {
      uint8_t mean;
      uint8_t min;
      uint8_t max;
    };
    Adafruit_GFX &tft;
    static const int16_t xstart=0;
    static const int16_t xend=320;
    static const int16_t yres=240;
    static const int16_t ymin=0;
    static const int16_t ymax=180;
    static const int16_t pmin=(ymax-ymin)*400L/2000+ymin;

    static const Transform<0,255,pmin,ymax> comr2coord;
    static const Transform <0,255,60,255>compr2color;
    
    StaticRing<PlotPoint,uint16_t,xend-xstart> pts;
    
public:
  Plotter(Adafruit_GFX &tft):tft(tft){ };
  
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
