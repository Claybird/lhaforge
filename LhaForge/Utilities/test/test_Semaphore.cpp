#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "Utilities/Semaphore.h"

TEST(semaphore, semaphore) {
	CSemaphoreLocker lock1;
	CSemaphoreLocker lock2;
	CSemaphoreLocker lock3;
	EXPECT_TRUE(lock1.Lock(L"test_semaphore", 2));
	EXPECT_TRUE(lock2.Lock(L"test_semaphore", 2));
	//EXPECT_FALSE(lock3.Lock(L"test_semaphore", 2));	will wait forever
	lock1.Release();
	EXPECT_TRUE(lock3.Lock(L"test_semaphore", 2));
}

#endif
