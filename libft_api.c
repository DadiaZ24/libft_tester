#include "tester.h"

/*
** Every libft function gets a weak default here. Yours (in libft.a) replace
** them at link time; a function you don't have yet lands in its stub, which
** reports it as [MISSING] instead of breaking the build. The stubs take no
** arguments: whatever the caller passes is just ignored.
*/

#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wattribute-alias"

#define STUB(fn) \
	static void	stub_##fn(void) { t_missing_call(#fn); } \
	__typeof__(fn) fn __attribute__((weak, alias("stub_" #fn)));

LIBFT_FUNCS(STUB)

typedef struct s_func
{
	const char	*name;
	void		*addr;
	void		*stub;
	int			declared;
}	t_func;

#define ENTRY(fn) {#fn, (void *)&fn, (void *)&stub_##fn, DECL_##fn},

static const t_func	g_funcs[] = {LIBFT_FUNCS(ENTRY)};

#define NFUNCS (sizeof(g_funcs) / sizeof(*g_funcs))

int	t_func_exists(const char *fn, int *declared)
{
	size_t	i;

	i = 0;
	while (i < NFUNCS)
	{
		if (!strcmp(g_funcs[i].name, fn))
		{
			if (declared)
				*declared = g_funcs[i].declared;
			return (g_funcs[i].addr != g_funcs[i].stub);
		}
		i++;
	}
	if (declared)
		*declared = 0;
	return (0);
}
