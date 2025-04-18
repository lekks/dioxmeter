#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <stdint.h>
#include <limits.h>

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
	Averager()
	  : cnt(0), accum(0),
	    min_val(INT_MAX),
	    max_val(INT_MIN) {
	}

	void reset() {
		cnt = 0;
		accum = 0;
		min_val = INT_MAX;
		max_val = INT_MIN;
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

	bool valid() const {
		return cnt > 0;
	}

	int get_mean() const {
		if (cnt)
			return static_cast<int>(accum / cnt);
		else
			return 0;
	}

	int get_trimmed_mean() const {
		if (cnt >= 3) {
			// Remove min and max values from the accumulator
			return static_cast<int>((accum - min_val - max_val) / (cnt - 2));
		} else if (cnt > 0) {
			// Not enough values to trim, return regular mean
			return static_cast<int>(accum / cnt);
		} else {
			return 0;
		}
	}

	int get_min() const {
		if (cnt)
			return min_val;
		else
			return 0;
	}

	int get_max() const {
		if (cnt)
			return max_val;
		else
			return 0;
	}
};

extern unsigned long time_diff(long unsigned time1, long unsigned time2);

#endif /* UTILS_HPP_ */
