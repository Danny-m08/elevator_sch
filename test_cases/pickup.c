#include <stdio.h>
#include <unistd.h>
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

int main(int argc, char ** argv[])
{
	srand(time(0));

	Person p1;
	setParameters(&p1, 1, 3, 9);
	Person p2;
	setParameters(&p2, 1, 5, 10);


	issue_request(p1.type, p1.start_floor, p1.dest_floor);
	issue_request(p2.type, p2.start_floor, p2.dest_floor);
	return 0;
}
