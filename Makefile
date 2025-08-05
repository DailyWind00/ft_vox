MAKEFLAGS += --no-print-directory
NAME = ft_vox
VALGRIND_ARGS =  # Default empty value
VALGRIND_OUTFILE = valgrind.log

# Output color definition
EC_DEFAULT = \033[0m
EC_RED = \033[1;31m
EC_BLUE = \033[1;34m
EC_WHITE = \033[1;37m
EC_ORANGE = \033[1;33m
EC_GRAY1 = \033[1;38;5;250m

all: release

release:
	@chmod +x fetch-dependencies.sh
	@./fetch-dependencies.sh
	@cmake -B build -DCMAKE_BUILD_TYPE=Release
	@make -C build -j $(MAKEFLAGS)
	@mv build/$(NAME) .
	@echo -e "$(EC_BLUE)> Start playing with : $(EC_WHITE)./$(NAME) $(EC_GRAY1)[$(EC_ORANGE)flags$(EC_GRAY1)] [$(EC_ORANGE)seed$(EC_GRAY1)]$(EC_DEFAULT)"

debug:
	@chmod +x fetch-dependencies.sh
	@./fetch-dependencies.sh
	@cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++
	@make -C build -j $(MAKEFLAGS)
	@mv build/$(NAME) .
	@echo -e "$(EC_BLUE)> Debug binary ready: $(EC_WHITE)./$(NAME) $(EC_GRAY1)[$(EC_ORANGE)flags$(EC_GRAY1)] [$(EC_ORANGE)seed$(EC_GRAY1)]$(EC_DEFAULT)"

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
		echo -e "$(EC_BLUE)> Valgrind log saved to: $(EC_WHITE)$(VALGRIND_OUTFILE)$(EC_DEFAULT)"; \
	else \
		echo -e "$(EC_RED)Error: Executable not found. Please build first.$(EC_DEFAULT)"; \
	fi

.PHONY: all release debug clean fclean wipe re valgrind
