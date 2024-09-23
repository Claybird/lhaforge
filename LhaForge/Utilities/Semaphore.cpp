#include "stdafx.h"
#ifdef UNIT_TEST
#include "Utilities/Semaphore.h"

TEST(semaphore, semaphore) {
	CSemaphoreLocker lock1;
	CSemaphoreLocker lock2;
	CSemaphoreLocker lock3;
	EXPECT_TRUE(lock1.Create(L"test_semaphore", 2));
	EXPECT_TRUE(lock1.Lock(INFINITE));
	EXPECT_TRUE(lock2.Create(L"test_semaphore", 2));
	EXPECT_TRUE(lock2.Lock(INFINITE));
	EXPECT_TRUE(lock3.Create(L"test_semaphore", 2));
	EXPECT_FALSE(lock3.Lock(10));
	lock1.Release();
	EXPECT_TRUE(lock3.Lock(10));
}

#endif
