#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../wrappers.h"

typedef struct passenger_type
{
	int weight;
	int start_floor;
	int dest_floor;
	int type;
} Person;


void setParameters(Person * s, int t, int sf, int ds)
{
	s -> weight = 0;
	s -> start_floor = sf;
	s -> dest_floor = ds;
	s -> type = t;
}

int main(int argc, char ** argc[])
{
	srand(time(0));

	Person p1;
	setParameters(&p1, 1, 3, 5);
	Person p2;
	setParmeters(&p1, 1, 3, 6);


	issue_request(p1.type, p1.start_floor, p1.dest_floor);
	issue_request(p2.type, p2.start_floor, p2.dest_floor);
	return 0;
}
