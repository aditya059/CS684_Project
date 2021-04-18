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

#define SCAN 'S'                       				// Search char for scan request

#define DESTINATION 'D'								// Search char for scanning arena

#define TIME_TO_COVER_1_LINE_AND_TURN 2         	// 2 sec

#define TIME_INTERVAL_BETWEEN_EACH_REQUEST 45 		// Each request received after 45 sec

#define APPROX_FACTOR 1.76                      	// Used to approximate real life time in simulation

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

// To store arena seen so far
unsigned char medical_camp_map[SIZE][SIZE];

// To store whether bot has already turned or not at corner nodes
unsigned char turned[SIZE][SIZE];

// To count completion of 45 sec
double TimeCounter = 0.0;

// To store time remaining for satisfying a request
double TimeRemaining = 0.0;

// To compute total time expired in simulation
double TotalTimeExpired = 0.0;

// To store current received request number
unsigned char request_no = 0;

// To store number of request satisfied
unsigned char satisfied = 0;

// Used for storing shortest path
stack<Point> Stack;


//---------------------------------- FUNCTIONS ----------------------------------

/**
 * @brief      Executes the logic to achieve the aim of Project
 */

// For serving requests
void serve_request(void);

// To compute time
void compute_time(unsigned int ms) {
	TimeCounter += (ms / 1000.0) * APPROX_FACTOR;          	// To count time in sec. Used for counting 45 sec
	TimeRemaining -= (ms / 1000.0) * APPROX_FACTOR;			// To compute remaining time for request in sec
	TotalTimeExpired += (ms / 1000.0) * APPROX_FACTOR;  	// To compute total time for simulation
}

// To give delay as well as compute time elapsed
void ms_delay_and_compute_time(unsigned int ms) {
	_delay_ms(ms);
	compute_time(ms);
}

// To read Sensor values
void readSensor(void)
{
	left_wl_sensor_data = convert_analog_channel_data(left_wl_sensor_channel);
	center_wl_sensor_data = convert_analog_channel_data(center_wl_sensor_channel);
	right_wl_sensor_data = convert_analog_channel_data(right_wl_sensor_channel);
	//printf("L = %d C = %d R = %d\n", left_wl_sensor_data, center_wl_sensor_data, right_wl_sensor_data);
}


/*
*
* Function Name: forward_wls
* Input: node
* Output: void
* Logic: Uses white line sensors to go forward by the number of nodes specified
* Example Call: forward_wls(2); //Goes forward by two nodes
*
*/
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
				if (dir_flag == NORTH)
					curr_loc.x--;
				else if (dir_flag == SOUTH)
					curr_loc.x++;
				else if (dir_flag == WEST)
					curr_loc.y--;
				else
					curr_loc.y++;
				//printf("\nIntersection Reached. Current: (%d, %d). Direction: %c\n", curr_loc.x, curr_loc.y, dir_flag);
				forward();
				velocity(200, 200);
				ms_delay_and_compute_time(125);
				stop();
				break;
			}
			// The bot will turn left,then back to original position, then right and again back to original position at some angle until it find black line
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
				ms_delay_and_compute_time(s * 5);
				if (t % 4 == 3)
					s *= 2;
				t++;
				continue;
			}
			t = 0;
			s = 1;
			ms_delay_and_compute_time(10);
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
	//printf("Left Turn\n");
	//printf("Initial Direction = %c\n", dir_flag);
	int timer = 0;                      // Used to find direction by counting delays of 10 ms
	left();
	ms_delay_and_compute_time(100);
	velocity(100, 100);
	while (1)
	{
		timer++;
		readSensor();
		if (left_wl_sensor_data < 50 && center_wl_sensor_data > 200 && right_wl_sensor_data < 50)
		{
			stop();
			break;
		}
		ms_delay_and_compute_time(10);
	}
	//printf("timer = %d\n", timer);
	for (unsigned char i = 0; i <= timer / 40; i++)
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
	//printf("Final Direction = %c\n\n", dir_flag);
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
	//printf("Right Turn\n");
	//printf("Initial Direction = %c\n", dir_flag);
	int timer = 0;						 // Used to find direction by counting delays of 10 ms
	right();
	ms_delay_and_compute_time(100);
	velocity(100, 100);
	while (1)
	{
		timer++;
		readSensor();
		if (left_wl_sensor_data < 50 && center_wl_sensor_data > 200 && right_wl_sensor_data < 50)
		{
			stop();
			break;
		}
		ms_delay_and_compute_time(10);
	}
	//printf("Timer = %d\n", timer);
	for (unsigned char i = 0; i <= timer / 40; i++)
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
	//printf("Final Direction = %c\n\n", dir_flag);
}

