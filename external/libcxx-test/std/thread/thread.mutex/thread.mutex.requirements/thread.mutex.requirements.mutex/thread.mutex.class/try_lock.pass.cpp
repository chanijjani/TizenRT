//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads

// <mutex>

// class mutex;

// bool try_lock();

#include <mutex>
#include <thread>
#include <cstdlib>
#include <cassert>
#include "test_macros.h"
#include "libcxx_tc_common.h"

static std::mutex m;

typedef std::chrono::system_clock Clock;
typedef Clock::time_point time_point;
typedef Clock::duration duration;
typedef std::chrono::milliseconds ms;
typedef std::chrono::nanoseconds ns;

static int f()
{
    time_point t0 = Clock::now();
    TC_ASSERT_EXPR(!m.try_lock());
    TC_ASSERT_EXPR(!m.try_lock());
    TC_ASSERT_EXPR(!m.try_lock());
    while(!m.try_lock())
        ;
    time_point t1 = Clock::now();
    m.unlock();
    ns d = t1 - t0 - ms(250);
    TC_ASSERT_EXPR(d < ms(400));  // within 200ms
    return 0;
}

int tc_libcxx_thread_thread_mutex_class_try_lock(void)
{
    m.lock();
    std::thread t(f);
    std::this_thread::sleep_for(ms(250));
    m.unlock();
    t.join();
    TC_SUCCESS_RESULT();
    return 0;
}
