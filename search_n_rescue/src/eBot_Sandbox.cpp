/*
 * eBot_Sandbox.cpp
 *
 * Created on: 21-Mar-2021
 * Author: TAs of CS 684 Spring 2020
 */

//---------------------------------- INCLUDES -----------------------------------

#include "eBot_Sandbox.h"
#include <stdlib.h>
#include <bits/stdc++.h>

#define N 9

using namespace std;

//------------------------------ GLOBAL VARIABLES -------------------------------

// To store 8-bit data of left, center and right white line sensors
unsigned char left_wl_sensor_data, center_wl_sensor_data, right_wl_sensor_data;


// To store the eBot's Current and Goal location
typedef struct
{
	int x, y;
} Point;



// To store the direction in which eBot is currently facing
char dir_flag = 'n';

bool medical_camp[N][N];

Point curr_loc = {9, 4}, med_loc = {4, 8};


//---------------------------------- FUNCTIONS ----------------------------------

/**
 * @brief      Executes the logic to achieve the aim of Lab 3
 */


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
	//printf("L = %d C = %d R = %d\n", left_wl_sensor_data, center_wl_sensor_data, right_wl_sensor_data);
}

bool forward_wls(unsigned char node)
{

	bool flag = true;
	for (int i = 0; i < node; i++)
	{
		unsigned char t = 0;
		unsigned char s = 1;
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
					curr_loc.x--;
				else if (dir_flag == 's')
					curr_loc.x++;
				else if (dir_flag == 'w')
					curr_loc.y--;
				else
					curr_loc.y++;
				printf("Current: (%d, %d)\n", curr_loc.x, curr_loc.y);
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
				if (s > 63) {
					flag = false;
					break;
				}
				continue;
			}
			t = 0;
			s = 1;
			_delay_ms(10);
		}
	}
	return flag;
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


bool turn_west(void) {           // turn west if possible and return true, otherwise return false
	if(dir_flag == 'n') {
		left_turn_wls();
		if(dir_flag != 'w') {
			right_turn_wls();
		}
	}
	else if(dir_flag == 's') {
		right_turn_wls();
		if(dir_flag != 'w') {
			left_turn_wls();
		}
	}
	else if(dir_flag == 'e') {
		left_turn_wls();
		while(dir_flag != 'e') {
			if(dir_flag == 'w')
				return true;
			left_turn_wls();
		}
	}
	if(dir_flag == 'w') {
		return true;
	}
	return false;
}

bool turn_east(void) {		// turn east if possible and return true, otherwise return false
	if(dir_flag == 'n') {
		right_turn_wls();
		if(dir_flag != 'e') {
			left_turn_wls();
		}
	}
	else if(dir_flag == 's') {
		left_turn_wls();
		if(dir_flag != 'e') {
			right_turn_wls();
		}
	}
	else if(dir_flag == 'w') {
		left_turn_wls();
		while(dir_flag != 'w') {
			if(dir_flag == 'e')
				return true;
			left_turn_wls();
		}
	}
	if(dir_flag == 'e') {
		return true;
	}
	return false;
}

bool turn_north(void) {		// turn north if possible and return true, otherwise return false
	if(dir_flag == 'e') {
		left_turn_wls();
		if(dir_flag != 'n') {
			right_turn_wls();
		}
	}
	else if(dir_flag == 'w') {
		right_turn_wls();
		if(dir_flag != 'n') {
			left_turn_wls();
		}
	}
	else if(dir_flag == 's') {
		left_turn_wls();
		while(dir_flag != 's') {
			if(dir_flag == 'n')
				return true;
			left_turn_wls();
		}
	}
	if(dir_flag == 'n') {
		return true;
	}
	return false;
}

