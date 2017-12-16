/*
 * static_ring.hpp
 *
 *  Created on: 16 ���. 2017 �.
 *      Author: ldir
 */

#ifndef STATIC_RING_HPP_
#define STATIC_RING_HPP_

template<typename T, typename ndx_t, int SIZE>
class StaticRing {
	T data[SIZE];
	ndx_t used;
	ndx_t tail;
public:
	StaticRing():used(0),tail(0){};

	ndx_t get_used() const {return used;}
	ndx_t get_size() const {return SIZE;}
	ndx_t get_free() const {return SIZE-used;}

	const T* get(ndx_t i) const {
		if (i < used) {
			return &data[(tail + i) % SIZE];
		} else
			return 0;
	}

	bool drop(ndx_t i) {
		if (i <= used) {
			tail = (tail + i) % SIZE;
			used -= i;
			return true;
		} else
			return false;
	}

	bool put(const T& c) {
		if (SIZE > used) {
			data[(tail + used) % SIZE] = c;
			used++;
			return true;
		} else
			return false;
	}

	void push(const T& c) {
		if (used >= SIZE) {
			tail = (tail + 1) % SIZE;
			used --;
		}
		data[(tail + used) % SIZE] = c;
		used++;
	}

};


#endif /* STATIC_RING_HPP_ */
