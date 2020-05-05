#include "stdafx.h"
#ifdef UNIT_TEST
#include "CppUnitTest.h"
#include "Utilities/FileOperation.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(file_operation)
	{
	public:
		TEST_METHOD(test_UtilGetTempPath) {
			Assert::IsTrue(std::filesystem::exists(UtilGetTempPath()));
		}
		TEST_METHOD(test_UtilGetTemporaryFileName) {
			Assert::IsTrue(std::filesystem::exists(UtilGetTemporaryFileName()));
		}
	};
};

#endif
