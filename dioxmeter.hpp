/*
 * dioxmeter.hpp
 *
 *  Created on: Sep 19, 2020
 *      Author: ldir
 */

#ifndef DIOXMETER_HPP_
#define DIOXMETER_HPP_

struct Measurment {
	int value;
	bool valid;
};


class Dashboard {
public:
	virtual void setup(void){
	}

	virtual void update(const Measurment&)=0;

	virtual ~Dashboard(){};
};

Dashboard* dashboard_impl();

void setup_co2();
bool measue_co2(Measurment&);

#endif /* DIOXMETER_HPP_ */
