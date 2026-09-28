# **************************************************************************** #
#                                  Makefile                                    #
# **************************************************************************** #
#
#  make          -> subject checks + build libft + build tester + run everything
#  make test     -> build + run the tests only
#  make check    -> subject checks only (Makefile, relink, forbidden functions,
#                   global variables, norminette, README...)
#  make run T=split  -> only the tests whose name contains "split"
#

NAME        = tester

SRC         = main.c framework.c test_prototypes.c test_part1.c test_part2.c \
              test_part3.c test_memory.c
OBJ         = $(SRC:.c=.o)

LIBFT_DIR   = ..
LIBFT_LIB   = $(LIBFT_DIR)/libft.a

CC          = cc
# -O0 and -fno-builtin: the compiler must not optimise away the tester's
# own malloc/free/memcmp calls or assume things about them.
CFLAGS      = -Wall -Wextra -O0 -g -fno-builtin -D_GNU_SOURCE -I$(LIBFT_DIR)
LDFLAGS     = -L$(LIBFT_DIR) -lft -ldl

all: check test

test: $(NAME)
	@./$(NAME) $(T) || true

run: test

check:
	@bash ./check_project.sh $(LIBFT_DIR)

libft:
	@echo "📦 Building libft..."
	@$(MAKE) -C $(LIBFT_DIR) --no-print-directory

%.o: %.c tester.h $(LIBFT_DIR)/libft.h
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): libft $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)
	@echo "✔️  Built $(NAME)"

clean:
	@rm -f $(OBJ)
	@echo "🧹 Cleaned object files"

fclean: clean
	@rm -f $(NAME)
	@$(MAKE) fclean -C $(LIBFT_DIR) --no-print-directory
	@echo "🧼 Fully cleaned $(NAME)"

re: fclean all

.PHONY: all test run check libft clean fclean re
