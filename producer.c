#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "wrappers.h"

typedef struct passenger_type
{
	int weight; 
	int start_floor;
	int dest_floor;
	int type;
} Person;

int rnd(int min, int max) {
	return rand() % (max - min + 1) + min; //slight bias towards first k
}

Person generatePerson()
{
	Person person;
	person.type = rnd(0, 3);
	person.start_floor = rnd(1, 10);
	do 
	{
		person.dest_floor = rnd(1, 10);
	} while(person.dest_floor == person.start_floor);

	return person;
}

int main(int argc, char **argv) {
	srand(time(0));

	if (argc != 1) {
		printf("wrong number of args\n");
		return -1;
	}

	Person generatedPerson = generatePerson();
	long ret = issue_request(generatedPerson.type, generatedPerson.start_floor, generatedPerson.dest_floor);
	printf("Issue (%d, %d, %d) returned %ld\n", generatedPerson.type, generatedPerson.start_floor, generatedPerson.dest_floor, ret);

	return 0;
}