bool turn_south(void) {		 // turn south if possible and return true, otherwise return false
	if(dir_flag == 'e') {
		right_turn_wls();
		if(dir_flag != 's') {
			left_turn_wls();
		}
	}
	else if(dir_flag == 'w') {
		left_turn_wls();
		if(dir_flag != 's') {
			right_turn_wls();
		}
	}
	else if(dir_flag == 'n') {
		left_turn_wls();
		while(dir_flag != 'n') {
			if(dir_flag == 's')
				return true;
			left_turn_wls();
		}
	}
	if(dir_flag == 's') {
		return true;
	}
	return false;
}

Point get_cords(unsigned char plot_no)
{
	unsigned char plots_x[] = {8, 6, 4, 2}, plots_y[] = {7, 1, 3, 5};
	Point cords;
	int x;
	x = (plot_no - 1) / 4;
	cords.x = N - plots_x[x];
	cords.y = plots_y[(plot_no % 4)];
	return cords;
}

void initialize_medical_camp(void) {

	for(int i = 0; i < N; i++) {
		for(int j = 0; j < N; j++) {
			medical_camp[i][j] = 1;
		}
	}

	Point cords;
	for (int i = 1; i <= 16; i++)
	{
		cords = get_cords(i);
		medical_camp[cords.x][cords.y] = 0;
	}
}


bool move(Point source, Point destination) {

	bool is_possible;

	if(destination.y < source.y) {
		is_possible = turn_west() && forward_wls(1);;
	}
	else if(destination.y > source.y) {
		is_possible = turn_east() && forward_wls(1);;
	}
	else if(destination.x < source.x) {
		is_possible = turn_north() && forward_wls(1);;
	}
	else {
		is_possible = turn_south() && forward_wls(1);;
	}

	if(!is_possible) {
		medical_camp[destination.x][destination.y] = 0;
		return false;
	}
	return true;
}


/**
 * @brief      Executes the logic to achieve the aim of Lab 4
 */

char traverse_line_to_goal(Point source, Point destination) {

	bool visited[N][N];
	Point parent[N][N];
	char found = 0;

	for(int i = 0; i < N; i++) {
		for(int j = 0; j < N; j++) {
			visited[i][j] = 0;
			parent[i][j] = {0, 0};
		}
	}

	printf("Source = {%d, %d}, Destination = {%d, %d}\n", source.x, source.y, destination.x, destination.y);

	queue<Point> Queue;
	Queue.push(source);
	visited[source.x][source.y] = 1;
	parent[source.x][source.y] = {-1, -1};
	while(!Queue.empty()) {
		Point p = Queue.front();
		Queue.pop();

		if(p.x == destination.x && p.y == destination.y) {
			found = 1;
			break;
		}

		// Moving West
		if(p.y - 1 >= 0 && medical_camp[p.x][p.y - 1] == 1 && visited[p.x][p.y - 1] == 0) {
			Queue.push({p.x, p.y - 1});
			visited[p.x][p.y - 1] = 1;
			parent[p.x][p.y - 1] = {p.x, p.y};
		}

		// Moving North
		if(p.x - 1 >= 0 && medical_camp[p.x - 1][p.y] == 1 && visited[p.x - 1][p.y] == 0) {
			Queue.push({p.x - 1, p.y});
			visited[p.x - 1][p.y] = 1;
			parent[p.x - 1][p.y] = {p.x, p.y};
		}

		// Moving East
		if(p.y + 1 < N && medical_camp[p.x][p.y + 1] == 1 && visited[p.x][p.y + 1] == 0) {
			Queue.push({p.x, p.y + 1});
			visited[p.x][p.y + 1] = 1;
			parent[p.x][p.y + 1] = {p.x, p.y};
		}

		// Moving South
		if(p.x + 1 < N && medical_camp[p.x + 1][p.y] == 1 && visited[p.x + 1][p.y] == 0) {
			Queue.push({p.x + 1, p.y});
			visited[p.x + 1][p.y] = 1;
			parent[p.x + 1][p.y] = {p.x, p.y};
		}
	}

	if(found) {

		stack<Point> Stack;
		Point temp = destination;
		while(temp.x != -1 && temp.y != -1) {
			Stack.push(temp);
			temp = parent[temp.x][temp.y];
		}

		temp = Stack.top();
		Stack.pop();
		while(!Stack.empty()) {
			Point temp1 = Stack.top();
			Stack.pop();
			if(!move(temp, temp1))
				return 2;
			temp = temp1;
		}
	}

	return found;
}

