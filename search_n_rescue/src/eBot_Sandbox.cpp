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

#define NORTH 'n'
#define SOUTH 's'
#define WEST 'w'
#define EAST 'e'

#define GREEN 'G'
#define RED 'R'

#define SCAN 'S'

#define DESTINATION 'D'

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

void readSensor(void)
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
			stop();
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
			stop();
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
		medical_camp_map[cords.x][cords.y] = DESTINATION;
	}

}

unsigned char plot_not_checked(int x, int y) {

	if(x < 0 || x >= SIZE || y < 0 || y >= SIZE)          // Invalid coordinate
		return 0;

	if(medical_camp_map[x][y] == DESTINATION || medical_camp_map[x][y] == SCAN)
		return 1;
	return 0;
}

void store_and_print_injury_type(int x, int y) {

	unsigned char plot_no = 4 * (x / 2) + (y / 2) + 1;

	medical_camp_map[x][y] = read_color_sensor_data();

	if(medical_camp_map[x][y] == GREEN)
		printf("\n\nPlot No. = %d - MinorInjury\n\n\n", plot_no);
	else if(medical_camp_map[x][y] == RED)
		printf("\n\nPlot No. = %d - MajorInjury\n\n\n", plot_no);
	else
		printf("\n\nPlot No. = %d - NoInjury\n\n\n", plot_no);
}

void check_plot(void) {
	if(dir_flag == NORTH) {
		if(plot_not_checked(curr_loc.x, curr_loc.y - 1)) {
			store_and_print_injury_type(curr_loc.x, curr_loc.y - 1);
		}
		if(plot_not_checked(curr_loc.x, curr_loc.y + 1)) {
			turn_south();
			store_and_print_injury_type(curr_loc.x, curr_loc.y + 1);
		}
	}
	if(dir_flag == SOUTH) {
		if(plot_not_checked(curr_loc.x, curr_loc.y + 1)) {
			store_and_print_injury_type(curr_loc.x, curr_loc.y + 1);
		}
		if(plot_not_checked(curr_loc.x, curr_loc.y - 1)) {
			turn_north();
			store_and_print_injury_type(curr_loc.x, curr_loc.y - 1);
		}
	}
	if(dir_flag == WEST) {
		if(plot_not_checked(curr_loc.x + 1, curr_loc.y)) {
			store_and_print_injury_type(curr_loc.x + 1, curr_loc.y);
		}
		if(plot_not_checked(curr_loc.x - 1, curr_loc.y)) {
			turn_east();
			store_and_print_injury_type(curr_loc.x - 1, curr_loc.y);
		}
	}
	if(dir_flag == EAST) {
		if(plot_not_checked(curr_loc.x - 1, curr_loc.y)) {
			store_and_print_injury_type(curr_loc.x - 1, curr_loc.y);
		}
		if(plot_not_checked(curr_loc.x + 1, curr_loc.y)) {
			turn_west();
			store_and_print_injury_type(curr_loc.x + 1, curr_loc.y);
		}
	}
}

unsigned char check_front(void) {
	readSensor();
	if(left_wl_sensor_data < 50 && center_wl_sensor_data < 50 && right_wl_sensor_data < 50) {
		return 0;
	}
	return 1;
}

void check_path_possible(int x, int y) {
	if(turned[x][y])										// Has already turned before to check all valid paths
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

char not_satisfied = 1;
void serve_request(void);

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

	if(TimeCounter > 3000 && not_satisfied) {
		not_satisfied = 0;
		serve_request();
	}

	return 1;
}

/*
 * search_char = 'R' for MajorInjury of fetch request, or
 *               'G' for MinorInjury of fetch request, or
 *               'D' for destination
 *               'S' for scan request
*/
// Reference : https://www.geeksforgeeks.org/shortest-distance-two-cells-matrix-grid/
unsigned char traverse_line_to_goal(unsigned char search_char) {

	unsigned char visited[SIZE][SIZE];
	Point parent[SIZE][SIZE];
	unsigned char is_found = 0;

	for(int i = 0; i < SIZE; i++) {
		for(int j = 0; j < SIZE; j++) {
			if(medical_camp_map[i][j] == 1 || medical_camp_map[i][j] == search_char)
				visited[i][j] = 0;
			else
				visited[i][j] = 1;
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

		if(medical_camp_map[p.x][p.y] == search_char) {
			is_found = 1;
			destination = parent[p.x][p.y];
			break;
		}

		// Moving West
		if(p.y - 1 >= 0 && visited[p.x][p.y - 1] == 0) {
			Queue.push({p.x, p.y - 1});
			visited[p.x][p.y - 1] = 1;
			parent[p.x][p.y - 1] = {p.x, p.y};
		}

		// Moving North
		if(p.x - 1 >= 0 && visited[p.x - 1][p.y] == 0) {
			Queue.push({p.x - 1, p.y});
			visited[p.x - 1][p.y] = 1;
			parent[p.x - 1][p.y] = {p.x, p.y};
		}

		// Moving East
		if(p.y + 1 < SIZE && visited[p.x][p.y + 1] == 0) {
			Queue.push({p.x, p.y + 1});
			visited[p.x][p.y + 1] = 1;
			parent[p.x][p.y + 1] = {p.x, p.y};
		}

		// Moving South
		if(p.x + 1 < SIZE && visited[p.x + 1][p.y] == 0) {
			Queue.push({p.x + 1, p.y});
			visited[p.x + 1][p.y] = 1;
			parent[p.x + 1][p.y] = {p.x, p.y};
		}
	}

	printf("Source = {%d, %d}, Destination = {%d, %d}, Direction = %c\n", curr_loc.x, curr_loc.y, destination.x, destination.y, dir_flag);

	if(is_found) {
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

	return is_found;
}

void scan_arena(unsigned char search_char) {
	while(traverse_line_to_goal(search_char));
}

void traverse_to_medical_camp(void) {
	medical_camp_map[med_loc.x][med_loc.y] = DESTINATION;
	while(traverse_line_to_goal(DESTINATION) == 2);
	move(curr_loc, med_loc);
	turn_east();
	forward_wls(1);
}

void print_all_result(void) {
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

void scan(unsigned char plot_no) {
	Point cords = get_cords(plot_no);
	medical_camp_map[cords.x][cords.y] = SCAN;
	while(traverse_line_to_goal(SCAN) == 2);
}

void fetch(unsigned char search_char) {
	while(traverse_line_to_goal(search_char) == 2);
	_delay_ms(10000);
}

void serve_request(void) {

	// Scan Request
	_delay_ms(2000);
	printf("Identify Survivor at plot 3\n");
	scan(3);

	_delay_ms(2000);
	printf("Fetch GREEN Survivor\n");
	fetch(GREEN);
}

void path_planning(void)
{
	initialize_medical_camp_map();

	forward_wls(1);

	scan_arena(DESTINATION);

	traverse_to_medical_camp();

	printf("TimeCounter = %lld\n", TimeCounter);

	print_all_result();

}
