#include "include/catch.hpp"

#include <string>
#include <iostream>

#include "src/middlewares/Volume/volume.hpp"

using namespace TinyCDN;
using namespace TinyCDN::Utility;
using namespace TinyCDN::Middleware::Volume;

TEST_CASE("Volume") {

  SECTION("Loading a Volume from a configuration file") {
    REQUIRE(true == true);
  }
}
