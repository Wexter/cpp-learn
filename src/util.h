#include <iostream>
#include <string_view>

#define PRINT(msg) std::cout << msg

#define PRINTLN(msg) PRINT(msg) << std::endl

#define LOG_INFO(msg) PRINTLN("INFO: " << msg)

#ifdef DEBUG
#define LOG_DEBUG(msg) PRINTLN("DEBUG: " << __FILE__ << ":" << __LINE__ << " " << msg)
#else
#define LOG_DEBUG(msg)
#endif

#define LOG_DEBUG_IF(expr, msg) if (expr) { LOG_DEBUG(msg); }

template <typename T>
constexpr auto get_type_name() {
  std::string_view name, prefix, suffix;
#ifdef __clang__
  name = __PRETTY_FUNCTION__;
  prefix = "auto type_name() [T = ";
  suffix = "]";
#elif defined(__GNUC__)
  name = __PRETTY_FUNCTION__;
  prefix = "constexpr auto type_name() [with T = ";
  suffix = "]";
#elif defined(_MSC_VER)
  name = __FUNCSIG__;
  prefix = "auto __cdecl type_name<";
  suffix = ">(void)";
#endif
  name.remove_prefix(prefix.size());
  name.remove_suffix(suffix.size());
  return name.data();
}