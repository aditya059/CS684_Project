/*
 * eBot_Sandbox.cpp
 *
 * Created on: 21-Mar-2021
 * Author: TAs of CS 684 Spring 2020
 */

//---------------------------------- INCLUDES -----------------------------------

#include "eBot_Sandbox.h"
#include <stdlib.h>

//------------------------------ GLOBAL VARIABLES -------------------------------

// To store 8-bit data of left, center and right white line sensors
unsigned char left_wl_sensor_data, center_wl_sensor_data, right_wl_sensor_data;

// To store 8-bit data of 5th IR proximity sensors
unsigned char ir_prox_5_sensor_data;

//---------------------------------- FUNCTIONS ----------------------------------

/**
 * @brief      Executes the logic to achieve the aim of Lab 3
 */
void send_sensor_data(void)
{
	int return_code;

	while (1)
	{
		// get the ADC converted data of three white line sensors and
		// 5th IR Proximity sensor sensor from their appropriate channel numbers
		left_wl_sensor_data = convert_analog_channel_data(left_wl_sensor_channel);
		center_wl_sensor_data = convert_analog_channel_data(center_wl_sensor_channel);
		right_wl_sensor_data = convert_analog_channel_data(right_wl_sensor_channel);

		ir_prox_5_sensor_data = convert_analog_channel_data(ir_prox_5_sensor_channel);

		printf("LCR : %d %d %d", left_wl_sensor_data, center_wl_sensor_data, right_wl_sensor_data);

		// return_code = print_ir_prox_5_data(ir_prox_5_sensor_data);

		return_code = print_color_sensor_data();

		if (return_code == 0)
		{
			break;
		}

		_delay_ms(500);
	}
}

// To store the eBot's current and Goal location
typedef struct
{
	int x, y;
} tuple;

tuple curr_loc = {0, 4}, med_loc = {5, 9}, goal_loc;

// To store the direction in which eBot is currently facing
char dir_flag = 'n';

//---------------------------------- FUNCTIONS ----------------------------------

/*
*
* Function Name: forward_wls
* Input: node
* Output: void
* Logic: Uses white line sensors to go forward by the number of nodes specified
* Example Call: forward_wls(2); //Goes forward by two nodes
*
*/
//

void readSensor()
{
	left_wl_sensor_data = convert_analog_channel_data(left_wl_sensor_channel);
	center_wl_sensor_data = convert_analog_channel_data(center_wl_sensor_channel);
	right_wl_sensor_data = convert_analog_channel_data(right_wl_sensor_channel);
	//print_color_sensor_data();
	printf("L = %d C = %d R = %d\n", left_wl_sensor_data, center_wl_sensor_data, right_wl_sensor_data);
}

void forward_wls(unsigned char node)
{
	unsigned char t = 0;
	unsigned char s = 1;
	for (int i = 0; i < node; i++)
	{
		while (1)
		{

			readSensor();

			if (left_wl_sensor_data < 50 && center_wl_sensor_data > 200 && right_wl_sensor_data < 50)
			{ // WBW
				forward();
				velocity(200, 200);
			}
			else if (left_wl_sensor_data < 50 && right_wl_sensor_data > 200)
			{ // WWB & WBB
				right();
				velocity(20, 20);
			}
			else if (left_wl_sensor_data > 200 && right_wl_sensor_data < 50)
			{ // BWW & BBW
				left();
				velocity(20, 20);
			}
			else if (left_wl_sensor_data > 200 && center_wl_sensor_data > 200 && right_wl_sensor_data > 200)
			{ // BBB
				printf("Intersection Reached %d\n", i + 1);
				forward();
				velocity(200, 200);
				_delay_ms(110);
				stop();
				if (dir_flag == 'n')
					curr_loc.x++;
				else if (dir_flag == 's')
					curr_loc.x--;
				else if (dir_flag == 'w')
					curr_loc.y--;
				else
					curr_loc.y++;
				printf("Current: (%d, %d)\n", curr_loc.x, curr_loc.y);
				printf("Goal: (%d, %d)\n", med_loc.x, med_loc.y);
				break;
			}
			else /*if(left_wl_sensor_data < 50 && center_wl_sensor_data < 50 && right_wl_sensor_data < 50)*/
			{	 // WWW
				if (t % 4 == 0)
				{
					left();
					velocity(20, 20);
				}
				else if (t % 4 == 1)
				{
					right();
					velocity(20, 20);
				}
				else if (t % 4 == 2)
				{
					right();
					velocity(20, 20);
				}
				else
				{
					left();
					velocity(20, 20);
				}
				_delay_ms(s * 5);
				if (t % 4 == 3)
					s *= 2;
				t++;
				continue;
			}
			t = 0;
			s = 1;
			_delay_ms(10);
		}
	}
}

