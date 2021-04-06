/*
 * eBot_Sandbox.cpp
 *
 * Created on: 21-Mar-2021
 * Author: TAs of CS 684 Spring 2020
 */

//---------------------------------- INCLUDES -----------------------------------

#include "eBot_Sandbox.h"
#include <bits/stdc++.h>

#define SIZE 9
#define NO_OF_PLOTS 16
#define NO_OF_MID_NODE_PER_PLOT 4

#define WEST 'w'
#define EAST 'e'
#define NORTH 'n'
#define SOUTH 's'

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
char dir_flag = NORTH;

char medical_camp_map[SIZE][SIZE];

bool turned[SIZE][SIZE];

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
				_delay_ms(130);
				stop();
				if (dir_flag == NORTH)
					curr_loc.x--;
				else if (dir_flag == SOUTH)
					curr_loc.x++;
				else if (dir_flag == WEST)
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
		if (dir_flag == NORTH)
			dir_flag = WEST;
		else if (dir_flag == WEST)
			dir_flag = SOUTH;
		else if (dir_flag == SOUTH)
			dir_flag = EAST;
		else
			dir_flag = NORTH;
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
		if (dir_flag == NORTH)
			dir_flag = EAST;
		else if (dir_flag == WEST)
			dir_flag = NORTH;
		else if (dir_flag == SOUTH)
			dir_flag = WEST;
		else
			dir_flag = SOUTH;
		printf("Direction = %c\n", dir_flag);
	}
}


void turn_west(void) {           // turn west if possible and return true, otherwise return false
	if(dir_flag == NORTH) {
		left_turn_wls();
	}
	else if(dir_flag == SOUTH) {
		right_turn_wls();
	}
	else if(dir_flag == EAST) {
		left_turn_wls();
		while(dir_flag != EAST) {
			if(dir_flag == WEST)
				break;
			left_turn_wls();
		}
	}
	if(dir_flag != WEST && curr_loc.y - 1 >= 0)
		medical_camp_map[curr_loc.x][curr_loc.y - 1] = 0;
}

void turn_east(void) {		// turn east if possible and return true, otherwise return false
	if(dir_flag == NORTH) {
		right_turn_wls();

	}
	else if(dir_flag == SOUTH) {
		left_turn_wls();

	}
	else if(dir_flag == WEST) {
		left_turn_wls();
		while(dir_flag != WEST) {
			if(dir_flag == EAST)
				break;
			left_turn_wls();
		}
	}
	if(dir_flag != EAST && curr_loc.y + 1 < SIZE)
		medical_camp_map[curr_loc.x][curr_loc.y + 1] = 0;
}

void turn_north(void) {		// turn north if possible and return true, otherwise return false
	if(dir_flag == EAST) {
		left_turn_wls();
	}
	else if(dir_flag == WEST) {
		right_turn_wls();
	}
	else if(dir_flag == SOUTH) {
		left_turn_wls();
		while(dir_flag != SOUTH) {
			if(dir_flag == NORTH)
				break;
			left_turn_wls();
		}
	}
	if(dir_flag != NORTH && curr_loc.x - 1 >= 0)
		medical_camp_map[curr_loc.x - 1][curr_loc.y] = 0;
}

void turn_south(void) {		 // turn south if possible and return true, otherwise return false
	if(dir_flag == EAST) {
		right_turn_wls();
	}
	else if(dir_flag == WEST) {
		left_turn_wls();
	}
	else if(dir_flag == NORTH) {
		left_turn_wls();
		while(dir_flag != NORTH) {
			if(dir_flag == SOUTH)
				break;
			left_turn_wls();
		}
	}
	if(dir_flag != SOUTH && curr_loc.x + 1 < SIZE)
		medical_camp_map[curr_loc.x + 1][curr_loc.y] = 0;
}

Point get_cords(unsigned char plot_no)
{
	unsigned char plots_x[] = {8, 6, 4, 2}, plots_y[] = {7, 1, 3, 5};
	Point cords;
	int x = (plot_no - 1) / 4;
	cords.x = SIZE - plots_x[x];
	cords.y = plots_y[(plot_no % 4)];
	return cords;
}

void initialize_medical_camp_map(void) {

	for(int i = 0; i < SIZE; i++) {
		for(int j = 0; j < SIZE; j++) {
			medical_camp_map[i][j] = 1;
			turned[i][j] = 0;
		}
	}

	Point cords;
	for (int i = 1; i <= NO_OF_PLOTS; i++)
	{
		cords = get_cords(i);
		medical_camp_map[cords.x][cords.y] = 0;
	}

}

