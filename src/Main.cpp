#include <iostream>

#include <Windows.h>
#include "lib/OlsApi.h"


int main() {
	if (InitializeOls()) 
	{
		std::cout << "yes" << std::endl;
	}
	else 
	{
		std::cout << "no" << std::endl;
	}
}