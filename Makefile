# **************************************************************************** #
#                                  Makefile                                    #
# **************************************************************************** #

NAME             = tester
MAIN             = main.c
MAIN_BONUS       = main_bonus.c

SRC              = ft_calloc_tester.c ft_memmove_tester.c ft_memcpy_tester.c ft_atoi_tester.c ft_itoa_tester.c \
                   ft_split_tester.c ft_str_tester.c ft_strlcatcpy_tester.c ft_strtrimmapi_tester.c ft_bzerostrncmpiteri_tester.c \
                   ft_alphadigitupperlower_tester.c

SRC_BONUS        = bonus/ft_lstlast_tester.c bonus/ft_lstadd_back_tester.c bonus/ft_lstclear_tester.c bonus/ft_lstadd_front_tester.c 
					

CC               = cc
CFLAGS           = -O2 -g -I../
LDFLAGS          = -L../ -lft -lpthread

OBJ              = $(SRC:.c=.o)
OBJ_BONUS        = $(SRC_BONUS:.c=.o)
MAIN_OBJ         = $(MAIN:.c=.o)
MAIN_BONUS_OBJ   = $(MAIN_BONUS:.c=.o)

LIBFT_DIR        = ../
LIBFT_LIB        = $(LIBFT_DIR)/libft.a

# Pattern rule to compile .c files into .o files (including those in subdirs)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Default rule
all: $(LIBFT_LIB) $(NAME) run

$(LIBFT_LIB):
	@echo "📦 Building libft..."
	@make -C $(LIBFT_DIR)

$(NAME): $(OBJ) $(MAIN_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "✔️  Built $(NAME)"

bonus: bonus_libft $(LIBFT_LIB) $(OBJ) $(OBJ_BONUS) $(MAIN_BONUS_OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(OBJ_BONUS) $(MAIN_BONUS_OBJ) -o $(NAME) $(LDFLAGS)
	@echo "✔️  Built bonus $(NAME)"
	@./$(NAME) 
	@make fclean --silent
	@echo "✔️  Program executed and cleaned"

bonus_libft:
	@echo "📦 Building libft with bonus..."
	@make -C $(LIBFT_DIR)
	@make bonus -C $(LIBFT_DIR)

clean:
	@rm -f $(OBJ) $(OBJ_BONUS) $(MAIN_OBJ) $(MAIN_BONUS_OBJ)
	@echo "🧹 Cleaned object files"

fclean: clean
	@rm -f $(NAME)
	@make fclean -C $(LIBFT_DIR)
	@echo "🧼 Fully cleaned $(NAME)"

run: 
	@./$(NAME)
	@make fclean --silent
	@echo "✔️  Program executed and cleaned"

re: fclean all
