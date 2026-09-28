#!/usr/bin/env bash
# Subject v19.3 checks that can't be done from C:
# files, Makefile rules/flags, relink, ar, forbidden functions, global
# variables, non-static helpers, restrict, norminette, README.

DIR="${1:-..}"
TRACE="${2:-/dev/null}"   # why a check failed goes here, not on the screen
RED=$'\e[1;31m'; GRN=$'\e[1;32m'; YEL=$'\e[1;33m'; CYN=$'\e[0;36m'; DIM=$'\e[2m'; RST=$'\e[0m'
KO=0; WARN=0

detail() {
	printf "%-9s %s\n" "$1" "$2" >> "$TRACE"
	[ -n "$3" ] && printf "%s\n" "$3" | sed 's/^/      ✗ /' >> "$TRACE"
	[ "$1" != "[OK]" ] && printf "\n" >> "$TRACE"
	return 0
}
ok()   { printf "  %-70s ${GRN}[OK]${RST}\n" "$1"; detail "[OK]" "$1"; }
ko()   { printf "  %-70s ${RED}[KO]${RST}\n" "$1"; detail "[KO]" "$1" "$2"; KO=$((KO+1)); }
warn() { printf "  %-70s ${YEL}[WARN]${RST}\n" "$1"; detail "[WARN]" "$1" "$2"; WARN=$((WARN+1)); }

REQUIRED="ft_isalpha ft_isdigit ft_isalnum ft_isascii ft_isprint ft_strlen ft_memset ft_bzero
ft_memcpy ft_memmove ft_strlcpy ft_strlcat ft_toupper ft_tolower ft_strchr ft_strrchr ft_strncmp
ft_memchr ft_memcmp ft_strnstr ft_atoi ft_calloc ft_strdup ft_substr ft_strjoin ft_strtrim
ft_split ft_itoa ft_strmapi ft_striteri ft_putchar_fd ft_putstr_fd ft_putendl_fd ft_putnbr_fd
ft_lstnew ft_lstadd_front ft_lstsize ft_lstlast ft_lstadd_back ft_lstdelone ft_lstclear
ft_lstiter ft_lstmap"
ALLOWED_EXT="malloc free write __stack_chk_fail _GLOBAL_OFFSET_TABLE_ __stack_chk_guard"

printf "${CYN}\n ========= SUBJECT CHECKS (%s) =========== \n\n${RST}" "$(cd "$DIR" && pwd)"
printf "\n==================== SUBJECT CHECKS (%s) ====================\n\n" "$(cd "$DIR" && pwd)" >> "$TRACE"

# ------------------------------------------------------------------ files
for f in Makefile libft.h README.md; do
	[ -f "$DIR/$f" ] && ok "$f exists at the root" || ko "$f exists at the root" "missing"
done
bad=$(cd "$DIR" && ls *.c 2>/dev/null | grep -v '^ft_.*\.c$' | tr '\n' ' ')
[ -z "$bad" ] && ok "only ft_*.c source files" || ko "only ft_*.c source files" "unexpected: $bad"
bonus=$(cd "$DIR" && ls *_bonus.c *_bonus.h 2>/dev/null | tr '\n' ' ')
[ -z "$bonus" ] && ok "no *_bonus files (v19.3: lists are mandatory, no bonus)" \
	|| warn "no *_bonus files (v19.3: lists are mandatory, no bonus)" "found: $bonus"
extra=$(cd "$DIR" && ls -A | grep -vE '^(Makefile|libft\.h|README\.md|ft_.*\.c|\.git|\.gitignore|libft_tester|.*\.o|libft\.a)$' | tr '\n' ' ')
[ -z "$extra" ] && ok "no unused files at the root" || warn "no unused files at the root" "check these: $extra"
if git -C "$DIR" rev-parse >/dev/null 2>&1; then
	tracked=$(git -C "$DIR" ls-files | grep -E '(^|/)libft_tester/' | head -1)
	[ -z "$tracked" ] && ok "the tester is not committed in your repo" \
		|| warn "the tester is not committed in your repo" "remove it before pushing (unused files are not allowed)"
fi

