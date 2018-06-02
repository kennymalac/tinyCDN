#define CATCH_CONFIG_MAIN
#include "include/catch.hpp"

#include <experimental/filesystem>
#include <string>
#include <iostream>

#include "src/master.hpp"
//#include "utility.hpp"
//#include "middlewares/file.hpp"

const std::string version = "0.0.1";

namespace file = TinyCDN::Middleware::File;
using namespace TinyCDN;

namespace fs = std::experimental::filesystem;

SCENARIO("a user uploads a file to the CDN") {

  GIVEN("a temporary file has been already stored and CDNMaster initialized and an upload session exists") {
    std::string testValue =
        R"(
        blah blah blah
        blah blah
        blah blah blah
        blah blahblah blah blah
        blah blahblah blah blah
        blah blahblah blah blah
        )";

    std::string fileName = "./copyThis.txt";
    {
      std::ofstream tmpFile(fileName);
      tmpFile << testValue;
    }

    CDNMaster master(false);
    master.spawnCDN();

    auto uploadSession = std::make_unique<file::FileUploadingSession>(
          master.session->registry);

    WHEN("the uploadFile method is invoked to be add a file to a public FileBucket") {
      // Fake file Size, type, etc.
      std::string ctype = "image";
      std::string ftype = "plain/txt";
      auto tags = std::vector<std::string>{"test"};
      auto fileSize = Size{2_mB};

      auto result = uploadSession->uploadFile(fileName, fileSize, ctype, ftype, tags, false);
      THEN("t1") {
      }
    }

    // Tear down
    uploadSession->currentFileBuckets[0]->storage->destroy();
    fs::remove("REGISTRY");
  }
}
