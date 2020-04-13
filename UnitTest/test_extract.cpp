#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(extract)
	{
	public:
		TEST_METHOD(test_LF_sanitize_pathname)
		{
			std::wstring LF_sanitize_pathname(const std::wstring rawPath);
			Assert::AreEqual(std::wstring(L""), LF_sanitize_pathname(L""));
		}
	};
}
