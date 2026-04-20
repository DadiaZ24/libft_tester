#include "tester.h"

int main()
{
	int fails = 0;

	HEADER();
	sleep(1);
	ATENTION();
	sleep(2);
	fails += testing_ft_calloc();
	fails += testing_ft_memmove();
	fails += testing_ft_memcpy();
	fails += testing_ft_atoi();
	fails += testing_ft_itoa();
	fails += testing_ft_string_funcs();
	fails += testing_ft_split();
	fails += testing_ft_strlcpy_strlcat();
	fails += testing_ft_strl();
	fails += testing_ft_striteri_bzero_strncmp();
	fails += testing_ft_isalpha_isdigit_toupper_tolower();

	fails += testing_ft_isalnum_isascii_isprint();
	fails += testing_ft_strlen_memset_memchr_memcmp();
	fails += testing_ft_strjoin_putfd();

	// BONUS
	fails += testing_ft_lst_last();
	fails += testing_ft_lstclear();
	fails += testing_ft_lstadd_back();
	fails += testing_ft_lstadd_front();
	fails += testing_ft_lstnew_lstsize_lstdelone();
	fails += testing_ft_lstiter_lstmap();
	fails += testing_ft_hardcore();
	fails += testing_ft_lstmap_hardcore();

	END(fails);
	return 0;
}