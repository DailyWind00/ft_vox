MAKEFLAGS += --no-print-directory
NAME = ft_vox

all:
	@chmod +x fetch-dependencies.sh
	@./fetch-dependencies.sh
	@cmake -B build
	@make -C build -j $(MAKEFLAGS)
	@mv build/$(NAME) .
	@echo "\033[1;32m> Start playing with : \033[1;37m./$(NAME) \033[1;38;5;250m[\033[1;33mflags\033[1;37m\033[1;38;5;250m] \033[1;38;5;250m[\033[1;33mseed\033[1;37m\033[1;38;5;250m]\033[0m"

clean:
	@rm -rf build
	@rm -f  *.logs

fclean: clean
	@rm -f $(NAME)

wipe: fclean
	@rm -rf dependencies

re: fclean all

.PHONY: all clean fclean wipe re