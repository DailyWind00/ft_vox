# include "FlagHandler.hpp"

FlagHandler::FlagHandler() {}
FlagHandler::~FlagHandler() {}

void	FlagHandler::setFlag(const std::string &name, const std::string &shortcut, void (f)(void *))
{
	this->_flags.push_back(Flag{name, shortcut, f});
}


int	FlagHandler::execFlagsFunction(const std::string &identifier, void *arg)
{
	for (struct Flag  &flag : this->_flags) {
		if (flag.shortcut == identifier || flag.name == identifier) {
			if (!flag.f)
				return (-1);
			flag.f(arg);
			return (1);
		}
	}
	return (0);
}

size_t	FlagHandler::getFlagNum() const
{
	return (this->_flags.size());
}
