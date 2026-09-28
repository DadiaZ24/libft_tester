# **Libft Tester** (subject v19.3)

A tester for the 42 `libft`, built around the things that give a **0** in evaluation:
**segfaults, infinite loops, memory leaks, double frees and unprotected mallocs**.

Every test runs in its **own process**, so a crash or an infinite loop is reported
(with the exact call that caused it) and the tester keeps going.

## Setup

Clone it **inside** your libft repository (next to `libft.h` and your `Makefile`):

```bash
cd libft
git clone <this repo> libft_tester
cd libft_tester
make
```

> ⚠️ Don't push `libft_tester/` to your libft repo: the subject forbids unused files.
> Add it to `.git/info/exclude` or delete it before submitting.

Linux (glibc) is required: 42 machines, WSL or a Docker Ubuntu all work.

## Commands

| command             | what it does                                                        |
|---------------------|---------------------------------------------------------------------|
| `make`              | subject checks + builds your libft + runs every test               |
| `make test`         | only the tests                                                      |
| `make check`        | only the subject checks (Makefile, relink, forbidden functions...) |
| `make run T=split`  | only the tests whose name contains `split`                          |
| `./tester lstmap`   | same thing, directly                                                |
| `make fclean`       | cleans the tester **and** your libft                               |

## What is detected

The tester replaces `malloc` / `free` for your libft. Every block gets:

- **garbage bytes** (`0xBE`) instead of zeros, so a missing `'\0'` or a missing `node->next = NULL` fails instead of "working by luck".
- **16 canary bytes** after the end, so writing a single byte too far is reported as a heap buffer overflow.
- **quarantine on free**: freed memory is filled with `0xDF` and never reused, so reading `node->next` after `free(node)` crashes straight away and writing to freed memory is reported.
- a table of every allocation, so **leaks**, **double frees** and results that are not `malloc`'d (for example `return ("")`) are caught.

Inputs are also placed against **guard pages** (`[guarded]` / `[front guarded]`):
reading one byte before or after a string is a segfault, with the call printed.

| result            | meaning                                                         |
|-------------------|-----------------------------------------------------------------|
| `[OK]`            | fine                                                            |
| `[KO]`            | wrong result, crash, timeout, leak, overflow...                 |
| `[WARN]`          | undefined behaviour (NULL args, 1M-node lists...) that evaluators like to try. It does not count as an error. |

### Memory killers

- **Malloc failure at every point**: each allocating function runs once to count its mallocs, then once per malloc with *exactly that one* returning `NULL`. At every point it must not crash, must return `NULL`, and must free what it already allocated. For `ft_lstmap`, it must also `del()` the content returned by `f`.
- **One-shot chain**: `substr > strtrim > strjoin > split > itoa > strdup > strmapi > calloc > lstnew > lstmap > lstclear` on guarded inputs, with the malloc failure injected at every point of the whole chain.
- **1 000 000-node lists**: a recursive `ft_lstclear` / `ft_lstmap` / `ft_lstsize` overflows the stack.
- **5000 repetitions** of every allocating function, to catch leaks that only happen in one branch.

### Subject checks (`make check`)

- Required files, only `ft_*.c`, no `*_bonus` files (the lists are mandatory in v19.3), no unused files
- Makefile rules `$(NAME) all clean fclean re`, `cc`, `-Wall -Wextra -Werror`, `ar`, no `libtool`, no `-std=c99`
- **Relink**: a second `make` must do nothing, and touching one `.c` must recompile only that file
- `nm libft.a`: all 43 functions present, **only `malloc` / `free` / `write`** used, **no global variables** (static ones included), helpers are `static`
- no `restrict`, `norminette` (if installed)
- README: italic first line, and the Description / Instructions / Resources sections, including how AI was used

### Prototypes (v19.3)

All 43 prototypes are checked at compile time. Note that **`ft_lstsize` returns `unsigned int`**
in this version, and the `ft_is*` functions must return **exactly 1 or 0**.
