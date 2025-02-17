MAKEFLAGS += --no-print-directory
NAME = ft_vox

all:
	@chmod +x fetch-dependencies.sh
	@./fetch-dependencies.sh
	@cmake -B build
	@make -C build -j $(MAKEFLAGS)
	@mv build/$(NAME) .

clean:
	@rm -rf build
	@rm -f  *.logs

fclean: clean
	@rm -f $(NAME)

wipe: fclean
	@rm -rf dependencies

re: fclean all

.PHONY: all clean fclean wipe re