/*
*
* Function Name: left_turn_wls
* Input: void
* Output: void
* Logic: Uses white line sensors to turn left until black line is encountered
* Example Call: left_turn_wls();
*
*/
void left_turn_wls(void)
{
	printf("Left Turn\n");
	int timer = 0;
	left();
	_delay_ms(100);
	velocity(20, 20);
	while (1)
	{
		timer++;
		readSensor();
		if (left_wl_sensor_data < 50 && center_wl_sensor_data > 200 && right_wl_sensor_data < 50)
		{
			velocity(0, 0);
			break;
		}
		_delay_ms(10);
	}
	printf("timer = %d\n", timer);
	for (int i = 0; i <= timer / 180; i++)
	{
		if (dir_flag == 'n')
			dir_flag = 'w';
		else if (dir_flag == 'w')
			dir_flag = 's';
		else if (dir_flag == 's')
			dir_flag = 'e';
		else
			dir_flag = 'n';
		printf("Direction = %c\n", dir_flag);
	}
}

/*
*
* Function Name: right_turn_wls
* Input: void
* Output: void
* Logic: Uses white line sensors to turn right until black line is encountered
* Example Call: right_turn_wls();
*
*/
void right_turn_wls(void)
{
	printf("Right Turn\n");
	int timer = 0;
	right();
	_delay_ms(100);
	velocity(20, 20);
	while (1)
	{
		timer++;
		readSensor();
		if (left_wl_sensor_data < 50 && center_wl_sensor_data > 200 && right_wl_sensor_data < 50)
		{
			velocity(0, 0);
			break;
		}
		_delay_ms(10);
	}
	printf("Timer = %d\n", timer);
	for (int i = 0; i <= timer / 180; i++)
	{
		if (dir_flag == 'n')
			dir_flag = 'e';
		else if (dir_flag == 'w')
			dir_flag = 'n';
		else if (dir_flag == 's')
			dir_flag = 'w';
		else
			dir_flag = 's';
		printf("Direction = %c\n", dir_flag);
	}
}



/**
 * @brief      Executes the logic to achieve the aim of Lab 4
 */
