#define CATCH_CONFIG_MAIN

#include <tuple>
#include <functional>

#include "include/catch.hpp"

#include "src/master.hpp"
#include "src/middlewares/file.hpp"

namespace file = TinyCDN::Middleware::File;
namespace storage = TinyCDN::Middleware::FileStorage;

using namespace std::placeholders;
using namespace TinyCDN;

namespace fs = std::experimental::filesystem;

using fbInputArgs = std::tuple<bool, bool, Size, std::vector<std::string>, std::vector<std::string>>;


auto static fbArgs = std::vector<fbInputArgs>{
  std::make_tuple(false, false, Size{1_mB}, std::vector<std::string>{std::string("test")}, std::vector<std::string>{std::string("test2")}),
  std::make_tuple(false, false, Size{2_mB}, std::vector<std::string>{std::string("test")}, std::vector<std::string>{std::string("test2")})
};

SCENARIO("A new CDN is created") {

  GIVEN("a new CDNMasterSingleton") {
    auto* master = (new CDNMasterSingleton)->getInstance(false);
    master->existing = false;

    WHEN("the master is spawned") {
      master->spawnCDN();
      THEN("it creates an empty REGISTRY file") {
        REQUIRE(fs::is_empty(fs::path{"REGISTRY"}) == true);

      AND_WHEN("FileBuckets are created by the FileBucketRegistry") {

        std::vector<std::unique_ptr<file::FileBucket>> fileBuckets;

        auto const& fbRegistry = master->session->registry;
        // Factory function for adding FileBuckets
        auto addBucket = [&fbRegistry]
            (auto t, auto t2, auto t3, auto t4, auto t5) {
          return fbRegistry->create(t, t2, t3, t4, t5);
        };

        for (auto spec : fbArgs) {
          fileBuckets.emplace_back(std::apply(addBucket, spec));
        }

        THEN("each FileBucket has an associated FileBucketRegistryItem and its REGISTRY file is populated with the each FileBucketRegistryItem contents") {
          REQUIRE( fbRegistry->registry.size() == fileBuckets.size() );

          file::FileBucketRegistryItemConverter converter;
          for (unsigned int i = 0; i < static_cast<unsigned int>(fileBuckets.size()); i++) {
            auto const& registryItem = master->session->registry->registry[i];
            auto const clonedItem = converter.convertInput(registryItem->contents);
            REQUIRE( registryItem->contents == clonedItem->contents );
          }

          REQUIRE(fs::is_empty(fs::path{"REGISTRY"}) == false);

          std::string line;
          std::ifstream registryFile(fbRegistry->location / fbRegistry->registryFileName);

          unsigned int counter = 0;
          auto const len = fbRegistry->registry.size();
          while (getline(registryFile, line)) {
            // NOTE: order is backwards for registry because it prepends, not appends
            REQUIRE( line == fbRegistry->registry[counter++]->contents );
          }

          REQUIRE( counter == fbArgs.size() );
        }
      }
      }
    }
  }
};

SCENARIO("A CDN with Persisting FileBucket storage is restarted") {

  GIVEN("a persisted CDNMaster with persisted buckets") {
    auto* master = (new CDNMasterSingleton)->getInstance(true);
    master->existing = true;

    WHEN("the master is spawned") {
      master->spawnCDN();
      THEN("The registry is initialized and it loads its FileBuckets into memory") {

        auto &fbRegistry = master->session->registry;
        auto findBucket = [&fbRegistry]
            (auto t, auto t2, auto t3, auto t4, auto t5) {
          return fbRegistry->findOrCreate(t, t2, t3, t4, t5);
        };

        REQUIRE( fbRegistry->registry.size() == fbArgs.size() );

        using fbTestProps = std::tuple<storage::fileId&, Size, Size&, fs::path, std::vector<std::string>&, std::vector<std::string>&>;

        std::vector<Size> fbSizes;
        std::vector<std::vector<std::string>> fbTypes;
        std::vector<storage::fileId> fbIds;

        for (unsigned int i = 0; i < static_cast<unsigned int>(fbArgs.size()); i++) {
          auto const fbArg = fbArgs[i];
          auto const& bucket = std::apply(findBucket, fbArg);
          auto const& registryItem = fbRegistry->registry[i];
          std::cout << "registryItem: " << registryItem->contents << "\n";

          // Ownership of the FileBucket was transferred to this scope
          REQUIRE( registryItem->fileBucket.has_value() == false );

          // Test the fields of the FileBucket
          fbSizes.emplace_back(static_cast<Size>(std::get<2>(fbArgs[i])));
          fbTypes.emplace_back(static_cast<std::vector<std::string>>(std::get<4>(fbArgs[i])));
          fbIds.emplace_back(static_cast<storage::fileId>(i+1));

          REQUIRE( bucket->id == fbIds[i] );
          REQUIRE( bucket->location == (fbRegistry->location / fs::path{std::to_string(fbIds[i])}) );
          REQUIRE( bucket->size.size == fbSizes[i].size );

          REQUIRE( fbRegistry->registry.size() == fbArgs.size() );

          // Test FileBucket storage
          REQUIRE( bucket->storage->getAllocatedSize().size == Size{0_kB}.size );

          // Tear down filebucket
          bucket->storage->destroy();
        }
      }
    }

    // Tear down
    fs::remove("REGISTRY");
    fs::remove("META");
  }
};
