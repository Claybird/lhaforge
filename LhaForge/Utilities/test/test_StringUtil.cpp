#include "stdafx.h"
#include "CppUnitTest.h"
#include "Utilities/StringUtil.h"



using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace UnitTest
{
	TEST_CLASS(StringUtil)
	{
	public:
		TEST_METHOD(test_Format) {
			Assert::AreEqual(std::wstring(L""), Format(L""));
			Assert::AreEqual(std::wstring(L"123"), Format(L"%d", 123));
			Assert::AreEqual(std::wstring(L"abc"), Format(L"%s", L"abc"));
		}
	};
};
