MAKEFLAGS += --no-print-directory
NAME = ft_vox

all:
	@cmake -B build
	@make -C build -j $(MAKEFLAGS)
	@mv build/$(NAME) .

clean:
	@rm -rf build

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re