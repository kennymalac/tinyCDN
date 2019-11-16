#include "../../include/catch.hpp"

#include <string>
#include <iostream>

#include "src/utility.hpp"
#include "src/middlewares/Volume/volume.hpp"
#include "src/middlewares/Volume/marshaller.hpp"

using namespace TinyCDN;
using namespace TinyCDN::Utility;
using namespace TinyCDN::Middleware::Volume;

TEST_CASE("VolumeParams") {
  VolumeParams params{VolumeId{"a32b8963a2084ba7"}, 1000_kB};
  REQUIRE(params.id.str() == std::string{"a32b8963a2084ba7"});
  REQUIRE(params.size == 1000_kB);
}

TEST_CASE("VirtualVolumeParams") {
  VirtualVolumeParams params{VolumeId{"a32b8963a2084ba7"}, fs::current_path(), fs::current_path() / "volumes.db", 1_gB, 4};
  REQUIRE(params.id.str() == std::string{"a32b8963a2084ba7"});
  REQUIRE(params.size == 4_gB);
  REQUIRE(params.location == fs::current_path());
  REQUIRE(params.fbVolDbLocation == fs::current_path() / "volumes.db");
  REQUIRE(params.defaultVolumeSize == 1_gB);
  REQUIRE(params.volumeLimit == 4);
}

// TEST_CASE("VolumeCSVMarshaller") {}

TEST_CASE("VirtualVolumeMarshaller") {
  VirtualVolumeMarshaller marshaller{};

  SECTION("getInstance returns unique_ptr of VirtualVolume") {
    VirtualVolumeParams params{VolumeId{"a32b8963a2084ba7"}, fs::current_path(), fs::current_path() / "volumes.db", 1_gB, 4};
    auto volume = marshaller.getInstance(params);

    REQUIRE(typeid(volume) == typeid(std::unique_ptr<VirtualVolume>));
    REQUIRE(volume->id.str() == std::string{"a32b8963a2084ba7"});
    REQUIRE(volume->getSize() == 4_gB);
    REQUIRE(volume->location == fs::current_path());
    // REQUIRE(volume->fbVolDbLocation == fs::current_path() / "volumes.db");
    // REQUIRE(volume->volumeLimit == 4);
  }
}
