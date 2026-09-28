#include "tester.h"

/*
** ./tester            -> everything
** ./tester split      -> only the tests whose name contains "split"
*/
int	main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IOLBF, 0);
	if (argc > 1)
		t_set_filter(argv[1]);
	else
	{
		HEADER();
		ATENTION();
	}
	run_prototypes();
	run_part1();
	run_part2();
	run_part3();
	run_memory();
	t_print_summary();
	END(t_total_fails(), t_total_warns());
	return (t_total_fails() != 0);
}
