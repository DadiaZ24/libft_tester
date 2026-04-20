#include "tester.h"

int main()
{
	int fails = 0;

	HEADER();
	sleep(1);
	ATENTION();
	sleep(2);

	fails += testing_ft_hardcore();

	END(fails);
	return 0;
}
