// Testing file. Becomes executable if DEBUG
#include <iostream>
#include <tokenizer.h>
#include <core_exceptions.h>
#include <llvm_value.h>
// Думаем над USL

int main()
{
#ifdef DEBUG
	
#else
	puts("This executable is preserved for any tests. Avaliable only on DEBUG mode.");
#endif
	return 0;
}