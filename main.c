#include "tester.h"

/*
** ./tester              -> everything, one line per function
** ./tester -v           -> one line per test
** ./tester split        -> only ft_split (or the tests whose name has "split")
** What each test checks and why it failed goes to the trace file.
*/
int	main(int argc, char **argv)
{
	int	i;
	int	filtered;

	setvbuf(stdout, NULL, _IOLBF, 0);
	t_trace_open();
	filtered = 0;
	i = 1;
	while (i < argc)
	{
		if (!strcmp(argv[i], "-v"))
			t_set_verbose(1);
		else if (argv[i][0])
		{
			t_set_filter(argv[i]);
			filtered = 1;
		}
		i++;
	}
	if (!filtered)
		HEADER();
	run_prototypes();
	run_part1();
	run_part2();
	run_part3();
	run_memory();
	t_print_summary();
	return (t_total_fails() != 0 || t_total_missing() != 0);
}
