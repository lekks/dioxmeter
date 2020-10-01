#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <stdint.h>

template<const int16_t smin, const int16_t smax, const int16_t cmin, const int16_t cmax>
class Transform {
public:
	int16_t operator()(int16_t s) const {
		if (s <= smin)
			return cmin;
		if (s >= smax)
			return cmax;
		return cmin + (int32_t)(s - smin) * (cmax - cmin) / (smax - smin);
	}
};

class Averager {
	unsigned int cnt;
	long int accum;
	int min_val;
	int max_val;
public:
	Averager() :
			cnt(0), accum(0), min_val(0), max_val(0) {
	};

	void reset() {
		cnt = 0;
		accum = 0;
	}

	void add(int x) {
		if (!cnt) {
			min_val = max_val = x;
		} else {
			if (x > max_val)
				max_val = x;
			if (x < min_val)
				min_val = x;
		}
		accum += x;
		++cnt;
	}

	bool valid() {
		return cnt > 0;
	}

	int16_t get_mean() {
		if (cnt)
			return accum / cnt;
		else
			return 0;
	}

	int16_t get_min() {
		if (cnt)
			return min_val;
		else
			return 0;
	}

	int16_t get_max() {
		if (cnt)
			return max_val;
		else
			return 0;
	}
};

extern unsigned long time_diff(long unsigned time1, long unsigned time2);

#endif /* UTILS_HPP_ */
