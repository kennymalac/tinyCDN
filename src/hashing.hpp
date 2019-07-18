#include <array>
#include <random>
#include <string>
#include <cstring>

namespace TinyCDN {

class PseudoRandomAlphanumericFactory {
public:
  PseudoRandomAlphanumericFactory() {
    std::random_device r;
    re = std::mt19937{r()};
  }

  char* operator()(int size) {
    char* buffer = new char[size+1];

    for (int i = 0; i<size; i++) {
      buffer[i] = alphanumeric[dist(re)];
    }
    buffer[size] = '\0';

    return buffer;
  }

private:
  std::uniform_int_distribution<int> dist{0, 61};
  std::mt19937 re;
  static constexpr char alphanumeric[63]{"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"};
};


//! Wrapper type for fixed size C++-style fixed-length array with conversion method to C string
template <int fixedSize>
class Id {
public:
  //! WARNING: When val is longer than the size of the Id, the rest of the char* is ignored.
  //! Undefined Behavior when val is not lengthy enough for this Id
  Id& operator=(char* val) {
    char buffer[fixedSize+1];
    strncpy(buffer, val, fixedSize);

    std::copy(std::begin(buffer), std::end(buffer), std::begin (_value));
    return *this;
  }

  Id& operator=(std::array<char, fixedSize> val) {
    std::copy(std::begin(val), std::end(val), std::begin(_value));
    return *this;
  }

  friend std::ostream& operator<< (std::ostream &out, const Id<fixedSize> &id) {
    out << id.value();
    return out;
  }

  friend std::ostream& operator<< (std::ostream &out, Id<fixedSize> &id) {
    out << id.value();
    return out;
  }

  inline constexpr char* c_str() {
    return *(char(*)[fixedSize+1]) _value.data();
  }

  inline constexpr std::array<char, fixedSize> value() {
    return _value;
  }

  inline constexpr int size() noexcept {
    return fixedSize;
  }

  inline constexpr int length() noexcept {
    return fixedSize;
  }

 private:
  std::array<char, fixedSize> _value;
};

using UUID = Id<128>;

class UUID4Factory {
public:
  UUID4Factory() {}

  UUID operator()() {
    char buffer[128];
    strcat(buffer, generator(8));
    buffer[9] = '-';
    strcat(buffer, generator(4));
    buffer[14] = '-';
    strcat(buffer, generator(4));
    buffer[19] = '-';
    strcat(buffer, generator(4));
    buffer[24] = '-';
    strcat(buffer, generator(10));

    UUID output;
    output = buffer;
    return output;
  }

private:
  PseudoRandomAlphanumericFactory generator;
};
}
