#include "tester.h"

int main()
{
	int fails = 0;

	HEADER();
	sleep(1);
	ATENTION();
	sleep(2);
	fails += testing_ft_split();
	fails += testing_ft_calloc();
	fails += testing_ft_memmove();
	fails += testing_ft_memcpy();
	fails += testing_ft_atoi();
	fails += testing_ft_itoa();
	fails += testing_ft_string_funcs();
	fails += testing_ft_strlcpy_strlcat();
	fails += testing_ft_strl();
	fails += testing_ft_striteri_bzero_strncmp();
	fails += testing_ft_isalpha_isdigit_toupper_tolower();
	fails += testing_ft_isalnum_isascii_isprint();
	fails += testing_ft_strlen_memset_memchr_memcmp();
	fails += testing_ft_strjoin_putfd();
	fails += testing_ft_hardcore();

	END(fails);
	return 0;
}