#pragma once

#include <pthread.h>
#include <sched.h>

#include <cstddef>
#include <memory>

struct rlimit;

namespace fdl {

// <pthread.h>
struct IPthreadAdapter {
  virtual ~IPthreadAdapter() = default;

  virtual pthread_t pthread_self() = 0;

  virtual int pthread_attr_init(pthread_attr_t* attr) = 0;

  virtual int pthread_attr_destroy(pthread_attr_t* attr) = 0;

  virtual int pthread_attr_setstacksize(pthread_attr_t* attr, size_t stacksize) = 0;

  virtual int pthread_attr_setschedpolicy(pthread_attr_t* attr, int policy) = 0;

  virtual int pthread_attr_setschedparam(pthread_attr_t* attr, const sched_param* param) = 0;

  virtual int pthread_attr_setaffinity_np(pthread_attr_t* attr, size_t cpusetsize,
                                          const cpu_set_t* cpuset) = 0;

  virtual int pthread_attr_setinheritsched(pthread_attr_t* attr, int inheritsched) = 0;

  virtual int pthread_create(pthread_t* thread, const pthread_attr_t* attr,
                             void* (*start_routine)(void*), void* arg) = 0;

  virtual int pthread_setname_np(pthread_t thread, const char* name) = 0;

  virtual int pthread_cancel(pthread_t thread) = 0;

  virtual int pthread_join(pthread_t thread, void** retval) = 0;
};

// <sys/resource.h>
struct IResourceAdapter {
  virtual ~IResourceAdapter() = default;

  virtual int getrlimit(int resource, rlimit* rlp) = 0;
};

// <sys/mman.h>
struct IMManAdapter {
  virtual ~IMManAdapter() = default;

  virtual int mlockall(int flags) = 0;
};

// <thread>
struct IThreadAdapter {
  virtual ~IThreadAdapter() = default;

  virtual unsigned int hardware_concurrency() = 0;
};

struct PthreadAdapter : public IPthreadAdapter {
  pthread_t pthread_self() override;

  int pthread_attr_init(pthread_attr_t* attr) override;

  int pthread_attr_destroy(pthread_attr_t* attr) override;

  int pthread_attr_setstacksize(pthread_attr_t* attr, size_t stacksize) override;

  int pthread_attr_setschedpolicy(pthread_attr_t* attr, int policy) override;

  int pthread_attr_setschedparam(pthread_attr_t* attr, const sched_param* param) override;

  int pthread_attr_setaffinity_np(pthread_attr_t* attr, size_t cpusetsize,
                                  const cpu_set_t* cpuset) override;

  int pthread_attr_setinheritsched(pthread_attr_t* attr, int inheritsched) override;

  int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*),
                     void* arg) override;

  int pthread_setname_np(pthread_t thread, const char* name) override;

  int pthread_cancel(pthread_t thread) override;

  int pthread_join(pthread_t thread, void** retval) override;
};

struct ResourceAdapter : public IResourceAdapter {
  int getrlimit(int resource, rlimit* rlp) override;
};

struct MManAdapter : public IMManAdapter {
  int mlockall(int flags) override;
};

struct ThreadAdapter : public IThreadAdapter {
  unsigned int hardware_concurrency() override;
};

struct SystemAdapter {
  SystemAdapter() {
    pthread = std::make_shared<PthreadAdapter>();
    resource = std::make_shared<ResourceAdapter>();
    mman = std::make_shared<MManAdapter>();
    thread = std::make_shared<ThreadAdapter>();
  }

  std::shared_ptr<IPthreadAdapter> pthread{};
  std::shared_ptr<IResourceAdapter> resource{};
  std::shared_ptr<IMManAdapter> mman{};
  std::shared_ptr<IThreadAdapter> thread{};
};

}  // namespace fdl
