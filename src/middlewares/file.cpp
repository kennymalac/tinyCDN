#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <fstream>
#include <memory>
#include <iostream>
#include <string>
#include <cinttypes>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "file.hpp"
#include "exceptions.hpp"
#include "filestorage.hpp"

namespace TinyCDN::Middleware::File {
auto FileUploadingSession::uploadFile(std::string temporaryLocation, Size fileSize, std::string contentType, std::string fileType, std::vector<std::string> tags, bool wantsOwned) {

  // fileBucket bucket;
  // this->assignBucket();

  // If wantsOwned, determine if the user has existing FileBuckets that take these types
  if (wantsOwned) {
    return std::make_tuple(1, static_cast<std::string>("test"));
  }
  else {
    // First remove all full FileBuckets
    std::remove_if(currentFileBuckets.begin(), currentFileBuckets.end(), [](auto const& fb) {
      return fb->size >= fb->allocatedSize;
    });

    // Check if there is a ready bucket that supports this file
    auto maybeBucket = std::find_if(currentFileBuckets.begin(), currentFileBuckets.end(), [contentType, fileType, fileSize](const auto& b) {
      return
          // If this FileBucket has enough free space for this file,
          (b->allocatedSize - b->size) > fileSize
          // supports this file's ContentType,
          && std::find(b->types.begin(), b->types.end(), contentType) != b->types.end();
      // and supports this file's FileType.
      //&& std::find(b->fileTypes.begin(), b->fileTypes.end(), fileType) != b=>fileTypes.end();
    });

    if (maybeBucket == currentFileBuckets.end()) {
      // i.e. Audio -> read MANIFEST.in in Public/Audio/
      FileBucketAllocator allocator{this->registry};
      try {
        auto assignedBucket = allocator.findOrCreate(true, wantsOwned, fileSize, std::vector<std::string>{contentType}, tags);
        return this->obtainStoredFileUpload(temporaryLocation, fileSize, std::move(assignedBucket));
      }
      catch (FileBucketException) {

      }
    }
    else {
      return this->obtainStoredFileUpload(temporaryLocation, fileSize, std::move(*maybeBucket));
    }
  }
};

std::tuple<int, std::string> FileUploadingSession::obtainStoredFileUpload(fs::path temporaryLocation, Size fileSize, std::unique_ptr<FileBucket> assignedBucket) {
  // Great! Now generate an id for this file and store it
  //    auto ks = assignedBucket->retrieveProperKeystore();
  //    auto key = ks->generateKey();

  auto storedFile = assignedBucket->createStoredFile(temporaryLocation, fileSize, true);

  // Copy id and throw away unique_ptr
  auto const fbId = assignedBucket->id;
  currentFileBuckets.push_back(std::move(assignedBucket));

  return {fbId, storedFile->location.string()};
}


FileBucketAllocator::FileBucketAllocator(std::shared_ptr<FileBucketRegistry> registry)
  : registry(registry)
{

}

std::unique_ptr<FileBucket> FileBucketAllocator::findOrCreate(
    bool copyable,
    bool owned,
    Size minimumSize,
    std::vector<std::string> types,
    //    std::string fileType,
    std::vector<std::string> tags) {
  bool bucketExists;

  // Introspective sort
  // TODO

  //  auto& assignedBucket = std::find_if(.begin(), .end(), [contentType, fileType, fileSize](auto const& b) {
  //      return
  //      // If this FileBucket has enough free space for this file,
  //      b.maxSize - b.size > fileSize
  //      // supports this file's ContentType,
  //      && std::find(b.contentTypes.begin(), b.contentTypes.end(), contentType) != b.contentTypes.end()
  //      // and supports this file's FileType.
  //      && std::find(b.fileTypes.begin(), b.fileTypes.end(), fileType) != b.fileTypes.end()
  //    });

  bucketExists = false;

  //  if (bucketExists) {
  //    //return FileBucket{}
  //  }
  // else {
  return this->createBucket(copyable, owned, registry->defaultBucketSize, types, tags);
  // }
};

std::unique_ptr<FileBucket> FileBucketAllocator::createBucket(
    bool copyable,
    bool owned,
    Size size,
    std::vector<std::string> types,
    //    std::string fileType,
    std::vector<std::string> tags) {

  // Assign semi-permanent location
  const int id = 100; //gensym()

  // Create the bucket, create its required directories, and assign the location
  auto bucket = std::make_unique<FileBucket>(id, size, types, this->registry);
  auto location = this->registry->location / fs::path(std::to_string(id));
  fs::create_directory(location);
  for (auto t : types) {
    // Create directory of symlinks for each mediatype this bucket supports
    // e.g.
    // "Audio/"
    // "Video/"
    // "Image/"
    fs::create_directory(location / fs::path(t));
  }
  bucket->location = location;
  bucket->storage = std::make_unique<FileStorage::Haystack>(bucket->allocatedSize, bucket->location, false);

  // Save file bucket information to REGISTRY
  registry->registerItem(bucket);
  return bucket;

  // Create MANIFEST file for content files
  // fs::create_directory(this->location, );

  // TODO make this asynchronous
  // TODO notify nameserver?
}

// TODO constructor for FileBucket that takes in existing location???
FileBucket::FileBucket (int id, Size allocatedSize, std::vector<std::string> types, std::shared_ptr<FileBucketRegistry> registry)
  : id(id), allocatedSize(allocatedSize), size(Size{0}), types(types), registry(registry)
{

  // Make sure there is enough space for this size
  auto availableSpace = fs::space(registry->location).available;

  if (availableSpace < size.size) {
    throw FileBucketException(*this, 0);
  }
}

//FileBucket::removeFromRegistry () {
//  // Remove all symlinks to this bucket from MANIFEST.in}
//}

std::shared_ptr<StoredFile> FileBucket::createStoredFile(fs::path tmpfile, Size fileSize, bool temporary) {
  // tmpfile is the uploaded file stored in /tmp/
  auto temporaryFile = std::make_shared<StoredFile>(location, fileSize, temporary);
  openFiles.push_back(temporaryFile);
  return temporaryFile;
}


// TODO figure out return type
auto FileBucket::getFile(std::size_t position, std::size_t fileSize) {
  std::vector<std::unique_ptr<FileStorage::HaystackBlock>> test;
  for (auto it = this->storage->read(position); it <= it.end(position + fileSize.size); ++it) {

  }
}

// auto location = bucket.uid / name;
// if (fs::exists(location)) {
//   auto ftype = location.extension();
//   std::ifstream (location);
//   // TODO async load file
//  };

FileBucketRegistryItem::FileBucketRegistryItem (std::unordered_map<std::string, std::string> fields) {
  // RAII
  //contents = "";

  for (std::pair<std::string, std::string> pair : fields) {
    // TODO optimize
    contents.append(pair.first);
    contents.append("=");
    contents.append(pair.second);
    contents.append(";");
  }
}

auto FileBucketRegistryItemConverter::convertField(std::string field, std::string value) {
  // TODO validate permissions etc.
  // return
  if (field == "location") {
    params->location = fs::path{value};
  }
  else if (field == "id") {
    params->id = std::stoi(value);
  }
  else if (field == "allocatedSize") {
    char* nptr;
    params->assignedSize = std::strtoumax(value.c_str(), &nptr, 10);
  }
  else if (field == "types") {
    params->types = fromCSV(value);
  }
}

auto FileBucketRegistryItemConverter::convertToValue() {
  auto fb = std::make_unique<FileBucket>(
        params->id,
        Size{params->assignedSize},
        params->location,
        params->types,
        this->registry);

  fb->storage = std::make_unique<FileStorage::Haystack>(fb->allocatedSize, fb->location, false);

  return fb;
}

auto FileBucketRegistry::loadRegistry() {
  std::string line;
  std::ifstream registryFile(this->location / this->registryFileName);

  if (!registryFile.is_open()) {
    throw FileBucketRegistryException(*this, 0, "Registry file could not be opened");
  }

  std::unique_ptr<FileBucketRegistryItemConverter> converter;
  while (getline(registryFile, line)) {
    auto item = converter->convertInput(line);

    // Extract contents of this registry item
    // For each field, try to find the persisted value of that field
    for (auto field : {"id", "location", "allocatedSize", "types"}) {
      auto const assignment = item->assignmentToken(field);
      auto const n = item->contents.find(assignment);
      if (n == std::string::npos) {
        continue;
      }
      auto const valueEnd = item->contents.find(";", n);
      if (valueEnd == std::string::npos) {
        continue;
      }

      const auto value = item->contents.substr(n + assignment.length(), valueEnd);
      // TODO dispatch table...
      converter->convertField(field, value);
    }

    auto persistedBucket = converter->convertToValue();
    converter->reset();
  }

  if (registryFile.bad()) {
    throw FileBucketRegistryException(*this, 0, "Error while loading Registry file");
  }
}
}
