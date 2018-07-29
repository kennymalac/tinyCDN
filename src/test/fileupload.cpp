#include "include/catch.hpp"

#include <experimental/filesystem>
#include <string>
#include <iostream>

#include "src/master.hpp"
//#include "utility.hpp"
//#include "middlewares/file.hpp"

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

SCENARIO("a user uploads a file to the CDN") {

  GIVEN("a temporary file has been already stored and CDNMaster initialized and an upload session exists") {
    std::string fileName = "copyThis.txt";
    {
      std::ofstream tmpFile(std::string{"./"}.append(fileName));
      tmpFile << testValue;
    }

    auto* master = (new CDNMasterSingleton)->getInstance(false);
    master->existing = false;
    master->spawnCDN();

    auto uploadService = std::make_unique<file::FileUploadingService>(
          master->session->registry);


    WHEN("the requestBucket method is invoked to obtain an available public FileBucket") {
      // Fake file Size, type, etc.
      std::string ctype = "image";
      std::string ftype = "plain/txt";
      auto tags = std::vector<std::string>{"test"};
      auto fileSize = Size{fs::file_size(fileName)};

      auto storedFile = std::make_unique<storage::StoredFile>(fileSize, fileName, true);
      auto bucket = uploadService->requestFileBucket(storedFile, ctype, ftype, tags, false).get();

      THEN("An empty bucket is retrieved") {
        REQUIRE( bucket->storage->getAllocatedSize().size == 0 );

        AND_WHEN("the uploadFile method is invoked to add a file to the public FileBucket") {
          auto result = uploadService->uploadFile(std::move(bucket), std::move(storedFile), ctype, ftype, tags).get();
          const auto& [fileId, fbId] = result;

          THEN("the file can be retrieved and the allocatedSize was increased") {
            // TODO get bucket from fbId
            auto&& fb = uploadService->currentFileBuckets[0];
            auto sf = fb->storage->lookup(fileId);

            REQUIRE( sf->id == fileId );
            // TODO test situation with same filename
            REQUIRE( sf->location == (master->session->registry->location / fs::path(fbId) / "store" / "1" / fileName) );

            REQUIRE( fb->storage->getAllocatedSize().size == fileSize.size );

            {
              std::ifstream storedFileIn(sf->location);
              std::string fileContents((std::istreambuf_iterator<char>(storedFileIn)),
                                       std::istreambuf_iterator<char>());
              REQUIRE( fileContents == testValue );
            }
          }
        }
      }
    }
  }
}

SCENARIO("The CDN with Persisting FileBucket storage and a uploaded file is restarted") {

  GIVEN("A spawned CDNMaster and a persisted FileBucket") {
    auto* master = (new CDNMasterSingleton)->getInstance(true);
    master->existing = true;
    master->spawnCDN();
    auto&& fileBucket = master->session->registry->registry[0]->fileBucket.value();

    // TODO test duplicate file name upload

    WHEN("the FileBucket instance is present and StoredFile retrieved") {
      storage::fileId const fileId = 1;
      auto storedFile = fileBucket->storage->lookup(static_cast<storage::fileId>(1));

      THEN("the FileBucket storage is still the same allocatedSize, the StoredFile id is the same, the StoredFile is readable") {
        REQUIRE( fileBucket->storage->getAllocatedSize().size == fs::file_size(storedFile->location) );
        REQUIRE( storedFile->id == fileId );
        std::ifstream storedFileIn(storedFile->location);
        std::string fileContents((std::istreambuf_iterator<char>(storedFileIn)),
                                 std::istreambuf_iterator<char>());
        REQUIRE( fileContents == testValue );

        AND_WHEN("an uploaded file is removed") {
          // NOTE: when adding multithreading, address possible race condition if file is being fetched before delete?
          storage::fileId const fileId = 1;
          auto storedFile = fileBucket->storage->lookup(static_cast<storage::fileId>(1));
          fileBucket->storage->remove(std::move(storedFile));

          THEN("The storedFile is no longer in scope, the allocatedSize was decreased, and a lookup fails") {
            REQUIRE( storedFile == nullptr );
            REQUIRE( fileBucket->storage->getAllocatedSize().size == 0 );
            REQUIRE_THROWS_AS( fileBucket->storage->lookup(static_cast<storage::fileId>(1)), file::FileStorageException );
          }
        }
      }
      // Tear down
      fileBucket->storage->destroy();
      fs::remove("REGISTRY");
      fs::remove("META");
    }
  }
}

// TODO test with multiple buckets
