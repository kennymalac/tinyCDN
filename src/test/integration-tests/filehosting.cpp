#include "../include/catch.hpp"

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
    // Get a new master node
    MasterSession masterSession;
    auto [masterLock, master] = masterSession.getMasterNode();
    master->existing = false;
    master->configure(masterSession.loadConfig(fs::path{"master.json"}));
    masterSession.spawn();

    // Get a new storage cluster node
    StorageClusterSession storageClusterSession;
    auto [storageClusterLock, storageCluster] = storageClusterSession.getStorageClusterNode();
    storageCluster->existing = false;
    storageCluster->configure(storageClusterSession.loadConfig(fs::path{"storage.json"}));
    storageClusterSession.spawn();

    // Create a FileBucket
    auto fb = master->registry->create(Size{0_kB}, std::vector<std::string>{"text"}, std::vector<std::string>{"test"});
    auto fbId = fb->id;
    // Assign the file bucket's virtual volume to the storage cluster's
    fb->virtualVolumeId = storageCluster->virtualVolume->id;

    // Add a storage volume to the cluster
    auto storageVolume = storageCluster->virtualVolume->storageVolumeManager->createStorageVolume<StorageVolume<FileStorage::FilesystemStorage>>(std::string{"./"});
    storageCluster->virtualVolume->commitStorageVolume<StorageVolume<FileStorage::FilesystemStorage>>(storageVolume);

    // Assign this storage volume to the bucket
    storageCluster->virtualVolume->addFileBucketVolume(fbId, storageVolume->id);

    // Manually add a temporary file to the storage cluster volume
    std::string fileName = "copyThis.txt";
    {
      std::ofstream tmpFile(std::string{"./"}.append(fileName));
      tmpFile << testValue;
    }

    std::shared_mutex hostedFileMutex;
    auto tmpFile = std::make_unique<storage::StoredFile>("copyThis.txt", true, std::make_unique<std::unique_lock<std::shared_mutex>>(hostedFileMutex));

    std::cout << "Adding file..." << std::endl;
    auto storedFile = storageVolume->storage->add(std::move(tmpFile));

    // Add a file to the bucket
    master->registry->registerFile(fbId, storedFile->id);

    auto fileId = storedFile->id;

    // Simulate a hosting Client using FileHostingSession, FileHostingService
    auto hostingService = std::make_unique<file::FileHostingService>(master->session->registry);

    WHEN("the obtainStoredFile is invoked to retrieve the StoredFile from the bucket") {
      std::cout << "Obtaining stored file..." << std::endl;
      masterLock.unlock();
      auto [_sf, hasValue] = hostingService->obtainStoredFile(masterSession, bucketId, fileId, fileName).get();
      std::cout << "Tried obtaining stored file" << std::endl;

      REQUIRE( hasValue );
      REQUIRE( _sf.has_value() );

      auto sf = std::move(_sf.value());

      REQUIRE( sf->id == storedFile->id );

      THEN("the hostFile method can be invoked to retrieve a stream to the file's contents") {
	std::ifstream stream;
	std::cout << "hosting file..." << std::endl;
	hostingService->hostFile(stream, std::move(storedFile));

	std::string contents((std::istreambuf_iterator<char>(stream)),
			     std::istreambuf_iterator<char>());

	REQUIRE(contents == testValue);
      }
    }

    // Clean up
    storageCluster->virtualVolume->destroy();
    fs::remove("VOLUMES");
    fs::remove("REGISTRY");
  }
}
