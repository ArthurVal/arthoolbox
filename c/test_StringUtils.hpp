extern "C" {
#include "String.h"
}

#include <ostream>
#include <string_view>

inline std::ostream &operator<<(std::ostream &os, const atb_StringView &view) {
  os << "atb_StringView{.data=" << static_cast<void *>(view.data);
  if (view.data != nullptr) {
    os << " -> \"" << std::string_view(view.data, view.size) << '\"';
  }
  os << ", .size=" << view.size << "}";

  return os;
}

constexpr bool operator==(const atb_StringView &lhs, std::string_view rhs) {
  return std::string_view(lhs.data, lhs.size) == rhs;
}

constexpr bool operator!=(const atb_StringView &lhs, std::string_view rhs) {
  return not(lhs == rhs);
}

constexpr bool operator==(std::string_view lhs, const atb_StringView &rhs) {
  return lhs == std::string_view(rhs.data, rhs.size);
}

constexpr bool operator!=(std::string_view lhs, const atb_StringView &rhs) {
  return not(lhs == rhs);
}

constexpr bool operator==(const atb_StringView &lhs,
                          const atb_StringView &rhs) {
  return std::string_view(lhs.data, lhs.size) ==
         std::string_view(rhs.data, rhs.size);
}

constexpr bool operator!=(const atb_StringView &lhs,
                          const atb_StringView &rhs) {
  return not(lhs == rhs);
}

inline std::ostream &operator<<(std::ostream &os,
                                const atb_ConstStringView &view) {
  os << "atb_ConstStringView{.data=" << static_cast<const void *>(view.data);
  if (view.data != nullptr) {
    os << " -> \"" << std::string_view(view.data, view.size) << '\"';
  }
  os << ", .size=" << view.size << "}";

  return os;
}

constexpr bool operator==(const atb_ConstStringView &lhs,
                          std::string_view rhs) {
  return std::string_view(lhs.data, lhs.size) == rhs;
}

constexpr bool operator!=(const atb_ConstStringView &lhs,
                          std::string_view rhs) {
  return not(lhs == rhs);
}

constexpr bool operator==(std::string_view lhs,
                          const atb_ConstStringView &rhs) {
  return lhs == std::string_view(rhs.data, rhs.size);
}

constexpr bool operator!=(std::string_view lhs,
                          const atb_ConstStringView &rhs) {
  return not(lhs == rhs);
}

constexpr bool operator==(const atb_ConstStringView &lhs,
                          const atb_ConstStringView &rhs) {
  return std::string_view(lhs.data, lhs.size) ==
         std::string_view(rhs.data, rhs.size);
}

constexpr bool operator!=(const atb_ConstStringView &lhs,
                          const atb_ConstStringView &rhs) {
  return not(lhs == rhs);
}

inline std::ostream &operator<<(std::ostream &os, const atb_String &str) {
  os << "atb_String{.data=" << static_cast<void *>(str.data);
  if (str.data != nullptr) {
    os << " -> \"" << std::string_view(str.data, str.size) << '\"';
  }
  os << ", .capacity=" << str.capacity << ", .size=" << str.size << "}";

  return os;
}

inline std::ostream &operator<<(std::ostream &os, atb_String const *const str) {
  os << (void *)str;
  if (str != NULL) {
    os << " -> " << *str;
  }
  return os;
}

constexpr bool operator==(const atb_String &lhs, std::string_view rhs) {
  return std::string_view(lhs.data, lhs.size) == rhs;
}

constexpr bool operator!=(const atb_String &lhs, std::string_view rhs) {
  return not(lhs == rhs);
}

constexpr bool operator==(std::string_view lhs, const atb_String &rhs) {
  return lhs == std::string_view(rhs.data, rhs.size);
}

constexpr bool operator!=(std::string_view lhs, const atb_String &rhs) {
  return not(lhs == rhs);
}

constexpr bool operator==(const atb_String &lhs, const atb_String &rhs) {
  return std::string_view(lhs.data, lhs.size) ==
         std::string_view(rhs.data, rhs.size);
}

constexpr bool operator!=(const atb_String &lhs, const atb_String &rhs) {
  return not(lhs == rhs);
}
