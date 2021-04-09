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

#define WEST 'w'
#define EAST 'e'
#define NORTH 'n'
#define SOUTH 's'

#define GREEN 'G'
#define RED 'R'
#define WHITE 'W'

using namespace std;

//------------------------------ GLOBAL VARIABLES -------------------------------

// To store 8-bit data of left, center and right white line sensors
unsigned char left_wl_sensor_data, center_wl_sensor_data, right_wl_sensor_data;


// To store the eBot's Current and Goal location
typedef struct
{
	int x, y;
} Point;

Point curr_loc = {9, 4}, med_loc = {4, 8};

// To store the direction in which eBot is currently facing
unsigned char dir_flag = NORTH;

unsigned char medical_camp_map[SIZE][SIZE];

unsigned char turned[SIZE][SIZE];

long long int TimeCounter = 0L;


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
	TimeCounter++;
	//printf("L = %d C = %d R = %d\n", left_wl_sensor_data, center_wl_sensor_data, right_wl_sensor_data);
}

void forward_wls(unsigned char node)
{
	for (unsigned char i = 0; i < node; i++)
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
				printf("Intersection Reached\n");
				forward();
				velocity(200, 200);
				_delay_ms(125);
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
			else
			{	 // WWW or any other reading
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
	printf("Initial Direction = %c\n", dir_flag);
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
	for (unsigned char i = 0; i <= timer / 180; i++)
	{
		if (dir_flag == NORTH)
			dir_flag = WEST;
		else if (dir_flag == WEST)
			dir_flag = SOUTH;
		else if (dir_flag == SOUTH)
			dir_flag = EAST;
		else
			dir_flag = NORTH;
	}
	printf("Final Direction = %c\n\n", dir_flag);
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
	printf("Initial Direction = %c\n", dir_flag);
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
	for (unsigned char i = 0; i <= timer / 180; i++)
	{
		if (dir_flag == NORTH)
			dir_flag = EAST;
		else if (dir_flag == WEST)
			dir_flag = NORTH;
		else if (dir_flag == SOUTH)
			dir_flag = WEST;
		else
			dir_flag = SOUTH;
	}
	printf("Final Direction = %c\n\n", dir_flag);
}

unsigned char turn_west(void) {           // turn west if possible and return 1, otherwise return 0
	if(dir_flag == NORTH) {
		left_turn_wls();
	}
	else if(dir_flag == SOUTH) {
		right_turn_wls();
	}
	else if(dir_flag == EAST) {
		do {
			left_turn_wls();
			if(dir_flag == WEST)
				break;
		} while(dir_flag != EAST);
	}
	if(dir_flag == WEST)
		return 1;
	return 0;
}

unsigned char turn_east(void) {		// turn east if possible and return 1, otherwise return 0
	if(dir_flag == NORTH) {
		right_turn_wls();
	}
	else if(dir_flag == SOUTH) {
		left_turn_wls();
	}
	else if(dir_flag == WEST) {
		do {
			left_turn_wls();
			if(dir_flag == EAST)
				break;
		} while(dir_flag != WEST);
	}
	if(dir_flag == EAST)
		return 1;
	return 0;
}

unsigned char turn_north(void) {		// turn north if possible and return 1, otherwise return 0
	if(dir_flag == EAST) {
		left_turn_wls();
	}
	else if(dir_flag == WEST) {
		right_turn_wls();
	}
	else if(dir_flag == SOUTH) {
		do {
			left_turn_wls();
			if(dir_flag == NORTH)
				break;
		} while(dir_flag != SOUTH);
	}
	if(dir_flag == NORTH)
		return 1;
	return 0;
}

unsigned char turn_south(void) {		 // turn south if possible and return 1, otherwise return 0
	if(dir_flag == EAST) {
		right_turn_wls();
	}
	else if(dir_flag == WEST) {
		left_turn_wls();
	}
	else if(dir_flag == NORTH) {
		do {
			left_turn_wls();
			if(dir_flag == SOUTH)
				break;
		} while(dir_flag != NORTH);
	}
	if(dir_flag == SOUTH)
		return 1;
	return 0;
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
	for (unsigned char i = 1; i <= NO_OF_PLOTS; i++)
	{
		cords = get_cords(i);
		medical_camp_map[cords.x][cords.y] = i + 1;
	}

}

unsigned char plot_not_checked(int x, int y) {

	if(x < 0 && x >= SIZE && y < 0 && y >= SIZE)
		return 0;

	unsigned char stored_value = medical_camp_map[x][y];

	if(stored_value >= 2 && stored_value <= NO_OF_PLOTS + 1)
		return 1;
	return 0;
}

void store_and_print_injury_type(int x, int y) {

	printf("\n\nPlot No. = %d - ", medical_camp_map[x][y] - 1);

	medical_camp_map[x][y] = read_color_sensor_data();

	if(medical_camp_map[x][y] == GREEN)
		printf("MinorInjury\n\n\n");
	else if(medical_camp_map[x][y] == RED)
		printf("MajorInjury\n\n\n");
	else
		printf("NoInjury\n\n\n");
}

void check_plot(void) {
	if(dir_flag == NORTH) {
		if(plot_not_checked(curr_loc.x, curr_loc.y - 1)) {
			store_and_print_injury_type(curr_loc.x, curr_loc.y - 1);
		}
		if(plot_not_checked(curr_loc.x, curr_loc.y + 1)) {
			turn_south();
			store_and_print_injury_type(curr_loc.x, curr_loc.y + 1);
			turn_north();
		}
	}
	if(dir_flag == SOUTH) {
		if(plot_not_checked(curr_loc.x, curr_loc.y + 1)) {
			store_and_print_injury_type(curr_loc.x, curr_loc.y + 1);
		}
		if(plot_not_checked(curr_loc.x, curr_loc.y - 1)) {
			turn_north();
			store_and_print_injury_type(curr_loc.x, curr_loc.y - 1);
			turn_south();
		}
	}
	if(dir_flag == WEST) {
		if(plot_not_checked(curr_loc.x + 1, curr_loc.y)) {
			store_and_print_injury_type(curr_loc.x + 1, curr_loc.y);
		}
		if(plot_not_checked(curr_loc.x - 1, curr_loc.y)) {
			turn_east();
			store_and_print_injury_type(curr_loc.x - 1, curr_loc.y);
			turn_west();
		}
	}
	if(dir_flag == EAST) {
		if(plot_not_checked(curr_loc.x - 1, curr_loc.y)) {
			store_and_print_injury_type(curr_loc.x - 1, curr_loc.y);
		}
		if(plot_not_checked(curr_loc.x + 1, curr_loc.y)) {
			turn_west();
			store_and_print_injury_type(curr_loc.x + 1, curr_loc.y);
			turn_east();
		}
	}
}

/*
void check_path_possible(int x, int y) {
	if(turned[x][y])
		return;
	turned[x][y] = 1;

	bool is_north_possible = false;
	bool is_south_possible = false;
	bool is_east_possible = false;
	bool is_west_possible = false;

	if(dir_flag == WEST) {
		is_east_possible = true;
		while(dir_flag != EAST) {
			right_turn_wls();
			if(dir_flag == NORTH) {
				is_north_possible = true;
				break;
			}
		}
		left_turn_wls();
		while(dir_flag != EAST) {
			if(dir_flag == WEST) {
				is_west_possible = true;
			}
			if(dir_flag == SOUTH) {
				is_south_possible = true;
				break;
			}
			left_turn_wls();
		}
	}

	else if(dir_flag == EAST) {
		is_west_possible = true;
		while(dir_flag != WEST) {
			right_turn_wls();
			if(dir_flag == SOUTH) {
				is_south_possible = true;
				break;
			}
		}
		left_turn_wls();
		while(dir_flag != WEST) {
			if(dir_flag == EAST) {
				is_east_possible = true;
			}
			if(dir_flag == NORTH) {
				is_north_possible = true;
				break;
			}
			left_turn_wls();
		}
	}

	else if(dir_flag == NORTH) {
		is_south_possible = true;
		while(dir_flag != SOUTH) {
			right_turn_wls();
			if(dir_flag == EAST) {
				is_east_possible = true;
				break;
			}
		}
		left_turn_wls();
		while(dir_flag != SOUTH) {
			if(dir_flag == NORTH) {
				is_north_possible = true;
			}
			if(dir_flag == WEST) {
				is_west_possible = true;
				break;
			}
			left_turn_wls();
		}
	}

	else {
		is_north_possible = true;
		while(dir_flag != NORTH) {
			right_turn_wls();
			if(dir_flag == WEST) {
				is_west_possible = true;
				break;
			}
		}
		left_turn_wls();
		while(dir_flag != NORTH) {
			if(dir_flag == SOUTH) {
				is_south_possible = true;
			}
			if(dir_flag == EAST) {
				is_east_possible = true;
				break;
			}
			left_turn_wls();
		}
	}

	if(!is_north_possible && x - 1 >= 0)
		medical_camp_map[x - 1][y] = 0;
	if(!is_south_possible && x + 1 < SIZE)
		medical_camp_map[x + 1][y] = 0;
	if(!is_east_possible && y + 1 < SIZE)
		medical_camp_map[x][y + 1] = 0;
	if(!is_west_possible && y - 1 >= 0)
		medical_camp_map[x][y - 1] = 0;

}
*/

unsigned char check_front() {
	readSensor();
	if(left_wl_sensor_data < 50 && center_wl_sensor_data < 50 && right_wl_sensor_data < 50) {
		return 0;
	}
	return 1;
}

void check_path_possible(int x, int y) {
	if(turned[x][y])
		return;
	turned[x][y] = 1;

	if(dir_flag == NORTH) {
		if(x - 1 >= 0 && medical_camp_map[x - 1][y] == 1)
			medical_camp_map[x - 1][y] = check_front();
		if(y - 1 >= 0 && medical_camp_map[x][y - 1] == 1)
			medical_camp_map[x][y - 1] = turn_west();
		if(y + 1 < SIZE && medical_camp_map[x][y + 1] == 1)
			medical_camp_map[x][y + 1] = turn_east();
	}
	else if(dir_flag == SOUTH) {
		if(x + 1 < SIZE && medical_camp_map[x + 1][y] == 1)
			medical_camp_map[x + 1][y] = check_front();
		if(y - 1 >= 0 && medical_camp_map[x][y - 1] == 1)
			medical_camp_map[x][y - 1] = turn_west();
		if(y + 1 < SIZE && medical_camp_map[x][y + 1] == 1)
			medical_camp_map[x][y + 1] = turn_east();
	}
	else if(dir_flag == WEST) {
		if(y - 1 >= 0 && medical_camp_map[x][y - 1] == 1)
			medical_camp_map[x][y - 1] = check_front();
		if(x - 1 >= 0 && medical_camp_map[x - 1][y] == 1)
			medical_camp_map[x - 1][y] = turn_north();
		if(x + 1 < SIZE && medical_camp_map[x + 1][y] == 1)
			medical_camp_map[x + 1][y] = turn_south();
	}
	else {
		if(y + 1 < SIZE && medical_camp_map[x][y + 1] == 1)
			medical_camp_map[x][y + 1] = check_front();
		if(x - 1 >= 0 && medical_camp_map[x - 1][y] == 1)
			medical_camp_map[x - 1][y] = turn_north();
		if(x + 1 < SIZE && medical_camp_map[x + 1][y] == 1)
			medical_camp_map[x + 1][y] = turn_south();
	}
}

unsigned char is_mid_node(Point &node) {
	if (((node.x & 1) == 0 && (node.y & 1) == 1) || ((node.x & 1) == 1 && (node.y & 1) == 0))
		return 1;
	return 0;
}

unsigned char move(Point &source, Point &destination) {

	if(!is_mid_node(source)) {
		check_path_possible(source.x, source.y);
	}

	if (medical_camp_map[destination.x][destination.y] == 0)
		return 0;

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

	return 1;
}


/**
 * @brief      Executes the logic to achieve the aim of Lab 4
 */

unsigned char traverse_line_to_goal(Point &top_mid_node, Point &bottom_mid_node, Point &left_mid_node, Point &right_mid_node) {

	unsigned char visited[SIZE][SIZE];
	Point parent[SIZE][SIZE];

	for(int i = 0; i < SIZE; i++) {
		for(int j = 0; j < SIZE; j++) {
			visited[i][j] = 0;
			parent[i][j] = {0, 0};
		}
	}

	Point destination;
	queue<Point> Queue;
	Queue.push(curr_loc);
	visited[curr_loc.x][curr_loc.y] = 1;
	parent[curr_loc.x][curr_loc.y] = {-1, -1};
	while(!Queue.empty()) {
		Point p = Queue.front();
		Queue.pop();

		if(p.x == top_mid_node.x && p.y == top_mid_node.y) {
			destination = top_mid_node;
			break;
		}
		if(p.x == bottom_mid_node.x && p.y == bottom_mid_node.y) {
			destination = bottom_mid_node;
			break;
		}
		if(p.x == left_mid_node.x && p.y == left_mid_node.y) {
			destination = left_mid_node;
			break;
		}
		if(p.x == right_mid_node.x && p.y == right_mid_node.y) {
			destination = right_mid_node;
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

	printf("Source = {%d, %d}, Destination = {%d, %d}, Direction = %c\n", curr_loc.x, curr_loc.y, destination.x, destination.y, dir_flag);

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
			return 0;
		temp = temp1;
	}

	return 1;
}

void traverse_to_nearest_node(Point &top_mid_node, Point &bottom_mid_node, Point &left_mid_node, Point &right_mid_node) {
	while(!traverse_line_to_goal(top_mid_node, bottom_mid_node, left_mid_node, right_mid_node));
}

void serve_request() {
	// Scan_Request
	_delay_ms(2000);
	printf("Identify Survivor at plot 3\n");
	Point cords = get_cords(3);
	Point top_mid_node = {cords.x - 1, cords.y};
	Point bottom_mid_node = {cords.x + 1, cords.y};
	Point left_mid_node = {cords.x, cords.y - 1};
	Point right_mid_node = {cords.x, cords.y + 1};
	traverse_to_nearest_node(top_mid_node, bottom_mid_node, left_mid_node, right_mid_node);

	//
//	_delay_ms(2000);
//	printf("Fetch RED Survivor\n");
//
}

void path_planning(void)
{
	initialize_medical_camp_map();

	forward_wls(1);

	Point cords;
	char not_satisfied = 1;
	for (unsigned char i = NO_OF_PLOTS; i >= 1; i--)
	{
		printf("Plot No. = %d\n", i);
		cords = get_cords(i);
		if(medical_camp_map[cords.x][cords.y] <= NO_OF_PLOTS + 1) {

			Point top_mid_node = {cords.x - 1, cords.y};
			Point bottom_mid_node = {cords.x + 1, cords.y};
			Point left_mid_node = {cords.x, cords.y - 1};
			Point right_mid_node = {cords.x, cords.y + 1};

			traverse_to_nearest_node(top_mid_node, bottom_mid_node, left_mid_node, right_mid_node);
		}

		//printf("TimeCounter = %lld\n", TimeCounter);
		//_delay_ms(100000);
		if(TimeCounter > 3000 && not_satisfied) {
			serve_request();
			not_satisfied = 0;
		}

	}
	traverse_to_nearest_node(med_loc, med_loc, med_loc, med_loc);
	turn_east();
	forward_wls(1);

	printf("\n\n");
	unsigned char plot_no = 0;
	for(int i = 0; i < SIZE; i++) {
		for(int j = 0; j < SIZE; j++) {
			if(i % 2 == 1 && j % 2 == 1) {
				plot_no++;
				if(medical_camp_map[i][j] == GREEN)
					printf("%d - MinorInjury\n", plot_no);
				else if(medical_camp_map[i][j] == RED)
					printf("%d - MajorInjury\n", plot_no);
				else
					printf("%d - \n", plot_no);
			}
		}
	}
}
