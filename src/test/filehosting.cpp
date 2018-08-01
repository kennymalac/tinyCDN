#include "include/catch.hpp"

#include <experimental/filesystem>
#include <string>
#include <iostream>

#include "src/master.hpp"
//#include "utility.hpp"
//#include "src/middlewares/file.hpp"

namespace file = TinyCDN::Middleware::File;
namespace storage = TinyCDN::Middleware::FileStorage;
using namespace TinyCDN;

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

  GIVEN("a StoredFile has been already stored in a FileBucket, a CDNMaster initialized, and a hosting session exists") {
    std::string fileName = "copyThis.txt";
    {
      std::ofstream tmpFile(std::string{"./"}.append(fileName));
      tmpFile << testValue;
    }

    auto* master = (new CDNMasterSingleton)->getInstance(false);
    master->existing = false;
    master->spawnCDN();

    auto tmpFile = std::make_unique<storage::StoredFile>("copyThis.txt", true);

    auto fb = master->session->registry->create(true, false, Size{0_kB}, std::vector<std::string>{"text"}, std::vector<std::string>{"test"});
    auto fbId = fb->id;

    master->session->registry->registry[0]->fileBucket = std::move(fb);

    // Add the file to the filebucket
    auto storedFile = fb->storage->add(std::move(tmpFile));
    auto fileId = storedFile->id.value();

    auto hostingService = std::make_unique<file::FileHostingService>(master->session->registry);

    WHEN("the obtainFileBucket method is invoked to obtain the bucket containing the hosted file") {
      // TODO expand these test(s)
      auto bucket = hostingService->obtainFileBucket(fbId).get().value();

      THEN("The correct bucket is retrieved") {
        auto const bucketId = bucket->id;

        // TODO expand these test(s)
        REQUIRE( bucket->id == bucketId );
        REQUIRE( fb == nullptr );

        AND_WHEN("the obtainStoredFile is invoked to retrieve the StoredFile from the bucket") {
          auto sf = std::get<0>(hostingService->obtainStoredFile(bucket, fileId, fileName).get()).value();

          REQUIRE( sf->id == storedFile->id );

          THEN("the hostFile method can be invoked to retrieve a stream to the file's contents") {
            std::ifstream stream;
            hostingService->hostFile(stream, std::move(bucket), std::move(storedFile));
            //REQUIRE(typeof(std::ofstream) == typeof(stream));

            std::string contents((std::istreambuf_iterator<char>(stream)),
                                 std::istreambuf_iterator<char>());

            REQUIRE(contents == testValue);

          }
        }
      }

      bucket->storage->destroy();
      fs::remove("REGISTRY");
      fs::remove("META");
    }
  }
}