void traverse_line_to_goal(void)
{

	if(dir_flag == 'n') {
		// For trying left
		if (goal_loc.y < curr_loc.y) {
			left_turn_wls();
			if (dir_flag == 'w') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 'e') {
					right_turn_wls();
				}
				forward_wls(2);
				left_turn_wls();
			}
			else {
				right_turn_wls();
			}
		}
		// For trying right
		else if(goal_loc.y > curr_loc.y) {
			right_turn_wls();
			if (dir_flag == 'e') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 'w') {
					left_turn_wls();
				}
				forward_wls(2);
				right_turn_wls();
			}
			else {
				left_turn_wls();
			}
		}

		// For going forward
		forward_wls(1);
		// Add plot checking logic here
		forward_wls(1);
		traverse_line_to_goal();
		while(dir_flag != 's') {
			left_turn_wls();
		}
		forward_wls(2);

		// For trying right side if left was tried earlier
		if (goal_loc.y < curr_loc.y) {
			left_turn_wls();
			if (dir_flag == 'e') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 'w') {
					right_turn_wls();
				}
				forward_wls(2);
				left_turn_wls();
			}
			else {
				right_turn_wls();
			}
		}
		// For trying left side if right was tried earlier
		else if(goal_loc.y > curr_loc.y) {
			right_turn_wls();
			if (dir_flag == 'w') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 'e') {
					left_turn_wls();
				}
				forward_wls(2);
				right_turn_wls();
			}
			else {
				left_turn_wls();
			}
		}
	}


	else if(dir_flag == 'e') {
		if (goal_loc.x > curr_loc.x) {
			left_turn_wls();
			if (dir_flag == 'n') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 's') {
					right_turn_wls();
				}
				forward_wls(2);
				left_turn_wls();
			}
			else {
				right_turn_wls();
			}
		}
		else if(goal_loc.x < curr_loc.x) {
			right_turn_wls();
			if (dir_flag == 's') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 'n') {
					left_turn_wls();
				}
				forward_wls(2);
				right_turn_wls();
			}
			else {
				left_turn_wls();
			}
		}


		forward_wls(1);
		// Add plot checking logic here
		forward_wls(1);
		traverse_line_to_goal();
		while(dir_flag != 'w') {
			left_turn_wls();
		}
		forward_wls(2);


		if (goal_loc.x > curr_loc.x) {
			left_turn_wls();
			if (dir_flag == 's') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 'n') {
					right_turn_wls();
				}
				forward_wls(2);
				left_turn_wls();
			}
			else {
				right_turn_wls();
			}
		}
		else if(goal_loc.x < curr_loc.x) {
			right_turn_wls();
			if (dir_flag == 'n') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 's') {
					left_turn_wls();
				}
				forward_wls(2);
				right_turn_wls();
			}
			else {
				left_turn_wls();
			}
		}
	}


	else if(dir_flag == 'w') {

		if (goal_loc.x < curr_loc.x) {
			left_turn_wls();
			if (dir_flag == 's') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 'n') {
					right_turn_wls();
				}
				forward_wls(2);
				left_turn_wls();
			}
			else {
				right_turn_wls();
			}
		}
		else if(goal_loc.x > curr_loc.x) {
			right_turn_wls();
			if (dir_flag == 'n') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 's') {
					left_turn_wls();
				}
				forward_wls(2);
				right_turn_wls();
			}
			else {
				left_turn_wls();
			}
		}


		forward_wls(1);
		// Add plot checking logic here
		forward_wls(1);
		traverse_line_to_goal();
		while(dir_flag != 'e') {
			left_turn_wls();
		}
		forward_wls(2);


		if (goal_loc.x < curr_loc.x) {
			left_turn_wls();
			if (dir_flag == 'n') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 's') {
					right_turn_wls();
				}
				forward_wls(2);
				left_turn_wls();
			}
			else {
				right_turn_wls();
			}
		}
		else if(goal_loc.x > curr_loc.x) {
			right_turn_wls();
			if (dir_flag == 's') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 'n') {
					left_turn_wls();
				}
				forward_wls(2);
				right_turn_wls();
			}
			else {
				left_turn_wls();
			}
		}


	}
	else {

		// For trying left
		if (goal_loc.y > curr_loc.y) {
			left_turn_wls();
			if (dir_flag == 'e') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 'w') {
					right_turn_wls();
				}
				forward_wls(2);
				left_turn_wls();
			}
			else {
				right_turn_wls();
			}
		}
		// For trying right
		else if(goal_loc.y < curr_loc.y) {
			right_turn_wls();
			if (dir_flag == 'w') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 'e') {
					left_turn_wls();
				}
				forward_wls(2);
				right_turn_wls();
			}
			else {
				left_turn_wls();
			}
		}

		// For going forward
		forward_wls(1);
		// Add plot checking logic here
		forward_wls(1);
		traverse_line_to_goal();
		while(dir_flag != 'n') {
			left_turn_wls();
		}
		forward_wls(2);

		// For trying right side if left was tried earlier
		if (goal_loc.y > curr_loc.y) {
			left_turn_wls();
			if (dir_flag == 'w') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 'e') {
					right_turn_wls();
				}
				forward_wls(2);
				left_turn_wls();
			}
			else {
				right_turn_wls();
			}
		}
		// For trying left side if right was tried earlier
		else if(goal_loc.y < curr_loc.y) {
			right_turn_wls();
			if (dir_flag == 'e') {
				forward_wls(1);
				// Add plot checking logic here
				forward_wls(1);
				traverse_line_to_goal();
				while(dir_flag != 'w') {
					left_turn_wls();
				}
				forward_wls(2);
				right_turn_wls();
			}
			else {
				left_turn_wls();
			}
		}
	}
	/*unsigned char step = 0;
	forward_wls(1);
	//curr_loc.y--;
	curr_loc.x--;
	left_turn_wls();
	ir_prox_5_sensor_data = convert_analog_channel_data(ir_prox_5_sensor_channel);
	if(ir_prox_5_sensor_data == 200){
		forward_wls(1);
	}
	else{
		right_turn_wls();
		dir_flag = 'e';
		forward_wls(1);
	}
	while(curr_loc.y != goal_loc.y){
		if(ir_prox_5_sensor_data == 200){
			if(step == 1){
				forward_wls(1);
				step = 0;
			}
			else{
				if(dir_flag == 'w'){
					right_turn_wls();
				}
				else if(dir_flag == 'e'){
					left_turn_wls();
				}
				else if(dir_flag == 'n'){
					forward_wls(1);
				}
				else {
					right_turn_wls();
				}
			}
		}
		else{
			//printf("Curr.x: %d\nGoal.x: %d\n", curr_loc.x, goal_loc.x);
			if(curr_loc.x < goal_loc.x){
				if(dir_flag == 'w'){
					right_turn_wls();
					step = 1;
				}
				else if(dir_flag == 'e'){
					left_turn_wls();
					step = 1;
				}
				else if(dir_flag == 'n'){
					left_turn_wls();
					step = 1;
				}
				else {
					right_turn_wls();
					step = 1;
				}
			}
			else {
				if(dir_flag == 'w'){
					right_turn_wls();
					step = 1;
				}
				else if(dir_flag == 'e'){
					left_turn_wls();
					step = 1;
				}
				else if(dir_flag == 'n'){
					left_turn_wls();
					step = 1;
				}
				else{
					left_turn_wls();
					step = 1;
				}
			}
		}
	}
	while(curr_loc.x != goal_loc.x){
		if(curr_loc.x < goal_loc.x){
			if(dir_flag == 'w'){
				right_turn_wls();
				dir_flag = 'e';
			}
			else if(dir_flag == 'e'){
				forward_wls(1);
			}
			else if(dir_flag == 'n'){
				right_turn_wls();
			}
			else {
				left_turn_wls();
			}
		}
		else {
			if(dir_flag == 'w'){
				forward_wls(1);
			}
			else if(dir_flag == 'e'){
				left_turn_wls();
				dir_flag = 'w';
			}
			else if(dir_flag == 'n'){
				left_turn_wls();
			}
			else{
				right_turn_wls();
			}
		}
		return true;
	}*/


