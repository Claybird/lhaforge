#include "stdafx.h"
#ifdef UNIT_TEST
#include "CppUnitTest.h"
#include "Utilities/Semaphore.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(semaphore)
	{
	public:
		TEST_METHOD(test_semaphore) {
			CSemaphoreLocker lock1;
			CSemaphoreLocker lock2;
			CSemaphoreLocker lock3;
			Assert::IsTrue(lock1.Lock(L"test_semaphore", 2));
			Assert::IsTrue(lock2.Lock(L"test_semaphore", 2));
			//Assert::IsFalse(lock3.Lock(L"test_semaphore", 2));	will wait forever
			lock1.Release();
			Assert::IsTrue(lock3.Lock(L"test_semaphore", 2));
		}
	};
};

#endif
