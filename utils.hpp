#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <stdint.h>


template <const int16_t smin,const int16_t smax,const int16_t cmin,const int16_t cmax>
class Transform {  
public:
  int16_t operator() (int16_t s) const {
      if (s<=smin)return cmin;
      if (s>=smax)return cmax;
      return cmin+(int32_t)(s-smin)*(cmax-cmin)/(smax-smin);
  }
};

class Averager {
  uint16_t cnt;
  int32_t accum;
  int16_t min;
  int16_t max;
public:
  Averager():cnt(0),accum(0){};
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


extern unsigned long time_diff(long unsigned time1, long unsigned time2);

#endif /* UTILS_HPP_ */
