MAKEFLAGS += --no-print-directory
NAME = ft_vox
VALGRIND_ARGS =  # Default empty value
VALGRIND_OUTFILE = valgrind.log

all: release

release:
	@chmod +x fetch-dependencies.sh
	@./fetch-dependencies.sh
	@cmake -B build -DCMAKE_BUILD_TYPE=Release
	@make -C build -j $(MAKEFLAGS)
	@mv build/$(NAME) .
	@echo "\033[1;34m> Start playing with : \033[1;37m./$(NAME) \033[1;38;5;250m[\033[1;33mflags\033[1;37m\033[1;38;5;250m] \033[1;38;5;250m[\033[1;33mseed\033[1;37m\033[1;38;5;250m]\033[0m"

debug:
	@chmod +x fetch-dependencies.sh
	@./fetch-dependencies.sh
	@cmake -B build -DCMAKE_BUILD_TYPE=Debug
	@make -C build -j $(MAKEFLAGS)
	@mv build/$(NAME) .
	@echo "\033[1;34m> Debug binary ready: \033[1;37m./$(NAME) \033[1;38;5;250m[\033[1;33mflags\033[1;37m\033[1;38;5;250m] \033[1;38;5;250m[\033[1;33mseed\033[1;37m\033[1;38;5;250m]\033[0m"

clean:
	@rm -rf build
	@rm -f  *.log

fclean: clean
	@rm -f $(NAME)

wipe: fclean
	@rm -rf dependencies

re: fclean all

valgrind:
	@if [ -f $(NAME) ]; then \
		valgrind --suppressions=suppresion.supp ./$(NAME) $(VALGRIND_ARGS) 2>&1 | tee $(VALGRIND_OUTFILE); \
		echo "\033[1;34m> Valgrind log saved to: \033[1;37m$(VALGRIND_OUTFILE)\033[0m"; \
	else \
		echo "\033[1;31mError: Executable not found. Please build first.\033[0m"; \
	fi

.PHONY: all release debug clean fclean wipe re valgrind