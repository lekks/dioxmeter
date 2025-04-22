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

template <typename T, int MAX_SIZE>
class MovingMedian {
private:
    T buffer[MAX_SIZE];
    int currentSize;
    int windowSize;

    // Insert value at the correct position to maintain sorted order
    void insertSorted(T value) {
        int i;
        // Find the position where the new value should be inserted
        for (i = currentSize - 1; (i >= 0 && buffer[i] > value); i--) {
            buffer[i + 1] = buffer[i]; // Shift elements to make space
        }
        buffer[i + 1] = value; // Insert the value at the correct position
    }

public:
    // Constructor
    MovingMedian(int initialWindowSize = 3) {
        if (initialWindowSize % 2 == 0) {
            // Ensure window size is odd
            initialWindowSize++;
        }
        
        // Ensure window size doesn't exceed MAX_SIZE
        windowSize = initialWindowSize < MAX_SIZE ? initialWindowSize : MAX_SIZE;
        currentSize = 0;
        
        // Initialize buffer
        for (int i = 0; i < MAX_SIZE; i++) {
            buffer[i] = T();
        }
    }

    // Add new value to the moving median calculation
    void add(T newValue) {
        // If we haven't reached the initial window size yet
        if (currentSize < windowSize) {
            insertSorted(newValue);
            currentSize++;
            return;
        }
        
        // When we're at full window size
        insertSorted(newValue);
        currentSize++;
        
        // We should now have windowSize + 1 elements
        // Remove elements to maintain the pattern of add 1, remove extreme
        if (currentSize > windowSize) {
            // Strategy: Remove smallest and largest until we're back to windowSize
            while (currentSize > windowSize) {
                // First remove the smallest element (at index 0) using memmove for better performance
                memmove(&buffer[0], &buffer[1], (currentSize - 1) * sizeof(T));
                currentSize--;
                
                // If we still need to remove more, remove the largest
                if (currentSize > windowSize) {
                    currentSize--; // Just decrease size to drop largest element
                }
            }
        }
    }

    // Get the current median value
    T get_median() {
        if (currentSize == 0) {
            return T(); // Return default value if empty
        }
        
        return buffer[currentSize / 2]; // Middle element is the median
    }

    // Clear the buffer
    void reset() {
        currentSize = 0;
    }
    
    // Returns the current number of elements in the buffer
    int size() {
        return currentSize;
    }
    
    bool valid() const {
		return currentSize > 0;
	}

    // Debug getters
    
    // Get a copy of value at specific index in the buffer
    T getValueAt(int index) {
        if (index >= 0 && index < currentSize) {
            return buffer[index];
        }
        return T(); // Return default value for out of bounds
    }
    
    // Get the window size
    int getWindowSize() {
        return windowSize;
    }
};

extern unsigned long time_diff(long unsigned time1, long unsigned time2);

#endif /* UTILS_HPP_ */
