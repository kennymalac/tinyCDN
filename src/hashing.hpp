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

  //! NOTE: this will not free the buffer afterwards!
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
    _value.fill('\0');

    auto len = strlen(val);
    if (len > fixedSize) {
      len = fixedSize;
    }

    for (int i = 0; i < len; i++) {
      _value[i] = val[i];
    }

    return *this;
  }

  Id& operator=(std::array<char, fixedSize> val) {
    std::copy(std::begin(val), std::end(val), std::begin(_value));
    return *this;
  }

  friend std::ostream& operator<< (std::ostream &out, const Id<fixedSize> &id) {
    for (auto c : id.value()) {
      out << c;
    }
    return out;
  }

  friend std::ostream& operator<< (std::ostream &out, Id<fixedSize> &id) {
    for (auto c : id.value()) {
      out << c;
    }
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
  // Use string_view and char* value in place of this?
  std::array<char, fixedSize> _value;
};

using UUID = Id<32>;

class UUID4Factory {
public:
  UUID4Factory() {}

  UUID operator()() {
    auto* id8   = generator(8);
    auto* id4_1 = generator(4);
    auto* id4_2 = generator(4);
    auto* id4_3 = generator(4);
    auto* id10  = generator(10);

    char buffer[33];

    strcpy(buffer, id8);
    buffer[8] = '-';
    buffer[9] = '\0';
    strcat(buffer, id4_1);
    buffer[13] = '-';
    buffer[14] = '\0';
    strcat(buffer, id4_2);
    buffer[18] = '-';
    buffer[19] = '\0';
    strcat(buffer, id4_3);
    buffer[23] = '-';
    buffer[24] = '\0';
    strcat(buffer, id10);

    UUID output;
    output = buffer;

    delete id8;
    delete id4_1;
    delete id4_2;
    delete id4_3;
    delete id10;

    return output;
  }

private:
  PseudoRandomAlphanumericFactory generator;
};
}
