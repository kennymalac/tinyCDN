#include "../../include/catch.hpp"

#include <string>
#include <iostream>

#include "src/middlewares/Volume/volume.hpp"

using namespace TinyCDN;
using namespace TinyCDN::Utility;
using namespace TinyCDN::Middleware::Volume;

TEST_CASE("VirtualVolume") {
  SECTION("a new VirtualVolume is created") {
    VirtualVolume volume{VolumeId{"a32b8963a2084ba7"}, 4_gB, 1_gB, fs::current_path(), fs::current_path() / "volumes.db"};
    REQUIRE(volume.id.str() == std::string{"a32b8963a2084ba7"});
    REQUIRE(volume.getSize() == 4_gB);
    REQUIRE(volume.location == fs::current_path());
    // REQUIRE(volume.fbVolDbLocation == fs::current_path() / "volumes.db");
    SECTION("StorageVolumeManager is correctly configured") {
      REQUIRE(volume.storageVolumeManager.defaultVolumeSize == 1_gB);
      REQUIRE(volume.storageVolumeManager.getSize() == 4_gB);
    }

    SECTION("a new FileBucket Volume mapping was created, persisted into db file") {
      REQUIRE(fs::exists(fs::current_path() / "volumes.db"));
      // REQUIRE( db mapping exists, 0 entries, etc. )
    }
    SECTION("commitStorageVolume") {
    }
    SECTION("getFileBucketVolumeIds") {
    }
    SECTION("addFileBucketVolume") {
    }
    SECTION("destroy") {
    }
  }

  SECTION("an existing virtual volume") {
    VirtualVolume volume{VolumeId{"a32b8963a2084ba7"}, 4_gB, 1_gB, fs::current_path(), fs::current_path() / "volumes.db"};
    SECTION("loadDb was called") {
      REQUIRE(fs::exists(fs::current_path() / "volumes.db"));
      // Check that a volume exists that was created and persisted
    }
  }
}
