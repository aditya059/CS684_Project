#include <stdio.h>

typedef struct
{
	unsigned char x, y;
}tuple;

tuple curr_loc = {0,4}, med_loc = {5,9};

tuple get_cords(unsigned char plot_no){
	unsigned char plots_x[] = {8,6,4,2}, plots_y[] = {7,1,3,5};
	tuple cords;
	int x;
	x = (plot_no - 1)/4;
	cords.x = plots_x[x];
	cords.y = plots_y[(plot_no % 4)];
	return cords;
}



int main(void) {
	tuple cords;
	for(int i=1;i<=16;i++){
		cords = get_cords(i);
		printf("Plot %d : (%d, %d)\n", i, cords.x, cords.y);
		printf("Left Cords: (%d, %d)\n", cords.x, cords.y-1);
		printf("Right Cords: (%d, %d)\n", cords.x, cords.y+1);
		printf("Top Cords: (%d, %d)\n", cords.x+1, cords.y);
		printf("Bottom Cords: (%d, %d)\n\n", cords.x-1, cords.y);
	}
	
	return 0;
}
