#include "tester.h"
#include <fcntl.h>
#include <unistd.h>

// ===== FT_STRJOIN =====

static int test_strjoin_basic()
{
	int fails = 0;
	printf("[TEST] strjoin Basic\n");
	char *res = ft_strjoin("Hello", " World");
	if (res && strcmp(res, "Hello World") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(res);
	return fails;
}

static int test_strjoin_empty_s1()
{
	int fails = 0;
	printf("[TEST] strjoin Empty s1\n");
	char *res = ft_strjoin("", "World");
	if (res && strcmp(res, "World") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(res);
	return fails;
}

static int test_strjoin_empty_s2()
{
	int fails = 0;
	printf("[TEST] strjoin Empty s2\n");
	char *res = ft_strjoin("Hello", "");
	if (res && strcmp(res, "Hello") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(res);
	return fails;
}

static int test_strjoin_both_empty()
{
	int fails = 0;
	printf("[TEST] strjoin Both Empty\n");
	char *res = ft_strjoin("", "");
	if (res && strcmp(res, "") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(res);
	return fails;
}

static int test_strjoin_numbers()
{
	int fails = 0;
	printf("[TEST] strjoin Numbers\n");
	char *res = ft_strjoin("42", "school");
	if (res && strcmp(res, "42school") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(res);
	return fails;
}

// ===== FT_PUTCHAR_FD / FT_PUTSTR_FD / FT_PUTENDL_FD / FT_PUTNBR_FD =====

static int test_putchar_fd()
{
	int fails = 0;
	printf("[TEST] putchar_fd to /dev/null (no crash)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	int fd = open("/dev/null", O_WRONLY);
	if (fd == -1)
	{
		printf("  ⚠️  Could not open /dev/null\n");
		return fails;
	}
	ft_putchar_fd('A', fd);
	ft_putchar_fd('\n', fd);
	ft_putchar_fd(0, fd);
	close(fd);
	PASS();
	return fails;
}

static int test_putstr_fd()
{
	int fails = 0;
	printf("[TEST] putstr_fd to /dev/null (no crash)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	int fd = open("/dev/null", O_WRONLY);
	if (fd == -1)
	{
		printf("  ⚠️  Could not open /dev/null\n");
		return fails;
	}
	ft_putstr_fd("Hello World", fd);
	ft_putstr_fd("", fd);
	close(fd);
	PASS();
	return fails;
}

static int test_putendl_fd()
{
	int fails = 0;
	printf("[TEST] putendl_fd to /dev/null (no crash)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	int fd = open("/dev/null", O_WRONLY);
	if (fd == -1)
	{
		printf("  ⚠️  Could not open /dev/null\n");
		return fails;
	}
	ft_putendl_fd("Hello", fd);
	ft_putendl_fd("", fd);
	close(fd);
	PASS();
	return fails;
}

static int test_putnbr_fd()
{
	int fails = 0;
	printf("[TEST] putnbr_fd to /dev/null (no crash)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	int fd = open("/dev/null", O_WRONLY);
	if (fd == -1)
	{
		printf("  ⚠️  Could not open /dev/null\n");
		return fails;
	}
	ft_putnbr_fd(0, fd);
	ft_putnbr_fd(42, fd);
	ft_putnbr_fd(-42, fd);
	ft_putnbr_fd(2147483647, fd);
	ft_putnbr_fd(-2147483648, fd);
	close(fd);
	PASS();
	return fails;
}

static int test_putchar_fd_output()
{
	int fails = 0;
	printf("[TEST] putchar_fd Output Verification\n");

	char tmpfile[] = "/tmp/libft_putchar_test_XXXXXX";
	int fd = mkstemp(tmpfile);
	if (fd == -1)
	{
		printf("  ⚠️  Could not create temp file\n");
		return fails;
	}
	ft_putchar_fd('X', fd);
	close(fd);

	fd = open(tmpfile, O_RDONLY);
	char buf[2] = {0};
	read(fd, buf, 1);
	close(fd);
	unlink(tmpfile);

	if (buf[0] == 'X')
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_putstr_fd_output()
{
	int fails = 0;
	printf("[TEST] putstr_fd Output Verification\n");

	char tmpfile[] = "/tmp/libft_putstr_test_XXXXXX";
	int fd = mkstemp(tmpfile);
	if (fd == -1)
	{
		printf("  ⚠️  Could not create temp file\n");
		return fails;
	}
	ft_putstr_fd("hi", fd);
	close(fd);

	fd = open(tmpfile, O_RDONLY);
	char buf[3] = {0};
	read(fd, buf, 2);
	close(fd);
	unlink(tmpfile);

	if (strcmp(buf, "hi") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_putendl_fd_output()
{
	int fails = 0;
	printf("[TEST] putendl_fd Output Verification (ends with newline)\n");

	char tmpfile[] = "/tmp/libft_putendl_test_XXXXXX";
	int fd = mkstemp(tmpfile);
	if (fd == -1)
	{
		printf("  ⚠️  Could not create temp file\n");
		return fails;
	}
	ft_putendl_fd("hi", fd);
	close(fd);

	fd = open(tmpfile, O_RDONLY);
	char buf[4] = {0};
	read(fd, buf, 3);
	close(fd);
	unlink(tmpfile);

	if (buf[0] == 'h' && buf[1] == 'i' && buf[2] == '\n')
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_putnbr_fd_output()
{
	int fails = 0;
	printf("[TEST] putnbr_fd Output Verification\n");

	char tmpfile[] = "/tmp/libft_putnbr_test_XXXXXX";
	int fd = mkstemp(tmpfile);
	if (fd == -1)
	{
		printf("  ⚠️  Could not create temp file\n");
		return fails;
	}
	ft_putnbr_fd(42, fd);
	close(fd);

	fd = open(tmpfile, O_RDONLY);
	char buf[3] = {0};
	read(fd, buf, 2);
	close(fd);
	unlink(tmpfile);

	if (strcmp(buf, "42") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

int testing_ft_strjoin_putfd()
{
	int fails = 0;

	TITLE("FT_STRJOIN");
	sleep(1);
	fails += test_strjoin_basic();
	sleep(1);
	fails += test_strjoin_empty_s1();
	sleep(1);
	fails += test_strjoin_empty_s2();
	sleep(1);
	fails += test_strjoin_both_empty();
	sleep(1);
	fails += test_strjoin_numbers();
	sleep(1);

	TITLE("FT_PUT*_FD");
	sleep(1);
	fails += test_putchar_fd();
	sleep(1);
	fails += test_putstr_fd();
	sleep(1);
	fails += test_putendl_fd();
	sleep(1);
	fails += test_putnbr_fd();
	sleep(1);
	fails += test_putchar_fd_output();
	sleep(1);
	fails += test_putstr_fd_output();
	sleep(1);
	fails += test_putendl_fd_output();
	sleep(1);
	fails += test_putnbr_fd_output();
	sleep(1);

	printf("\n===== Testing Complete =====\n");
	return fails;
}