//	forward_wls(5);
//	left_turn_wls();
//	forward_wls(2);
//	right_turn_wls();
//	forward_wls(4);
//	left_turn_wls();
//	forward_wls(2);
//	left_turn_wls();
//	_delay_ms(1000);
//	//forward_wls(2);
//	right_turn_wls();
//	forward_wls(1);
//	_delay_ms(1000000);

	//forward_wls(4);

	//	forward_wls(1);
	//	right_turn_wls();
	//	forward_wls(1);
	//	left_turn_wls();
	//	forward_wls(1);
}

tuple get_cords(unsigned char plot_no)
{
	unsigned char plots_x[] = {8, 6, 4, 2}, plots_y[] = {7, 1, 3, 5};
	tuple cords;
	int x;
	x = (plot_no - 1) / 4;
	cords.x = plots_x[x];
	cords.y = plots_y[(plot_no % 4)];
	return cords;
}

void pathPlanning(void)
{
	forward_wls(1);
//	tuple cords;
//	for (int i = 1; i <= 16; i++)
//	{
//		cords = get_cords(i);
//		printf("Plot %d : (%d, %d)\n", i, cords.x, cords.y);
//		printf("Left Cords: (%d, %d)\n", cords.x, cords.y - 1);
//		printf("Right Cords: (%d, %d)\n", cords.x, cords.y + 1);
//		printf("Top Cords: (%d, %d)\n", cords.x + 1, cords.y);
//		printf("Bottom Cords: (%d, %d)\n\n", cords.x - 1, cords.y);
//		tuple leftCords = {cords.x, cords.y - 1};
//		tuple rightCords = {cords.x, cords.y + 1};
//		tuple topCords = {cords.x + 1, cords.y};
//		tuple bottomCords = {cords.x - 1, cords.y};
//
//		int distToLeftCord = (curr_loc.x - leftCords.x) + (curr_loc.y - leftCords.y);
//		int distToRightCord = (curr_loc.x - rightCords.x) + (curr_loc.y - rightCords.y);
//		int distToTopCord = (curr_loc.x - topCords.x) + (curr_loc.y - topCords.y);
//		int distToBottomCord = (curr_loc.x - bottomCords.x) + (curr_loc.y - bottomCords.y);
//
//		int minDist1, minDist2, maxDist1, maxDist2;
//		tuple minCord1, minCord2, maxCord1, maxCord2;
//		char minNode1, minNode2, maxNode1, maxNode2;
//
//		if (distToTopCord <= distToBottomCord)
//		{
//			minDist1 = distToTopCord;
//			minCord1 = topCords;
//			minNode1 = 'T';
//			maxDist1 = distToBottomCord;
//			maxCord1 = bottomCords;
//			maxNode1 = 'B';
//		}
//		else
//		{
//			minDist1 = distToBottomCord;
//			minCord1 = bottomCords;
//			minNode1 = 'B';
//			maxDist1 = distToTopCord;
//			maxCord1 = topCords;
//			maxNode1 = 'T';
//		}
//
//		if (distToLeftCord <= distToRightCord)
//		{
//			minDist2 = distToLeftCord;
//			minCord2 = leftCords;
//			minNode2 = 'L';
//			maxDist2 = distToRightCord;
//			maxCord2 = rightCords;
//			maxNode2 = 'R';
//		}
//		else
//		{
//			minDist2 = distToRightCord;
//			minCord2 = rightCords;
//			minNode2 = 'R';
//			maxDist2 = distToLeftCord;
//			maxCord2 = leftCords;
//			maxNode2 = 'L';
//		}
//
//		/*goal_loc = {minCord1.x, minCord1.y};
//		if (traverse_line_to_goal())
//			continue;
//
//		goal_loc = {minCord2.x, minCord2.y};
//		if (traverse_line_to_goal())
//			continue;
//
//		goal_loc = {maxCord1.x, maxCord1.y};
//		if (traverse_line_to_goal())
//			continue;
//
//		goal_loc = {maxCord2.x, maxCord2.y};
//		if (traverse_line_to_goal())
//			continue;
//		*/
//
//		/*goal_loc = {minCord1.x, minCord2.y};
//		if (traverse_line_to_goal())
//			continue;
//
//		goal_loc = {minCord1.x, maxCord2.y};
//		if (traverse_line_to_goal())
//			continue;
//
//		goal_loc = {maxCord1.x, minCord2.y};
//		if (traverse_line_to_goal())
//			continue;
//
//		goal_loc = {maxCord1.x, maxCord2.y};
//		if (traverse_line_to_goal())
//			continue;*/
//	}
	goal_loc = med_loc;
	traverse_line_to_goal();
}
