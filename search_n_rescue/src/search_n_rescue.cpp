//==================================================================================
// Name        : search_n_rescue.cpp
// Author      : TAs of CS 684 Spring 2020
// Description : To implement the Problem Statement of Search and Rescue project
//==================================================================================


#include "eBot_Sandbox.h"


// Main Function
int main(int argc, char* argv[])
{
	int init_setup_success = 0;

	init_setup_success = init_setup();

	if (init_setup_success)
	{
		#ifdef NON_MATLAB_PARSING
			std::thread t_1(thread_calls);
			_delay_ms(4000);
		#endif

		//send_sensor_data();
		//traverse_line_to_goal();
		pathPlanning();

		#ifdef NON_MATLAB_PARSING
			clean_up();
			t_1.detach();
		#endif
	}
	else
	{
		printf("\n\tInitialization did not succeed! Try again.");
	}
}