// turn west if possible and return 1, otherwise return 0
unsigned char turn_west(void) {
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

// turn east if possible and return 1, otherwise return 0
unsigned char turn_east(void) {
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

// turn north if possible and return 1, otherwise return 0
unsigned char turn_north(void) {
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

// turn south if possible and return 1, otherwise return 0
unsigned char turn_south(void) {
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

// Takes plot no. as input and return its coordinate
Point get_cords(unsigned char plot_no)
{
	unsigned char plots_x[] = {8, 6, 4, 2}, plots_y[] = {7, 1, 3, 5};
	Point cords;
	int x = (plot_no - 1) / 4;
	cords.x = SIZE - plots_x[x];
	cords.y = plots_y[(plot_no % 4)];
	return cords;
}

// Used to initialise the arena in form of 2D Matrix
void initialize_medical_camp_map(void) {

	for(int i = 0; i < SIZE; i++) {
		for(int j = 0; j < SIZE; j++) {
			medical_camp_map[i][j] = 1;         // Initially bot has not seen arena so we fill everything with 1. 1 indicate that there is path and bot can move there. 0 will indicate obstacle
			turned[i][j] = 0;					// Stores 0 to indicate that bot has not turned there to check paths in every direction. 1 will indicate that bot has turned and already checked all directions
		}
	}

	Point cords;
	for (unsigned char i = 1; i <= NO_OF_PLOTS; i++)
	{
		cords = get_cords(i);
		medical_camp_map[cords.x][cords.y] = DESTINATION;     // The cells of 2D array which represent plot is filled with DESTINATION symbol which will be used for bfs search
	}

}

// To check whether plot had been checked earlier or not
unsigned char plot_not_checked(int x, int y) {

	if(x < 0 || x >= SIZE || y < 0 || y >= SIZE)          // Invalid coordinate
		return 0;

	if(medical_camp_map[x][y] == DESTINATION || medical_camp_map[x][y] == SCAN)     // Plot will be checked only when it stores SCAN or DESTINATION.
		return 1;
	return 0;
}

// Read the type of injury through color sensor and store in arena's 2D Matrix representation
void store_and_print_injury_type(int x, int y) {

	unsigned char plot_no = 4 * (x / 2) + (y / 2) + 1;

	medical_camp_map[x][y] = read_color_sensor_data();
	compute_time(1500);        // read_color_sensor_data gives delay of 1500 ms

	if(medical_camp_map[x][y] == GREEN)
		printf("\n\nPlot No. = %d - MinorInjury\n\n\n", plot_no);
	else if(medical_camp_map[x][y] == RED)
		printf("\n\nPlot No. = %d - MajorInjury\n\n\n", plot_no);
	else
		printf("\n\nPlot No. = %d - NoInjury\n\n\n", plot_no);
}

// Check plot if not checked earlier. Plot on left side of bot can be checked directly. For right side plot bot had to turn 180 degrees
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

// Check whether front direction is free or blocked by debris (indicated as WWW by color sensor)
unsigned char check_front(void) {
	readSensor();
	if(left_wl_sensor_data < 50 && center_wl_sensor_data < 50 && right_wl_sensor_data < 50) {
		return 0;
	}
	return 1;
}

// Check all possible paths at corner nodes so that we can compute arena's 2D Matrix representation quickly which will later help in finding path efficiently and quickly by bfs
void check_path_possible(int x, int y) {
	if(turned[x][y])										// Has already turned before to check all valid paths
		return;
	turned[x][y] = 1;

	if(dir_flag == NORTH) {                                     // If bot has come facing north so south exist. No need to check
		if(x - 1 >= 0 && medical_camp_map[x - 1][y] == 1)
			medical_camp_map[x - 1][y] = check_front();			// Check for path in north direction. If debris then fill 0 in corresponding cell of 2D arena representation
		if(y - 1 >= 0 && medical_camp_map[x][y - 1] == 1)
			medical_camp_map[x][y - 1] = turn_west();			// Check for path in west direction. If debris then fill 0 in corresponding cell of 2D arena representation
		if(y + 1 < SIZE && medical_camp_map[x][y + 1] == 1)
			medical_camp_map[x][y + 1] = turn_east();			// Check for path in east direction. If debris then fill 0 in corresponding cell of 2D arena representation
	}
	else if(dir_flag == SOUTH) {								// If bot has come facing south so north exist. No need to check
		if(x + 1 < SIZE && medical_camp_map[x + 1][y] == 1)
			medical_camp_map[x + 1][y] = check_front();			// Check for path in south direction. If debris then fill 0 in corresponding cell of 2D arena representation
		if(y - 1 >= 0 && medical_camp_map[x][y - 1] == 1)
			medical_camp_map[x][y - 1] = turn_west();
		if(y + 1 < SIZE && medical_camp_map[x][y + 1] == 1)
			medical_camp_map[x][y + 1] = turn_east();
	}
	else if(dir_flag == WEST) {									// If bot has come facing west so east exist. No need to check
		if(y - 1 >= 0 && medical_camp_map[x][y - 1] == 1)
			medical_camp_map[x][y - 1] = check_front();			// Check for path in west direction. If debris then fill 0 in corresponding cell of 2D arena representation
		if(x - 1 >= 0 && medical_camp_map[x - 1][y] == 1)
			medical_camp_map[x - 1][y] = turn_north();			// Check for path in north direction. If debris then fill 0 in corresponding cell of 2D arena representation
		if(x + 1 < SIZE && medical_camp_map[x + 1][y] == 1)
			medical_camp_map[x + 1][y] = turn_south();			// Check for path in south direction. If debris then fill 0 in corresponding cell of 2D arena representation
	}
	else { 														// If bot has come facing east so west exist. No need to check
		if(y + 1 < SIZE && medical_camp_map[x][y + 1] == 1)
			medical_camp_map[x][y + 1] = check_front();			// Check for path in east direction. If debris then fill 0 in corresponding cell of 2D arena representation
		if(x - 1 >= 0 && medical_camp_map[x - 1][y] == 1)
			medical_camp_map[x - 1][y] = turn_north();			// Check for path in north direction. If debris then fill 0 in corresponding cell of 2D arena representation
		if(x + 1 < SIZE && medical_camp_map[x + 1][y] == 1)
			medical_camp_map[x + 1][y] = turn_south();			// Check for path in south direction. If debris then fill 0 in corresponding cell of 2D arena representation
	}
}

// To check whether node is mid point node or not
unsigned char is_mid_node(Point &node) {
	if (((node.x & 1) == 0 && (node.y & 1) == 1) || ((node.x & 1) == 1 && (node.y & 1) == 0))
		return 1;
	return 0;
}

// To print all results at the end of simulation
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
	printf("\nTotal Time used for simulation = %f sec\n", TotalTimeExpired);
	printf("\nNo. of Request Received = %d\n", request_no);
	printf("\nNo. of Request satisfied = %d\n", satisfied);
}

// This function is move from one node to the next node
unsigned char move(Point &source, Point &destination) {

	// If bot is at corner node then check all direction for possible paths
	if(!is_mid_node(source)) {
		check_path_possible(source.x, source.y);
	}

	// If the cell corresponding to destination is 0, it means there is debris. So return and not move
	if (medical_camp_map[destination.x][destination.y] == 0)
		return 0;

	// Turn to appropriate direction
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

	// move 1 node forward
	forward_wls(1);

	// If it reach a mid node then take reading of plot if not taken earlier
	if(is_mid_node(destination))
		check_plot();

	return 1;
}

/*
 * search_char = 'R' for MajorInjury of fetch request, or
 *               'G' for MinorInjury of fetch request, or
 *               'D' for destination
 *               'S' for scan request
*/
// Reference : https://www.geeksforgeeks.org/shortest-distance-two-cells-matrix-grid/
// It perform bfs in 2D representation of arena to find path to search character. If path exist then it returns the no. of nodes from current location to destination, otherwise, returns 0
unsigned char bfs(unsigned char search_char) {

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

	if(!is_found)
		return 0;

	Stack = stack<Point>();
	Point temp = destination;
	while(temp.x != -1 && temp.y != -1) {
		Stack.push(temp);
		temp = parent[temp.x][temp.y];
	}
	//printf("\nSource = {%d, %d}, Destination = {%d, %d}, Direction = %c\n", curr_loc.x, curr_loc.y, destination.x, destination.y, dir_flag);
	return Stack.size();
}

// Used to traverse the path stored in stack (computed by bfs)
unsigned char traverse_line_to_goal() {

	Point temp = Stack.top();
	Stack.pop();
	while(!Stack.empty()) {

		if(TimeCounter > TIME_INTERVAL_BETWEEN_EACH_REQUEST) {                 // If 45 sec expired
			printf("\n%d sec expired. New Request might come\n", TIME_INTERVAL_BETWEEN_EACH_REQUEST);
			TimeCounter = 0.0;					// Reset timer to 0
			request_no++;						// Next request no.
			return 0;
		}

		Point temp1 = Stack.top();
		Stack.pop();
		if(!move(temp, temp1)) { 					// Use move function to reach destination
			return 2;								// If path is blocked by debris then return 2
		}
		temp = temp1;

		if(TimeCounter > TIME_INTERVAL_BETWEEN_EACH_REQUEST) {					// If 45 sec expired
			printf("\n%d sec expired. New Request might come\n", TIME_INTERVAL_BETWEEN_EACH_REQUEST);
			TimeCounter = 0.0;										// Reset timer to 0
			request_no++;											// Next request no.
			return 0;
		}

	}

	return 1;		// If reached destination then return 1
}

// Used to scan arena
void scan_arena(void) {
	while(bfs(DESTINATION)) {               // Scan until there is DESTINATION symbol in 2D representation of arena
		if(!traverse_line_to_goal()) {		// if 45 sec has expired then check for next request
			serve_request();
		}
	}
}

// Used to traverse to medical camp after scanning the entire arena
void traverse_to_medical_camp(void) {
	medical_camp_map[med_loc.x][med_loc.y] = DESTINATION;
	while(bfs(DESTINATION) && traverse_line_to_goal() == 2);  	// Find path and go until we reach at destination
	move(curr_loc, med_loc);									// The bot will reach 1 node before the node connected to medical camp due to my implementation. So use move function once to move to node connected to medical camp
	turn_east();												// Turn east to face medical camp
	forward_wls(1);												// go forward to end at medical camp
}

// Used for scan request
void scan(unsigned char plot, unsigned char completeIn) {
	printf("\nRequest No. %d : Identify Survivor at plot %d in %d seconds\n", request_no, plot, completeIn);

	// This delay is not required. Just given so that we can see request in terminal. So it is not used for computing time elapsed
	_delay_ms(2000);

	TimeRemaining = completeIn;                // Stores time remaining for completing the request
	Point cords = get_cords(plot);
	unsigned char prev_value = medical_camp_map[cords.x][cords.y];  // Store previous value so that if we are not able to reach the plot, we can keep it back later
	medical_camp_map[cords.x][cords.y] = SCAN;    // SCAN symbol will be used as search char in bfs
	unsigned char reached = 0;

	while((bfs(SCAN) - 1) * TIME_TO_COVER_1_LINE_AND_TURN <= TimeRemaining - 5) {
		if(traverse_line_to_goal() == 1) {
			reached = 1;
			satisfied++;
			printf("\nRequest No. %d Satisfied. Buzz the Buzzer\n", request_no);
			break;
		}
	}

	if(!reached) {
		medical_camp_map[cords.x][cords.y] = prev_value;
		printf("\nRequest No. %d Not Satisfied\n", request_no);
	}

}

void fetch_nearest(unsigned char search_char, unsigned char completeIn) {
	if(search_char == 'R')
		printf("\nRequest No. %d : Fetch nearest RED survivor in %d seconds\n", request_no, completeIn);
	else
		printf("\nRequest No. %d : Fetch nearest GREEN survivor in %d seconds\n", request_no, completeIn);

	// This delay is not required. Just given so that we can see request in terminal. So it is not used for computing time elapsed
	_delay_ms(2000);

	TimeRemaining = completeIn;				 // Stores time remaining for completing the request

	unsigned char reached = 0;
	if(bfs(search_char)) {					// It might be the case that there is no search_char in arena, i.e., we have not seen that particular injury earlier
		while((bfs(search_char) - 1) * TIME_TO_COVER_1_LINE_AND_TURN <= TimeRemaining - 5) {
			if(traverse_line_to_goal() == 1) {
				reached = 1;
				satisfied++;
				printf("\nRequest No. %d Satisfied. Buzz the Buzzer\n", request_no);
				ms_delay_and_compute_time(1000);
				break;
			}
		}
	}
	if(!reached) {
		printf("\nRequest No. %d Not Satisfied\n", request_no);
	}

}

// Used for serving requests
void serve_request(void) {

	if(request_no == 1) {
		// Scan Request
		scan(13, 40);
	}
	else if(request_no == 2) {
		// Fetch Request
		fetch_nearest(RED, 10);
	}
	else if(request_no == 3) {
		// Empty Request
	}
	else if(request_no == 4) {
		// Fetch Request
		fetch_nearest(GREEN, 10);
	}
	else if(request_no == 5) {
		// Scan Request
		scan(1, 40);
	}
	else if(request_no == 6) {
		// Fetch Request
		fetch_nearest(GREEN, 10);
	}
	else if(request_no == 7) {
		// Scan Request
		scan(4, 30);
	}
	else if(request_no == 8) {
		// Fetch Request
		fetch_nearest(RED, 10);
	}
	else if(request_no == 9) {
		// Scan Request
		scan(15, 40);
	}
	else if(request_no == 10) {
		// Fetch Request
		fetch_nearest(GREEN, 10);
	}
	else if(request_no == 11) {
		// Scan Request
		scan(2, 30);
	}
	else if(request_no == 12) {
		// Fetch Request
		fetch_nearest(RED, 10);
	}
	else if(request_no == 13) {
		// Scan Request
		scan(3, 40);
	}

}

// Path Planning algorithm
void path_planning(void)
{

	initialize_medical_camp_map();

	forward_wls(1);

	scan_arena();

	TimeCounter = -20000.0;  // Any big negative value so that after arena scan, bot don't wait for more request and just go to medical camp

	traverse_to_medical_camp();

	print_all_result();

}
