// clang-format off

// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4075.pdf
// https://github.com/bloomberg/bde/blob/master/groups/bsl/bsls/bsls_assert.h

#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>

// macros

#undef STD_CONTRACT_ASSERT_OPT_IS_ACTIVE
#undef STD_CONTRACT_ASSERT_DBG_IS_ACTIVE
#undef STD_CONTRACT_ASSERT_SAFE_IS_ACTIVE
#undef STD_CONTRACT_ASSERT_IS_ACTIVE

#undef STD_CONTRACT_ASSERT_OPT
#undef STD_CONTRACT_ASSERT_DBG
#undef STD_CONTRACT_ASSERT_SAFE
#undef STD_CONTRACT_ASSERT

#if !defined(STD_CONTRACT_ASSERT_LEVEL_NONE) &&              \
    !defined(STD_CONTRACT_ASSERT_LEVEL_ASSERT_OPT) &&        \
    !defined(STD_CONTRACT_ASSERT_LEVEL_ASSERT_DBG) &&        \
    !defined(STD_CONTRACT_ASSERT_LEVEL_ASSERT_SAFE)
#define STD_CONTRACT_ASSERT_LEVEL_ASSERT_DBG
#elif 1 != \
    defined(STD_CONTRACT_ASSERT_LEVEL_NONE) +                \
    defined(STD_CONTRACT_ASSERT_LEVEL_ASSERT_OPT) +          \
    defined(STD_CONTRACT_ASSERT_LEVEL_ASSERT_DBG) +          \
    defined(STD_CONTRACT_ASSERT_LEVEL_ASSERT_SAFE)
#error "Cannot define more than one STD_CONTRACT_ASSERT_LEVEL_* macro at one time"
#endif

#ifdef STD_CONTRACT_ASSERT_LEVEL_ASSERT_OPT

#define STD_CONTRACT_ASSERT_OPT_IS_ACTIVE

#endif // STD_CONTRACT_ASSERT_LEVEL_ASSERT_OPT

#ifdef STD_CONTRACT_ASSERT_LEVEL_ASSERT_DBG

#define STD_CONTRACT_ASSERT_OPT_IS_ACTIVE
#define STD_CONTRACT_ASSERT_DBG_IS_ACTIVE
#define STD_CONTRACT_ASSERT_IS_ACTIVE

#endif // STD_CONTRACT_ASSERT_LEVEL_ASSERT_DBG

#ifdef STD_CONTRACT_ASSERT_LEVEL_ASSERT_SAFE

#define STD_CONTRACT_ASSERT_OPT_IS_ACTIVE
#define STD_CONTRACT_ASSERT_DBG_IS_ACTIVE
#define STD_CONTRACT_ASSERT_SAFE_IS_ACTIVE
#define STD_CONTRACT_ASSERT_IS_ACTIVE

#endif // STD_CONTRACT_ASSERT_LEVEL_ASSERT_SAFE

#ifdef STD_CONTRACT_ASSERT_OPT_IS_ACTIVE

#define STD_CONTRACT_ASSERT_OPT(...)                         \
  do                                                         \
  {                                                          \
    if(!(__VA_ARGS__))                                       \
    {                                                        \
      std::experimental::handle_contract_violation({         \
          std::experimental::contract_assert_mode::opt,      \
          #__VA_ARGS__,                                      \
          __FILE__,                                          \
          __LINE__                                           \
      });                                                    \
    }                                                        \
  } while (0)

#else

#define STD_CONTRACT_ASSERT_OPT(...) do {} while (0)

#endif  // STD_CONTRACT_ASSERT_OPT_IS_ACTIVE

#ifdef STD_CONTRACT_ASSERT_DBG_IS_ACTIVE

#define STD_CONTRACT_ASSERT_DBG(...)                         \
  do                                                         \
  {                                                          \
    if(!(__VA_ARGS__))                                       \
    {                                                        \
      std::experimental::handle_contract_violation({         \
          std::experimental::contract_assert_mode::opt,      \
          #__VA_ARGS__,                                      \
          __FILE__,                                          \
          __LINE__                                           \
      });                                                    \
    }                                                        \
  } while (0)

