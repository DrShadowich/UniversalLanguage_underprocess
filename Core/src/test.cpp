// Testing file. Becomes executable if DEBUG
#include <iostream>
#include <tokenizer.h>
#include <core_exceptions.h>
#include <llvm_value.h>
int main()
{
#ifdef DEBUG
	printf_s("%d\n", true);
	printf_s("%d\n", false);
#else
	puts("This executable is preserved for any tests. Avaliable only on DEBUG mode.");
#endif
	return 0;
}