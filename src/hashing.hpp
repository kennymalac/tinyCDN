#include <bitset>
#include <random>
#include <string>
#include <cstring>
#include <sstream>

namespace TinyCDN::Utility::Hashing {

class PseudoRandomHexFactory {
public:
  PseudoRandomHexFactory() {
    std::random_device r;
    re = std::mt19937{r()};
  }

  //! NOTE: this will not free the buffer afterwards!
  char* operator()(int size) {
    char* buffer = new char[size+1];

    for (int i = 0; i<size; i++) {
      buffer[i] = hex[dist(re)];
    }
    buffer[size] = '\0';

    return buffer;
  }

private:
  std::uniform_int_distribution<int> dist{0, 15};
  std::mt19937 re;
  static constexpr char hex[17]{"0123456789ABCDEF"};
};

//! Wrapper type for fixed-size bitset with hex conversion to/from a string
template <int fixedSize>
class Id {
public:
  // //! WARNING: When val is longer than the size of the Id, the rest of the char* is ignored.
  // Id& operator=(char* val) {
  //   auto len = strlen(val);
  //   if (len > fixedSize) {
  //     len = fixedSize;
  //   }

  //   for (int i = 0; i < len; i++) {
  //     _value[i] = val[i];
  //   }

  //   return *this;
  // }

  Id& operator=(std::string val) {
    unsigned long hexVal;
    std::istringstream(val) >> std::hex >> hexVal;

    _value = hexVal;

    return *this;
  }

  bool operator==(const Id<fixedSize> &other) const {
    return this->_value == other->value();
  }

  friend std::ostream& operator<< (std::ostream &out, const Id<fixedSize> &id) {
    out << std::hex << id.value().to_ulong();
    return out;
  }

  friend std::ostream& operator<< (std::ostream &out, Id<fixedSize> &id) {
    out << std::hex << id.value().to_ulong();
    return out;
  }

  std::string str() const {
    std::stringstream hex;
    hex << std::hex << _value.to_ulong();
    return hex.str();
  }

  const char* c_str() const {
    return this->str().c_str();
  }

  inline std::bitset<fixedSize> value() const noexcept {
    return _value;
  }

  constexpr int size() noexcept {
    return fixedSize;
  }

  constexpr int length() noexcept {
    return fixedSize;
  }

  private:
  std::bitset<fixedSize> _value;
};

class IdHasher {
public:
  template <int fixedSize>
  std::size_t operator()(const Id<fixedSize> &id) const {
    const std::bitset<fixedSize> id2 = id.value();
    return std::hash<std::bitset<fixedSize>>()(id2);
  }
};

using UUID = Id<128>;

class UUID4Factory {
public:
  UUID4Factory() {}

  UUID operator()() {
    // NOTE not correct - UUID4 requires a few bits to be set to indicate version
    // TODO refactor this, lol
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
    std::string s(buffer);
    output = s;

    delete id8;
    delete id4_1;
    delete id4_2;
    delete id4_3;
    delete id10;

    return output;
  }

private:
  PseudoRandomHexFactory generator;
};
}
