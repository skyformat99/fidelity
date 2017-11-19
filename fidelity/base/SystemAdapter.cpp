#include "SystemAdapter.hpp"

#include <sys/mman.h>
#include <sys/resource.h>

#include <thread>

namespace fdl {

pthread_t PthreadAdapter::pthread_self() {
  return ::pthread_self();
}

int PthreadAdapter::pthread_attr_init(pthread_attr_t* attr) {
  return ::pthread_attr_init(attr);
}

int PthreadAdapter::pthread_attr_destroy(pthread_attr_t* attr) {
  return ::pthread_attr_destroy(attr);
}

int PthreadAdapter::pthread_attr_setstacksize(pthread_attr_t* attr, size_t stacksize) {
  return ::pthread_attr_setstacksize(attr, stacksize);
}

int PthreadAdapter::pthread_attr_setschedpolicy(pthread_attr_t* attr, int policy) {
  return ::pthread_attr_setschedpolicy(attr, policy);
}

int PthreadAdapter::pthread_attr_setschedparam(pthread_attr_t* attr, const sched_param* param) {
  return ::pthread_attr_setschedparam(attr, param);
}

int PthreadAdapter::pthread_attr_setaffinity_np(pthread_attr_t* attr, size_t cpusetsize,
                                                const cpu_set_t* cpuset) {
  return ::pthread_attr_setaffinity_np(attr, cpusetsize, cpuset);
}

int PthreadAdapter::pthread_attr_setinheritsched(pthread_attr_t* attr, int inheritsched) {
  return ::pthread_attr_setinheritsched(attr, inheritsched);
}

int PthreadAdapter::pthread_create(pthread_t* thread, const pthread_attr_t* attr,
                                   void* (*start_routine)(void*), void* arg) {
  return ::pthread_create(thread, attr, start_routine, arg);
}

int PthreadAdapter::pthread_setname_np(pthread_t thread, const char* name) {
  return ::pthread_setname_np(thread, name);
}

int PthreadAdapter::pthread_cancel(pthread_t thread) {
  return ::pthread_cancel(thread);
}

int PthreadAdapter::pthread_join(pthread_t thread, void** retval) {
  return ::pthread_join(thread, retval);
}

int ResourceAdapter::getrlimit(int resource, rlimit* rlp) {
  return ::getrlimit(resource, rlp);
}

int MManAdapter::mlockall(int flags) {
  return ::mlockall(flags);
}

unsigned int ThreadAdapter::hardware_concurrency() {
  return std::thread::hardware_concurrency();
}

}  // namespace fdl
