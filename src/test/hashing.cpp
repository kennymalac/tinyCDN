#include <iostream>
#include <cstring>
#include <string>

#include "include/catch.hpp"

#include "src/hashing.hpp"

using namespace TinyCDN;

SCENARIO("Random ids are generated") {
  GIVEN("Pseudo Random id factories of varying lengths") {
    PseudoRandomAlphanumericFactory generator;

    WHEN("Random ids are generated") {
      auto* id8 = generator(8);
      auto* id32 = generator(32);
      auto* id64 = generator(64);

      THEN("A random id of the specified length is returned for each") {
        std::cout << "id8: "  << id8  << " | " \
                  << "id32: " << id32 << " | " \
                  << "id64: " << id64 << ";\n";
        
        REQUIRE( strlen(id8) == 8 );
        REQUIRE( strlen(id32) == 32 );
        REQUIRE( strlen(id64) == 64 );
      }
    }
  }
}
