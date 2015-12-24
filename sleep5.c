#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int arc, char** argv)
{
	printf("sleeping 1 sec");
	sleep(1);

	printf("sleeping 1 sec");
	sleep(1);

	printf("sleeping 1 sec");
	sleep(5);

	printf("sleeping 1 sec");
	sleep(1);

	printf("sleeping 1 sec");
	sleep(1);

	return EXIT_SUCCESS;
}