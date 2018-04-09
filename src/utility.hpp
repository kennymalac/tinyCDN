#pragma once

#include <vector>

#include "src/middlewares/exceptions.hpp"

namespace TinyCDN {
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

template <typename t>
std::string asCSV(t container);

std::vector<std::string> fromCSV(std::string csv);
}