# --------------------------------------------------------------- libft.h
H="$DIR/libft.h"
if [ -f "$H" ]; then
	grep -qE '^#\s*ifndef' "$H" && grep -qE '^#\s*define' "$H" && ok "libft.h has an include guard" \
		|| warn "libft.h has an include guard"
	if grep -rqw restrict "$DIR"/*.c "$H" 2>/dev/null; then
		ko "no 'restrict' anywhere (C99 keyword, forbidden)" "$(grep -rlw restrict "$DIR"/*.c "$H" | tr '\n' ' ')"
	else
		ok "no 'restrict' anywhere (C99 keyword, forbidden)"
	fi
fi

# -------------------------------------------------------------- Makefile
M="$DIR/Makefile"
if [ -f "$M" ]; then
	for r in '\$\(NAME\)' all clean fclean re; do
		grep -qE "^$r[[:space:]]*:" "$M" && ok "Makefile rule ${r//\\/}" || ko "Makefile rule ${r//\\/}" "missing"
	done
	grep -qE '^NAME[[:space:]]*:?=[[:space:]]*libft\.a' "$M" && ok "NAME = libft.a" || ko "NAME = libft.a"
	for fl in -Wall -Wextra -Werror; do
		grep -q -- "$fl" "$M" && ok "Makefile uses $fl" || ko "Makefile uses $fl"
	done
	grep -qE '(^|[^a-z])cc([^a-z]|$)' "$M" && ok "Makefile compiles with cc" || warn "Makefile compiles with cc" "subject: use cc"
	grep -q 'libtool' "$M" && ko "no libtool" "libtool is forbidden, use ar" || ok "no libtool"
	grep -qE '(^|[^a-z])ar[[:space:]]|\$\(AR\)' "$M" && ok "library created with ar" || ko "library created with ar"
	grep -q -- '-std=c99' "$M" && ko "no -std=c99" "forbidden by the subject" || ok "no -std=c99"

	make -C "$DIR" fclean >/dev/null 2>&1
	out=$(make -C "$DIR" 2>&1); rc=$?
	if [ $rc -ne 0 ] || [ ! -f "$DIR/libft.a" ]; then
		ko "make builds libft.a at the root" "$(echo "$out" | grep -iE 'error' | head -5)"
	else
		ok "make builds libft.a at the root"
		w=$(echo "$out" | grep -i 'warning' | head -3)
		[ -z "$w" ] && ok "no compiler warnings" || ko "no compiler warnings" "$w"
		out2=$(make -C "$DIR" 2>&1)
		if echo "$out2" | grep -qE '(^|[[:space:]/])(cc|gcc|clang|ar)[[:space:]]'; then
			ko "second 'make' does nothing (no relink)" "$(echo "$out2" | grep -E '(cc|ar) ' | head -2)"
		else
			ok "second 'make' does nothing (no relink)"
		fi
		one=$(cd "$DIR" && ls ft_*.c | head -1)
		sleep 1; touch "$DIR/$one"
		out3=$(make -C "$DIR" 2>&1)
		n=$(echo "$out3" | grep -cE '(^|[[:space:]/])(cc|gcc|clang)[[:space:]].*-c')
		[ "$n" -le 1 ] && ok "touching one .c recompiles only that file" \
			|| ko "touching one .c recompiles only that file" "$n files were recompiled"
		make -C "$DIR" clean >/dev/null 2>&1
		if ls "$DIR"/*.o >/dev/null 2>&1; then ko "make clean removes the .o files"; else ok "make clean removes the .o files"; fi
		[ -f "$DIR/libft.a" ] && ok "make clean keeps libft.a" || warn "make clean keeps libft.a"
		out4=$(make -C "$DIR" 2>&1)
		echo "$out4" | grep -qE '(^|[[:space:]/])ar[[:space:]]' && [ -f "$DIR/libft.a" ] \
			&& ok "make after clean rebuilds" || warn "make after clean rebuilds" "check your dependencies on .o files"
		make -C "$DIR" fclean >/dev/null 2>&1
		[ -f "$DIR/libft.a" ] && ko "make fclean removes libft.a" || ok "make fclean removes libft.a"
		make -C "$DIR" re >/dev/null 2>&1
		[ -f "$DIR/libft.a" ] && ok "make re" || ko "make re"
	fi
fi

# ---------------------------------------------------------- libft.a symbols
LIB="$DIR/libft.a"
if [ -f "$LIB" ] && command -v nm >/dev/null; then
	ar t "$LIB" 2>/dev/null | grep -q '\.o$' && ok "libft.a is an ar archive of .o files" || ko "libft.a is an ar archive of .o files"
	defined=$(nm --defined-only "$LIB" 2>/dev/null | awk 'NF==3{print $3}' | sort -u)
	exported=$(nm --defined-only -g "$LIB" 2>/dev/null | awk 'NF==3 && $2 ~ /[TW]/ {print $3}' | sort -u)
	missing=""
	for f in $REQUIRED; do echo "$exported" | grep -qx "$f" || missing="$missing $f"; done
	[ -z "$missing" ] && ok "all 43 functions are in libft.a" || ko "all 43 functions are in libft.a" "missing:$missing"
	undef=$(nm -u "$LIB" 2>/dev/null | awk '{print $2}' | sort -u)
	forbidden=""
	for s in $undef; do
		echo "$defined" | grep -qx "$s" && continue
		echo " $ALLOWED_EXT " | grep -q " $s " && continue
		forbidden="$forbidden $s"
	done
	[ -z "$forbidden" ] && ok "external functions: only malloc, free, write" \
		|| ko "external functions: only malloc, free, write" "forbidden:$forbidden"
	globals=$(nm "$LIB" 2>/dev/null | awk 'NF==3 && $2 ~ /^[BbDdCcGgSsVv]$/ {print $3}' | sort -u | tr '\n' ' ')
	[ -z "$globals" ] && ok "no global (or static global) variables" \
		|| ko "no global (or static global) variables" "found: $globals"
	rodata=$(nm "$LIB" 2>/dev/null | awk 'NF==3 && $2 ~ /^[Rr]$/ && $3 !~ /^\./ && $3 !~ /^__func__/ {print $3}' | sort -u | tr '\n' ' ')
	[ -z "$rodata" ] && ok "no const global tables" || warn "no const global tables" "found: $rodata"
	helpers=""
	for s in $exported; do echo "$REQUIRED" | grep -qw "$s" || helpers="$helpers $s"; done
	[ -z "$helpers" ] && ok "helper functions are static" \
		|| warn "helper functions are static" "not static:$helpers (subject: define helpers as static)"
fi

# ------------------------------------------------------------ norminette
if command -v norminette >/dev/null; then
	nout=$(cd "$DIR" && norminette *.c libft.h 2>&1 | grep -v ': OK!$')
	[ -z "$nout" ] && ok "norminette" || ko "norminette" "$(echo "$nout" | head -8)"
else
	warn "norminette" "norminette is not installed here, run it yourself"
fi

# ---------------------------------------------------------------- README
R="$DIR/README.md"
if [ -f "$R" ]; then
	first=$(head -n 1 "$R")
	if echo "$first" | grep -qE '^[*_].*This project has been created as part of the 42 curriculum by [^*_]+[*_][[:space:]]*$'; then
		ok "README: first line in italics with the logins"
	else
		ko "README: first line in italics with the logins" \
			"expected: *This project has been created as part of the 42 curriculum by <login>.*  got: $first"
	fi
	for s in Description Instructions Resources; do
		grep -qiE "^#+[[:space:]]*$s" "$R" && ok "README: \"$s\" section" || ko "README: \"$s\" section" "missing"
	done
	grep -qiE '(^|[^a-z])(AI|IA|chatgpt|claude|copilot|artificial)([^a-z]|$)' "$R" \
		&& ok "README: explains how AI was used" || ko "README: explains how AI was used" "required in Resources"
fi

why=""
[ "$TRACE" != /dev/null ] && [ $((KO + WARN)) -gt 0 ] && why=" - why: $TRACE"
printf "\n  ${DIM}subject checks: %d KO, %d WARN%s${RST}\n" "$KO" "$WARN" "$why"
exit 0
