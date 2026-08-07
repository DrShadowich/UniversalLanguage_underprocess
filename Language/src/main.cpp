#include <iostream>
#include <core_exceptions.h>
#include <commander.h>

int main(int argc, char** argv)
{
	try
	{
		cmd::commander cmd_breach{ argc, argv };
		cmd_breach.execute_commands();
	}catch(ul::ex::commander_exception& ex)
	{
		std::cout << ex.what() << std::endl;
		return -1;
	}
	return 0;
}