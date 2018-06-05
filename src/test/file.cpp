#define CATCH_CONFIG_MAIN

#include <tuple>
#include <functional>

#include "include/catch.hpp"

#include "src/master.hpp"
#include "src/middlewares/file.hpp"

namespace file = TinyCDN::Middleware::File;

using namespace std::placeholders;
using namespace TinyCDN;

namespace fs = std::experimental::filesystem;

using fbInputArgs = std::tuple<bool, bool, Size, std::vector<std::string>, std::vector<std::string>>;


SCENARIO("A CDN with Persisting FileBucket storage is restarted") {
  GIVEN("a new CDNMaster") {
    CDNMaster master(false);

    WHEN("the master is spawned") {
      master.spawnCDN();
      THEN("it creates an empty REGISTRY file") {
        REQUIRE(fs::is_empty(fs::path{"REGISTRY"}) == true);
      }

      WHEN("FileBuckets are created by the FileBucketRegistry") {

        std::vector<std::unique_ptr<file::FileBucket>> fileBuckets;

        auto addBucket = [&fbRegistry=master.session->registry]
            (auto t, auto t2, auto t3, auto t4, auto t5) {
          return fbRegistry->create(t, t2, t3, t4, t5);
        };

        auto fbArgs = std::vector<fbInputArgs>{
            std::make_tuple(false, false, Size{1_mB}, std::vector<std::string>{std::string("test")}, std::vector<std::string>{std::string("test2")}),
            std::make_tuple(false, false, Size{2_mB}, std::vector<std::string>{std::string("test")}, std::vector<std::string>{std::string("test2")})
        };
        for (auto spec : fbArgs) {
          fileBuckets.emplace_back(std::apply(addBucket, spec));
        }

        THEN("each FileBucket has an associated FileBucketRegistryItem") {
          REQUIRE( master.session->registry->registry.size() == fileBuckets.size() );

          file::FileBucketRegistryItemConverter converter;
          for (unsigned int i = 0; i < static_cast<unsigned int>(fileBuckets.size()); i++) {
            auto const registryItem = std::move(master.session->registry->registry[i]);
            auto const clonedItem = converter.convertInput(registryItem->contents);
            REQUIRE( registryItem->contents == clonedItem->contents );
          }

        }
        THEN("its REGISTRY file is populated with the each FileBucketRegistryItem contents") {
          REQUIRE(fs::is_empty(fs::path{"REGISTRY"}) == false);

          std::string line;
          std::ifstream registryFile(master.session->registry->location);

          unsigned int counter = 0;
          while (getline(registryFile, line)) {
            REQUIRE( line == master.session->registry->registry[counter++]->contents );
          }
        }

        // Tear down filebuckets
        for (const auto& fb : fileBuckets) {
          fb->storage->destroy();
        }
      }

    }

    // Tear down

    fs::remove("REGISTRY");
    fs::remove("META");
  }
};
