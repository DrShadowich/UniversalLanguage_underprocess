#include <iostream>
#include <core_exceptions.h>
#include <commander.h>

int main(int argc, char** argv)
{
	char** i = new char* [] { const_cast<char*>("."), const_cast<char*>("main.ul"), const_cast<char*>("-o"), const_cast<char*>("main.exe"), };
	int in = 4;
	try
	{
		cmd::commander cmd_breach{ in, i };
		cmd_breach.execute_commands();
	}catch(ul::ex::commander_exception& ex)
	{
		std::cout << ex.what() << std::endl;
		return -1;
	}
	return 0;
}