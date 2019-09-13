#include <algorithm>
#include <bitset>
#include <random>
#include <string>
#include <cstring>
#include <sstream>
//#include <iomanip>
#include <ctype.h>

namespace TinyCDN::Utility::Hashing {

template <typename T>
const char* hexCharToBinary(T c) {
  switch(toupper(c)) {
  case '0': return "0000";
  case '1': return "0001";
  case '2': return "0010";
  case '3': return "0011";
  case '4': return "0100";
  case '5': return "0101";
  case '6': return "0110";
  case '7': return "0111";
  case '8': return "1000";
  case '9': return "1001";
  case 'A': return "1010";
  case 'B': return "1011";
  case 'C': return "1100";
  case 'D': return "1101";
  case 'E': return "1110";
  case 'F': return "1111";
  default: throw new std::logic_error("Not a hexidecimal character");
  }
}

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
  Id& operator=(std::string val) {
    _value = std::bitset<fixedSize>(hexToBinary(val));

    return *this;
  }

  bool operator==(const Id<fixedSize> &other) const {
    return this->_value == other->value();
  }

  friend std::ostream& operator<< (std::ostream &out, const Id<fixedSize> &id) {
    out << id.str();
    return out;
  }

  friend std::ostream& operator<< (std::ostream &out, Id<fixedSize> &id) {
    out << id.str();
    return out;
  }

  std::string str() const {
    std::stringstream hex;

    auto const str = _value.to_string();

    for (int i = 0; i < str.length(); i+=4) {
      auto const val = std::stoi(str.substr(i,4), nullptr, 2);
      hex << std::hex << val;
    }

    return hex.str();
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

  inline std::string hexToBinary(std::string hexInput) {
    std::string hexVal;
    std::istringstream(hexInput) >> std::hex >> hexVal;

    std::stringstream result;

    for (auto const c : hexVal) {
      result << hexCharToBinary<const char>(c);
    }

    return result.str();
  }

protected:
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

class UUID4 : public Id<128> {
public:
  UUID4& operator=(std::string val) {
    std::string hexVal;
    auto _val = val.substr();

    // Remove any hyphens
    _val.erase(std::remove(_val.begin(), _val.end(), '-'), _val.end());
    Id::operator=(_val);

    // Assign UUID version to 4
    // Set 4 most significant bytes of 7th most significent byte to 4
    _value.reset(76); // 0
    _value.reset(77); // 0
    _value.set(78); // 1
    _value.reset(79); // 0

    // Set 2 most significant bytes of 9th most significent byte (reserved UUID bits)
    _value.reset(62);
    _value.set(63);

    return *this;
  }

  std::string str() const {
    auto out = Id::str();
    out.insert(8, "-");
    out.insert(13, "-");
    out.insert(18, "-");
    out.insert(23, "-");

    return out;
  }

  friend std::ostream& operator<< (std::ostream &out, const UUID4 &id) {
    out << id.str();
    return out;
  }

  friend std::ostream& operator<< (std::ostream &out, UUID4 &id) {
    out << id.str();
    return out;
  }
};

class UUID4Factory {
public:
  UUID4Factory() {}

  UUID4 operator()() {
    auto* id32 = generator(32);

    UUID4 output;
    std::string s(id32);
    // UUID4 operator= sets version for us
    output = s;

    delete id32;

    return output;
  }

private:
  PseudoRandomHexFactory generator;
};
}
