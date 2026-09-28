# **************************************************************************** #
#                                  Makefile                                    #
# **************************************************************************** #
#
#  make          -> subject checks + build libft + build tester + run everything
#  make test     -> build + run the tests only
#  make check    -> subject checks only (Makefile, relink, forbidden functions,
#                   global variables, norminette, README...)
#  make run T=split  -> only ft_split (or the tests whose name contains "split")
#  make run V=1      -> one line per test instead of one line per function
#
#  The screen only gets one line per check / test.
#  Every detail (why it failed, compiler output...) goes to traces.log.
#

NAME        = tester

SRC         = main.c framework.c libft_api.c test_prototypes.c test_part1.c \
              test_part2.c test_part3.c test_memory.c
BUILD       = .build
OBJ         = $(SRC:%.c=$(BUILD)/%.o)

LIBFT_DIR   = ..
TRACE       = traces.log

CC          = cc
# -O0 and -fno-builtin: the compiler must not optimise away the tester's
# own malloc/free/memcmp calls or assume things about them.
CFLAGS      = -Wall -Wextra -O0 -g -fno-builtin -D_GNU_SOURCE -I$(BUILD) -I$(LIBFT_DIR)
# whole-archive: the tester has a weak default for every libft function (see
# libft_api.c), so nothing would pull yours out of the archive otherwise.
LDFLAGS     = -Wl,--whole-archive $(BUILD)/libft.a -Wl,--no-whole-archive \
              -Wl,--allow-multiple-definition -ldl
# last resort, when your libft calls something that exists nowhere (a helper
# in a file that doesn't compile...): link anyway, that call will crash.
LDFLAGS_ANY = -no-pie -Wl,--unresolved-symbols=ignore-all

MAKEFLAGS  += --no-print-directory

all: check test

test: trace
	@CC="$(CC)" bash ./prepare.sh $(LIBFT_DIR) $(BUILD) $(TRACE)
	@printf "\n==================== BUILD: tester ====================\n\n" >> $(TRACE)
	@$(MAKE) $(NAME) >> $(TRACE) 2>&1 || { rm -f $(NAME); \
		printf "  %-70s \e[1;31m[KO]\e[0m \e[2msee traces\e[0m\n" "build the tester"; }
	@[ -x ./$(NAME) ] && LIBFT_TESTER_TRACE=$(TRACE) ./$(NAME) $(if $(V),-v) $(T) || true

run: test

check: trace
	@bash ./check_project.sh $(LIBFT_DIR) $(TRACE)

trace:
	@printf "LIBFT TESTER - TRACES (%s)\n" "$$(date)" > $(TRACE)

$(BUILD)/%.o: %.c tester.h libft_api.h $(BUILD)/probe.h $(wildcard $(LIBFT_DIR)/libft.h)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJ) $(BUILD)/libft.a
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS) \
		|| $(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS) $(LDFLAGS_ANY)

clean:
	@rm -rf $(BUILD) *.o
	@echo "🧹 Cleaned object files"

fclean: clean
	@rm -f $(NAME) $(TRACE)
	@$(MAKE) fclean -C $(LIBFT_DIR) > /dev/null 2>&1 || true
	@echo "🧼 Fully cleaned $(NAME)"

re: fclean all

.PHONY: all test run check trace clean fclean re
