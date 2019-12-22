#include "pch.h"

#include "CppUnitTest.h"
#include "../Trials/Main.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TrialsUnitTest
{

	TEST_CLASS(TrialsUnitTest)
	{
	public:
	
		TEST_METHOD(TestMethod1)
		{
			int a = 5, b = 6;

			TestTheTester(a, b);
			Assert::IsTrue(a==6);
		}
	};
}
