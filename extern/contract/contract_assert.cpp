// clang-format off

#include "contract_assert.hpp"
#include <atomic>

namespace std {
namespace experimental {
inline namespace fundamentals_v2 {

namespace detail {

void abort_handler(const contract_violation_info&)
{
  std::abort();
}

std::atomic<handle_contract_violation_handler> handler{abort_handler};

thread_local
handle_contract_violation_handler installed_local_handler{nullptr};

} // namespace detail

handle_contract_violation_handler
set_handle_contract_violation(handle_contract_violation_handler handler) noexcept
{
  if (handler)
  {
    return std::atomic_exchange(&detail::handler, handler);
  }
  else
  {
    return std::atomic_exchange(&detail::handler, detail::abort_handler);
  }
}

handle_contract_violation_handler get_handle_contract_violation() noexcept
{
  return std::atomic_load(&detail::handler);
}

[[noreturn]] void handle_contract_violation(const contract_violation_info& info)
{
  [[noreturn]]
  handle_contract_violation_handler handler = detail::installed_local_handler;
  if (handler)
  {
    handler(info);
  }
  handler = get_handle_contract_violation();
  if (handler)
  {
    handler(info);
  }
  std::abort();
}

handle_contract_violation_guard::handle_contract_violation_guard(
    handle_contract_violation_handler handler)
: old_handler(detail::installed_local_handler)
{
  detail::installed_local_handler = handler;
}

handle_contract_violation_guard::~handle_contract_violation_guard()
{
  detail::installed_local_handler = old_handler;
}

} // namespace fundamentals_v2
} // namespace experimental
} // namespace std
