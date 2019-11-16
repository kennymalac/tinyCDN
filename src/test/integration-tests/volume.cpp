#include "include/catch.hpp"

#include <string>
#include <iostream>

using namespace TinyCDN;
using namespace TinyCDN::Utility;

SCENARIO("A CDN storage node is used to store filebuckets into volumes") {
  GIVEN("A spawned CDN Master with setup communication to a CDN Volume server") {
    auto* master = (new MasterNodeSingleton)->getInstance(false);
    master->spawnCDN(); // opens a socket

    StorageServer storageServer{"127.0.0.1", 994999, master->hostname, master->port};

    WHEN("A config for the volumes are loaded") {
      THEN("") {
      }
    }

    WHEN("A bucket is created") {
      THEN("The master assigns a volume to the bucket") {
	auto const& volume = master->getVirtualVolume(volume.id);
	REQUIRE( volumeId === volume.id );
      }
      THEN("The virtual volume can retrieve bucket files as a volume") {
	
      }
    }
  }
}
