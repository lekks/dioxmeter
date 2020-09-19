/*
 * dioxmeter.hpp
 *
 *  Created on: Sep 19, 2020
 *      Author: ldir
 */

#ifndef DIOXMETER_HPP_
#define DIOXMETER_HPP_

class Dashboard {
public:
	virtual void setup(void){
	}

	virtual void update(int ppm, bool valid)=0;

	virtual ~Dashboard(){};
};

Dashboard* dashboard_impl();


#endif /* DIOXMETER_HPP_ */
