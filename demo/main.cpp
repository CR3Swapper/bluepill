#include <iostream>
#include "hypercall.h"

int main()
{
	std::printf("hypercall result: 0x%x\n", bluepill::hypercall(bluepill::key));
	std::getchar();
}