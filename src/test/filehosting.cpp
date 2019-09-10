#include "include/catch.hpp"

#include <experimental/filesystem>
#include <string>
#include <iostream>

#include "src/middlewares/Master/master.hpp"
//#include "utility.hpp"
//#include "src/middlewares/file.hpp"

namespace file = TinyCDN::Middleware::File;
namespace storage = TinyCDN::Middleware::FileStorage;
using namespace TinyCDN;
using namespace TinyCDN::Utility;
using namespace TinyCDN::Middleware::Master;

namespace fs = std::experimental::filesystem;

static std::string testValue =
        R"(
        blah blah blah
        blah blah
        blah blah blah
        blah blahblah blah blah
        blah blahblah blah blah
        blah blahblah blah blah
        )";

SCENARIO("a user retrieves a file to the CDN") {

  GIVEN("a StoredFile has been already stored in a FileBucket, a MasterNode initialized, and a hosting session exists") {
    std::string fileName = "copyThis.txt";
    {
      std::ofstream tmpFile(std::string{"./"}.append(fileName));
      tmpFile << testValue;
    }

    auto* master = (new MasterNodeSingleton)->getInstance(false);
    master->existing = false;
    master->spawnCDN();

    std::shared_mutex hostedFileMutex;
    auto tmpFile = std::make_unique<storage::StoredFile>("copyThis.txt", true, std::make_unique<std::unique_lock<std::shared_mutex>>(hostedFileMutex));

    auto fb = master->session->registry->create(true, false, Size{0_kB}, std::vector<std::string>{"text"}, std::vector<std::string>{"test"});

    auto fbId = fb->id;

    // Add the file to the filebucket
    std::cout << "Adding file..." << std::endl;
    auto storedFile = fb->storage->add(std::move(tmpFile));
    std::cout << "Moving bucket..." << std::endl;
    master->session->registry->registry[0]->fileBucket = std::move(fb);

    auto fileId = storedFile->id.value();

    auto hostingService = std::make_unique<file::FileHostingService>(master->session->registry);

    WHEN("the obtainFileBucket method is invoked to obtain the bucket containing the hosted file") {
      // TODO expand these test(s)
      std::cout << "Obtaining file bucket..." << std::endl;
      auto [_bucket, _registryItem] = hostingService->obtainFileBucket(fbId).get();

      THEN("The correct bucket is retrieved") {
        REQUIRE( _bucket.has_value() );
        REQUIRE( _registryItem.has_value() );
        auto bucket = std::move(_bucket.value());
        auto registryItem = _registryItem.value();

        auto const bucketId = bucket->id;

        // TODO expand these test(s)

        REQUIRE( bucket->id == bucketId );
        REQUIRE( fb == nullptr );

        AND_WHEN("the obtainStoredFile is invoked to retrieve the StoredFile from the bucket") {
          std::cout << "Obtaining stored file..." << std::endl;
          auto [_sf, hasValue] = hostingService->obtainStoredFile(bucket, fileId, fileName).get();
          std::cout << "Tried obtaining stored file" << std::endl;

          REQUIRE( hasValue );
          REQUIRE( _sf.has_value() );

          auto sf = std::move(_sf.value());

          REQUIRE( sf->id == storedFile->id );

          THEN("the hostFile method can be invoked to retrieve a stream to the file's contents") {
            std::ifstream stream;
            std::cout << "hosting file..." << std::endl;
            hostingService->hostFile(stream, std::move(storedFile), std::move(bucket), registryItem);

            std::string contents((std::istreambuf_iterator<char>(stream)),
                                  std::istreambuf_iterator<char>());

            REQUIRE(contents == testValue);
          }
        }
      }
    }

    // Clean up
    master->session->registry->registry[0]->fileBucket.value()->storage->destroy();
    fs::remove("REGISTRY");
    fs::remove("META");
  }
}
