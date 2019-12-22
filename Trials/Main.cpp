#include <iostream>
#include "Main.hpp"

void TestTheTester(int &a, int b)
{
	if (b < a)
	{
		a = b;
	}
}

int main()
{
	using namespace std;

	int a = 5, b = 6;

	TestTheTester(a, b);
	std::cout << "Testing the tester! " << a << endl;

	return 0;
}