void check_plot(void) {
	if(dir_flag == 'n') {
		if(curr_loc.y - 1 >= 0 && medical_camp_map[curr_loc.x][curr_loc.y - 1] == 0) {
			medical_camp_map[curr_loc.x][curr_loc.y - 1] = read_color_sensor_data();
		}
		if(curr_loc.y + 1 < SIZE && medical_camp_map[curr_loc.x][curr_loc.y + 1] == 0) {
			turn_south();
			medical_camp_map[curr_loc.x][curr_loc.y + 1] = read_color_sensor_data();
			turn_north();
		}
	}
	if(dir_flag == 's') {
		if(curr_loc.y + 1 < SIZE && medical_camp_map[curr_loc.x][curr_loc.y + 1] == 0) {
			medical_camp_map[curr_loc.x][curr_loc.y + 1] = read_color_sensor_data();
		}
		if(curr_loc.y - 1 >= 0 && medical_camp_map[curr_loc.x][curr_loc.y - 1] == 0) {
			turn_north();
			medical_camp_map[curr_loc.x][curr_loc.y - 1] = read_color_sensor_data();
			turn_south();
		}
	}
	if(dir_flag == 'w') {
		if(curr_loc.x + 1 < SIZE && medical_camp_map[curr_loc.x + 1][curr_loc.y] == 0) {
			medical_camp_map[curr_loc.x + 1][curr_loc.y] = read_color_sensor_data();
		}
		if(curr_loc.x - 1 >= 0 && medical_camp_map[curr_loc.x - 1][curr_loc.y] == 0) {
			turn_east();
			medical_camp_map[curr_loc.x - 1][curr_loc.y] = read_color_sensor_data();
			turn_west();
		}
	}
	if(dir_flag == 'e') {
		if(curr_loc.x - 1 >= 0 && medical_camp_map[curr_loc.x - 1][curr_loc.y] == 0) {
			medical_camp_map[curr_loc.x - 1][curr_loc.y] = read_color_sensor_data();
		}
		if(curr_loc.x + 1 < SIZE && medical_camp_map[curr_loc.x + 1][curr_loc.y] == 0) {
			turn_west();
			medical_camp_map[curr_loc.x + 1][curr_loc.y] = read_color_sensor_data();
			turn_east();
		}
	}
}

void check_path_possible(void) {
	if(turned[curr_loc.x][curr_loc.y])
		return;
	turned[curr_loc.x][curr_loc.y] = 1;
	if(dir_flag == WEST) {
		turn_north();
		turn_east();
		turn_south();
		turn_west();
	}
	else if(dir_flag == EAST) {
		turn_north();
		turn_west();
		turn_south();
		turn_east();
	}
	else if(dir_flag == NORTH) {
		turn_east();
		turn_south();
		turn_west();
		turn_north();
	}
	else {
		turn_east();
		turn_north();
		turn_west();
		turn_south();
	}
}

bool is_mid_node(Point &node) {
	return ((node.x & 1) == 0 && (node.y & 1) == 1) || ((node.x & 1) == 1 && (node.y & 1) == 0);
}

bool move(Point &source, Point &destination) {

	if(!is_mid_node(source)) {
		check_path_possible();
	}

	if (medical_camp_map[destination.x][destination.y] == 0)
		return false;

	if(destination.y < source.y) {
		turn_west();
	}
	else if(destination.y > source.y) {
		turn_east();
	}
	else if(destination.x < source.x) {
		turn_north();
	}
	else {
		turn_south();
	}
	forward_wls(1);

	if(is_mid_node(destination))
		check_plot();

	return true;
}


/**
 * @brief      Executes the logic to achieve the aim of Lab 4
 */

char traverse_line_to_goal(Point &source, Point &destination) {

	bool visited[SIZE][SIZE];
	Point parent[SIZE][SIZE];
	unsigned char found = 0;

	for(int i = 0; i < SIZE; i++) {
		for(int j = 0; j < SIZE; j++) {
			visited[i][j] = 0;
			parent[i][j] = {0, 0};
		}
	}

	printf("Source = {%d, %d}, Destination = {%d, %d}, Direction = %c\n", source.x, source.y, destination.x, destination.y, dir_flag);
	_delay_ms(5000);

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
		if(p.y - 1 >= 0 && medical_camp_map[p.x][p.y - 1] == 1 && visited[p.x][p.y - 1] == 0) {
			Queue.push({p.x, p.y - 1});
			visited[p.x][p.y - 1] = 1;
			parent[p.x][p.y - 1] = {p.x, p.y};
		}

		// Moving North
		if(p.x - 1 >= 0 && medical_camp_map[p.x - 1][p.y] == 1 && visited[p.x - 1][p.y] == 0) {
			Queue.push({p.x - 1, p.y});
			visited[p.x - 1][p.y] = 1;
			parent[p.x - 1][p.y] = {p.x, p.y};
		}

		// Moving East
		if(p.y + 1 < SIZE && medical_camp_map[p.x][p.y + 1] == 1 && visited[p.x][p.y + 1] == 0) {
			Queue.push({p.x, p.y + 1});
			visited[p.x][p.y + 1] = 1;
			parent[p.x][p.y + 1] = {p.x, p.y};
		}

		// Moving South
		if(p.x + 1 < SIZE && medical_camp_map[p.x + 1][p.y] == 1 && visited[p.x + 1][p.y] == 0) {
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

bool traverse_to_node(Point &destination) {
	while(traverse_line_to_goal(curr_loc, destination) == 2);
	_delay_ms(5000);
	if(curr_loc.x == destination.x && curr_loc.y == destination.y)
		return true;
	return false;
}

void path_planning(void)
{
	initialize_medical_camp_map();

	forward_wls(1);

	Point cords;
	for (int i = 1; i <= NO_OF_PLOTS; i++)
	{
		cords = get_cords(i);
		if(medical_camp_map[cords.x][cords.y] == 0) {

			Point mid_node_cords[NO_OF_MID_NODE_PER_PLOT];
			mid_node_cords[0] = {cords.x - 1, cords.y};             // Top mid node
			mid_node_cords[1] = {cords.x, cords.y + 1};				// Right mid node
			mid_node_cords[2] = {cords.x + 1, cords.y}; 			// Bottom mid node
			mid_node_cords[3] = {cords.x, cords.y - 1};			 	// Left mid node

			for(int i = 0; i < NO_OF_MID_NODE_PER_PLOT; i++) {
				if(traverse_to_node(mid_node_cords[i]))
					break;
			}
		}
	}
	traverse_to_node(med_loc);
	turn_east();
	forward_wls(1);

	for(int i = 0; i < SIZE; i++) {
		for(int j = 0; j < SIZE; j++) {
			printf("%d %d %c\n", i, j, medical_camp_map[i][j]);
		}
	}
}
