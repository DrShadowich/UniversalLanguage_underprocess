// Testing file. Becomes executable if DEBUG
#include <iostream>
#include <tokenizer.h>
#include <core_exceptions.h>
int main()
{
#ifdef _DEBUG
#else
	puts("This executable is preserved for any tests. Avaliable only on DEBUG mode.");
#endif
	return 0;
}