#else

#define STD_CONTRACT_ASSERT_DBG(...) do {} while(0)

#endif // STD_CONTRACT_ASSERT_DBG_IS_ACTIVE

#ifdef STD_CONTRACT_ASSERT_SAFE_IS_ACTIVE

#define STD_CONTRACT_ASSERT_SAFE(...)                        \
  do                                                         \
  {                                                          \
    if (!(__VA_ARGS__))                                      \
    {                                                        \
      std::experimental::handle_contract_violation({         \
          std::experimental::contract_assert_mode::safe,     \
          #__VA_ARGS__,                                      \
          __FILE__,                                          \
          __LINE__                                           \
      });                                                    \
    }                                                        \
  } while (0)

#else

#define STD_CONTRACT_ASSERT_SAFE(...) do {} while(0)

#endif // STD_CONTRACT_ASSERT_SAFE_IS_ACTIVE

#define STD_CONTRACT_ASSERT(...) STD_CONTRACT_ASSERT_DBG(__VA_ARGS__)

#define CONTRACT(COND) STD_CONTRACT_ASSERT_DBG(COND)
#define CONTRACT_MSG(COND, MSG) STD_CONTRACT_ASSERT_DBG(COND && MSG)

#define CONTRACT_MACRO(_1, _2, NAME, ...) NAME
#define EXPECT(...) CONTRACT_MACRO(__VA_ARGS__, CONTRACT_MSG, CONTRACT, X)(__VA_ARGS__)
#define ENSURE(...) CONTRACT_MACRO(__VA_ARGS__, CONTRACT_MSG, CONTRACT, X)(__VA_ARGS__)

#ifndef INCLUDED_CONTRACT_ASSERT
#define INCLUDED_CONTRACT_ASSERT

namespace std {
namespace experimental {
inline namespace fundamentals_v2 {

// types
enum class contract_assert_mode
{
  opt,
  dbg,
  safe
};

struct contract_violation_info
{
  contract_assert_mode mode;
  const char *expression_text;
  const char *filename;
  size_t line_number;
};

struct contract_violation_error : public std::exception {
  contract_violation_error(const contract_violation_info& info) : info(info) {
    const char *path = info.filename;
    const char *file = path;
    const char *search = path;
    while ((search = std::strstr(file, "/")) != nullptr) {
      file = ++search;
    }
    int ret = std::snprintf(message_buffer, sizeof(message_buffer), "%s asserted in %s:%u",
                            info.expression_text, file, info.line_number);
    if (ret < 0) {
      message_buffer[0] = 0;
    }
  }

  contract_violation_info info;
  char message_buffer[256];

  const char* what() const throw() override {
    if (message_buffer[0] > 0) {
      return message_buffer;
    } else {
      return info.expression_text;
    }
  }
};

using handle_contract_violation_handler = void (*)(const contract_violation_info&);

// handler manipulators
handle_contract_violation_handler
set_handle_contract_violation(handle_contract_violation_handler handler) noexcept;

handle_contract_violation_handler get_handle_contract_violation() noexcept;

// handler invocation
[[noreturn]] void handle_contract_violation(const contract_violation_info& info);

// local precondition violation handler installation
class handle_contract_violation_guard
{
public:
  handle_contract_violation_guard(const handle_contract_violation_guard&) = delete;

  handle_contract_violation_guard&
  operator=(const handle_contract_violation_guard&) = delete;

  handle_contract_violation_guard(handle_contract_violation_guard&&) = delete;

  handle_contract_violation_guard&
  operator=( handle_contract_violation_guard&&) = delete;

  explicit
  handle_contract_violation_guard(handle_contract_violation_handler handler);

  ~handle_contract_violation_guard();

private:
  handle_contract_violation_handler old_handler;
};

} // namespace fundamentals_v2
} // namespace experimental
} // namespace std

#endif // INCLUDED_CONTRACT_ASSERT
