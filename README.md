# **Libft Tester**

This repository contains a **C program** designed to **automatically test your `libft` library**. It provides a comprehensive testing suite to ensure your `libft` works as expected under various conditions.

## **Features**
- Test your `libft` functions with different input cases.
- Prints detailed test results, including any discrepancies between your program's output and the expected output.
- The final message will notify you if the tests have passed or if there are errors in your implementation.

### **Tested Scenarios**
- Basic string operations (e.g., `ft_strlen`, `ft_strchr`, `ft_strcat`, etc.).
- Memory operations (e.g., `ft_memcpy`, `ft_memmove`, `ft_calloc`).
- Integer operations (e.g., `ft_itoa`, `ft_atoi`).
- Linked list operations (e.g., `ft_lstnew`, `ft_lstadd_back`, `ft_lstclear`) (BONUS).

Additionally, it runs tests for:
- **Edge cases** (e.g., NULL pointers, empty strings).
- **Compilation errors** and checks for expected behavior.

## **Prerequisites**
- **gcc** (GNU Compiler Collection) for compiling C code.

## **Setup**
1. Clone this repository into your libft repository:
   ```bash
   git clone https://github.com/yourusername/gnl-tester.git
   cd gnl-tester
   ```

## **Usage**

### Run the Tests
The script will automatically compile the necessary files, run the tests, and check for any errors or memory leaks. To run the tests, simply use the following command:

```bash
make
make bonus
```
This will:

1. Compile the libft library (from the parent directory).
2. Build the tester program.
3. Run all the tests on your library.
4. Clean the solution

### Example Output
If all tests pass, you'll see:
```bash
🎉 --- CONGRATULATIONS!! YOU PASSED ALL THE TESTS! MAY THE FORCE BE WITH YOU --- 🎉
```

If there are errors or memory leaks, you'll see:
```bash
😢 --- SADLY YOU DID NOT PASS THE TESTS. YOU HAVE [x] ERRORS. BETTER LUCK NEXT TIME --- 😢
```

## **Tested Functions**
This tester covers the following libft functions:
- Memory management: `ft_memset`, `ft_bzero`, `ft_memcpy`, `ft_memmove`, `ft_calloc`, `ft_strdup`.
- String manipulation: `ft_strlen`, `ft_strchr`, `ft_strrchr`, `ft_strncmp`, `ft_strlcat`, `ft_strlcpy`, `ft_strnstr`, `ft_strcmp`, `ft_strjoin`, `ft_substr`, `ft_strtrim`, `ft_split`, `ft_itoa`, `ft_atoi`, `ft_strmapi`.
- Character tests: `ft_isalpha`, `ft_isdigit`, `ft_isalnum`, `ft_isascii`, `ft_isprint`, `ft_toupper`, `ft_tolower`.
- Linked list operations (bonus): `ft_lstnew`, `ft_lstadd_front`, `ft_lstadd_back`, `ft_lstlast`, `ft_lstclear`,

## **How the Tests Work**

1. The script compiles the source code with the specified flags.
2. It runs a series of tests on your `libft library`, checking the results for various functions.
3. It compares the output of each function against expected results.
4. The script prints the results and shows the overall status of your tests.

## **Final Message**
- If no errors are detected and all tests pass, you will see:
  `🎉 --- CONGRATULATIONS!! YOU PASSED ALL THE TESTS! MAY THE FORCE BE WITH YOU --- 🎉`
  
- If there are errors, the final message will be:  
  `😢 --- SADLY YOU DID NOT PASS THE TESTS. YOU HAVE [x] ERRORS. BETTER LUCK NEXT TIME --- 😢`
