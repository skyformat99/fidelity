#pragma once

#include <gmock/gmock.h>

#include <memory>

#include "../SystemAdapter.hpp"

namespace fdl::test {

struct PthreadAdapterMock : public IPthreadAdapter {
  MOCK_METHOD0(pthread_self, pthread_t());
  MOCK_METHOD1(pthread_attr_init, int(pthread_attr_t*));
  MOCK_METHOD1(pthread_attr_destroy, int(pthread_attr_t*));
  MOCK_METHOD2(pthread_attr_setstacksize, int(pthread_attr_t*, size_t));
  MOCK_METHOD2(pthread_attr_setschedpolicy, int(pthread_attr_t*, int));
  MOCK_METHOD2(pthread_attr_setschedparam, int(pthread_attr_t*, const struct sched_param*));
  MOCK_METHOD3(pthread_attr_setaffinity_np, int(pthread_attr_t*, size_t, const cpu_set_t*));
  MOCK_METHOD2(pthread_attr_setinheritsched, int(pthread_attr_t*, int));
  MOCK_METHOD4(pthread_create, int(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*));
  MOCK_METHOD2(pthread_setname_np, int(pthread_t, const char*));
  MOCK_METHOD1(pthread_cancel, int(pthread_t));
  MOCK_METHOD2(pthread_join, int(pthread_t, void**));
};

struct ResourceAdapterMock : public IResourceAdapter {
  MOCK_METHOD2(getrlimit, int(int, struct rlimit*));
};

struct MManAdapterMock : public IMManAdapter {
  MOCK_METHOD1(mlockall, int(int flags));
};

struct ThreadAdapterMock : public IThreadAdapter {
  MOCK_METHOD0(hardware_concurrency, unsigned int());
};

struct SystemAdapterMock : public SystemAdapter {
  SystemAdapterMock() {
    pthread = std::make_shared<PthreadAdapterMock>();
    resource = std::make_shared<ResourceAdapterMock>();
    mman = std::make_shared<MManAdapterMock>();
    thread = std::make_shared<ThreadAdapterMock>();
  }

  PthreadAdapterMock& pthreadMock() {
    return *std::dynamic_pointer_cast<PthreadAdapterMock>(pthread);
  }

  ResourceAdapterMock& resourceMock() {
    return *std::dynamic_pointer_cast<ResourceAdapterMock>(resource);
  }

  MManAdapterMock& mmanMock() {
    return *std::dynamic_pointer_cast<MManAdapterMock>(mman);
  }

  ThreadAdapterMock& threadMock() {
    return *std::dynamic_pointer_cast<ThreadAdapterMock>(thread);
  }
};

}  // namespace fdl::test
