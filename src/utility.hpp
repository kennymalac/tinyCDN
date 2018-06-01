#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace TinyCDN {

constexpr std::size_t operator""_kB(unsigned long long v) {
  return 1024u * v;
}

constexpr std::size_t operator""_mB(unsigned long long v) {
  return 1048576u * v;
}

constexpr std::size_t operator""_gB(unsigned long long v) {
  return 1073741824 * v;
}

struct Size {
  // Immutable
  const uintmax_t size;
  Size(uintmax_t size) : size(size) {}

  const Size operator+(Size& S2) const {
    return Size{this->size + S2.size};
  }
  const Size operator-(Size& S2) const {
    return Size{this->size - S2.size};
  }
  const Size operator*(Size& S2) const {
    return Size{this->size * S2.size};
  }
  const Size operator/(Size& S2) const {
    return Size{this->size / S2.size};
  }
  bool operator>(const Size& S2) const {
    return this->size > S2.size;
  }
  bool operator>=(const Size& S2) const {
    return this->size >= S2.size;
  }
};

/*! Takes a container and converts to a comma-separated string
 * Treats a comma'd string value as a container of string-convertible values.
 * Overload for specific types to create k-tuples from stored text
 * This is used to convert structures into a format that can be created at run-time by reading from a file.
 */
template <typename t>
std::string asCSV(t container);

/*! Takes a comma-separated string and outputs a vector of the string's values
 */
std::vector<std::string> fromCSV(std::string csv);
}
