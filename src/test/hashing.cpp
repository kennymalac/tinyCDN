#include <bitset>
#include <iostream>
#include <cstring>
#include <string>
#include <ctype.h>

#include "include/catch.hpp"

#include "src/hashing.hpp"

using namespace TinyCDN;
using namespace TinyCDN::Utility::Hashing;

bool isValidHex(std::string s) {
  std::vector<char> hex{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  for (auto const _c : s) {
    auto const c = toupper(_c);
    if (std::none_of(hex.cbegin(), hex.cend(), [&c](auto hexChar){ return hexChar == c; })) {
      return false;
    }
  }
  return true;
}

TEST_CASE("PseudoRandomHexFactory", "[utility]") {
  PseudoRandomHexFactory generator;

  WHEN("Random strings are generated") {
    auto* hex8 = generator(8);
    auto* hex32 = generator(32);
    auto* hex64 = generator(64);


    THEN("A random hex of the specified length is returned for each") {
      std::cout << "hex8: "  << hex8  << " | " \
		<< "hex32: " << hex32 << " | " \
		<< "hex64: " << hex64 << ";\n";

      REQUIRE( strlen(hex8) == 8 );
      REQUIRE( strlen(hex32) == 32 );
      REQUIRE( strlen(hex64) == 64 );
    }
    delete hex8;
    delete hex32;
    delete hex64;
  }
}

TEST_CASE("UUID4Factory", "[utility]") {
  UUID4Factory generator;

  WHEN("UUID4 is generated") {
    auto id = generator();

    THEN("a proper UUID4 is returned") {
      std::cout << "UUID4: " << id << "\n";

      REQUIRE( typeid(id) == typeid(UUID4) );
      REQUIRE( strlen(id.str().c_str()) == 36 );

      auto id8 = id.str().substr(0,8);
      std::cout << "id8: " << id8 << "\n";
      REQUIRE( isValidHex(id8) );
      auto id4_1 = id.str().substr(9,4);
      std::cout << "id4_1: " << id4_1 << "\n";
      REQUIRE( isValidHex(id4_1) );
      auto id4_2 = id.str().substr(14,4);
      std::cout << "id4_2: " << id4_2 << "\n";
      REQUIRE( isValidHex(id4_2) );
      auto id4_3 = id.str().substr(19, 4);
      std::cout << "id4_3: " << id4_3 << "\n";
      REQUIRE( isValidHex(id4_3) );
      auto id10 = id.str().substr(24, 12);
      std::cout << "id10: " << id10 << "\n";
      REQUIRE( isValidHex(id10) );

      // Version should be set to 4
      REQUIRE( id4_2[0] == '4' );

      // Two bits are reserved
      std::vector<char> validChars{'8', '9', 'a', 'b'};
      auto const c = id4_3[0];

      REQUIRE( std::any_of(validChars.cbegin(), validChars.cend(), [&c](auto validChar){ return c == validChar; }) );
    }
  }
}

