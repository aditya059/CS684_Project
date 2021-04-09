/*
 * eBot_Sandbox.h
 *
 *  Created on: 11-Jan-2021
 *      Author: TAs of CS 684 Spring 2020
 */

#ifndef EBOT_SANDBOX_H_
#define EBOT_SANDBOX_H_


//---------------------------------- INCLUDES ----------------------------------

#ifdef NON_MATLAB_PARSING
	#include "eBot_Sim_Predef.h"
#else
	extern "C" {
		#include "eBot_MCU_Predef.h"
	}
#endif


//---------------------------------- FUNCTIONS ----------------------------------

//void send_sensor_data(void);
void path_planning(void);


#endif /* EBOT_SANDBOX_H_ */
