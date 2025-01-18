# pragma once

/// Defines

/// System includes
# include <string>
# include <list>

/// Dependencies

/// Global variables

struct	Flag {
	std::string	name;
	std::string	shortcut;
	void	(*f)(void *);
};

// that class allows to set multiple 
class	FlagHandler  {
	private:
		std::list<struct Flag>	_flags;
	
	public:
		FlagHandler();
		~FlagHandler();

		void	setFlag(const std::string &name, const std::string &shortcut, void (f)(void *));
		int	execFlagsFunction(const std::string &identifier, void *arg);

		size_t	getFlagNum() const;
};
