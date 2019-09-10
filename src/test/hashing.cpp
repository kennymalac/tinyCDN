#include <bitset>
#include <iostream>
#include <cstring>
#include <string>

#include "include/catch.hpp"

#include "src/hashing.hpp"

using namespace TinyCDN;
using namespace TinyCDN::Utility::Hashing;

SCENARIO("Random ids are generated") {
  GIVEN("a Pseudo Random hex string factory") {
    PseudoRandomHexFactory generator;

    WHEN("Random ids are generated") {
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
  GIVEN("an Id instance") {
    Id<16> id;

    WHEN("an Id is assigned to a hex string") {
      id = "AABB";
      THEN("the internal bitset has the hex value in bits") {
	std::cout << "bitset value: " << id << "\n";
	REQUIRE( typeid(id.value()) == typeid(std::bitset<16>) );
	REQUIRE( strlen(id.c_str()) == 4 );
	REQUIRE( id.str() == "aabb" );
      }
    }
  }
  // GIVEN("UUID4 factory") {
  //   UUID4Factory generator;

  //   WHEN("UUID is generated") {
  //     auto id = generator();

  //     THEN("UUID is returned") {
  //       std::cout << id << "\n";

  //       REQUIRE( typeid(id) ==  typeid(UUID) );
  //       REQUIRE( strlen(id.c_str()) == 32 );
  //     }
  //   }
  // }
}
