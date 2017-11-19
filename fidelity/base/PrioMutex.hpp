#pragma once

#include <contract/contract_assert.hpp>

#include <mutex>

namespace fdl {

/**
 * Mutex with PTHREAD_PRIO_INHERIT attribute to avoid priority inversion.
 * The thread which holds the look on this mutes will get the scheduling parameters of the high
 * priority thread which is waiting on the lock.
 * @see http://linux.die.net/man/3/pthread_mutexattr_setprotocol
 */
class PrioMutex : public std::mutex {
 public:
  PrioMutex() {
    pthread_mutexattr_t mutex_attr{};
    ENSURE(pthread_mutexattr_init(&mutex_attr) == 0);
    ENSURE(pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_INHERIT) == 0);
    ENSURE(pthread_mutex_init(native_handle(), &mutex_attr) == 0);
    ENSURE(pthread_mutexattr_destroy(&mutex_attr) == 0);
  }
};

}  // namespace fdl