void path_planning(void)
{
	initialize_medical_camp();

	forward_wls(1);

	Point cords;
	for (int i = 1; i <= 16; i++)
	{
		cords = get_cords(i);
//		printf("Plot %d : (%d, %d)\n", i, cords.x, cords.y);
//		printf("Left Cords: (%d, %d)\n", cords.x, cords.y - 1);
//		printf("Right Cords: (%d, %d)\n", cords.x, cords.y + 1);
//		printf("Top Cords: (%d, %d)\n", cords.x - 1, cords.y);
//		printf("Bottom Cords: (%d, %d)\n\n", cords.x + 1, cords.y);
		Point leftCords = {cords.x, cords.y - 1};
		Point rightCords = {cords.x, cords.y + 1};
		Point topCords = {cords.x - 1, cords.y};
		Point bottomCords = {cords.x + 1, cords.y};

		while(traverse_line_to_goal(curr_loc, leftCords) == 2);
		_delay_ms(1000);
		if(curr_loc.x == leftCords.x && curr_loc.y == leftCords.y)
			continue;
		while(traverse_line_to_goal(curr_loc, rightCords) == 2);
		_delay_ms(1000);
		if(curr_loc.x == rightCords.x && curr_loc.y == rightCords.y)
			continue;
		while(traverse_line_to_goal(curr_loc, topCords) == 2);
		_delay_ms(1000);
		if(curr_loc.x == topCords.x && curr_loc.y == topCords.y)
			continue;
		while(traverse_line_to_goal(curr_loc, bottomCords) == 2);
		_delay_ms(1000);


//		int distToLeftCord = abs(curr_loc.x - leftCords.x) + abs(curr_loc.y - leftCords.y);
//		int distToRightCord = abs(curr_loc.x - rightCords.x) + abs(curr_loc.y - rightCords.y);
//		int distToTopCord = abs(curr_loc.x - topCords.x) + abs(curr_loc.y - topCords.y);
//		int distToBottomCord = abs(curr_loc.x - bottomCords.x) + abs(curr_loc.y - bottomCords.y);
//
//		Point minCord1, minCord2, maxCord1, maxCord2;
//
//		if (distToTopCord <= distToBottomCord)
//		{
//			minCord1 = topCords;
//			maxCord1 = bottomCords;
//		}
//		else
//		{
//			minCord1 = bottomCords;
//			maxCord1 = topCords;
//		}
//
//		if (distToLeftCord <= distToRightCord)
//		{
//			minCord2 = leftCords;
//			maxCord2 = rightCords;
//		}
//		else
//		{
//			minCord2 = rightCords;
//			maxCord2 = leftCords;
//		}
//
//		while(traverse_line_to_goal(curr_loc, minCord1) == 2);
//		_delay_ms(1000);
//		if(curr_loc.x == minCord1.x && curr_loc.y == minCord1.y)
//			continue;
//		while(traverse_line_to_goal(curr_loc, minCord2) == 2);
//		_delay_ms(1000);
//		if(curr_loc.x == minCord2.x && curr_loc.y == minCord2.y)
//			continue;
//		while(traverse_line_to_goal(curr_loc, maxCord1) == 2);
//		_delay_ms(1000);
//		if(curr_loc.x == maxCord1.x && curr_loc.y == maxCord1.y)
//			continue;
//		while(traverse_line_to_goal(curr_loc, maxCord2) == 2);
//		_delay_ms(1000);

	}
	while(traverse_line_to_goal(curr_loc, med_loc) == 2);
	turn_east();
	forward_wls(1);
}
