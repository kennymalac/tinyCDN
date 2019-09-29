#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <fstream>
#include <memory>
#include <iostream>
#include <string>
#include <cinttypes>
#include <future>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "file.hpp"
#include "exceptions.hpp"
#include "FileStorage/filesystem.hpp"

using TinyCDN::Utility::Size;

namespace TinyCDN::Middleware::File {

template <typename LockType>
std::unique_ptr<FileBucket>& FileBucketRegistryItem::getBucket(std::shared_mutex& mutex) {
  LockType lock(mutex);
  if (fileBucket.has_value()) {
    return fileBucket.value();
  }

  // The FileBucket object was GHOSTED
  // TODO make creation of FileBuckets into Strategy class
  // TODO use atomic_shared_ptr to make this type of cloning unnecessary
  std::cout << "Making new FileBucket in FileBucketRegistryItem getBucket" << std::endl;
  auto converter = std::make_unique<FileBucketRegistryItemConverter>();
  fileBucket = this->convert(converter);
  return fileBucket.value();
}

// std::unique_ptr<FileBucket> FileBucketRegistry::findOrCreate(
//     bool copyable,
//     bool owned,
//     Size minimumSize,
//     std::vector<std::string> types,
//     //    std::string fileType,
//     std::vector<std::string> tags) {
//   // Introspective sort
//   // TODO optimize
//   std::shared_lock lock(mutex);

//   for (int i = 0; i < registry.size(); i++) {
//     auto& item = registry[i];
//     auto& fbMutex = bucketMutexes[i];
//     auto& fb = item->getBucket<std::shared_lock<std::shared_mutex>>(fbMutex);

//     if (inspectBucket(fb, minimumSize, types)) {
//       auto _fb = std::move(fb);
//       // We need to make sure the optional isn't storing a nullptr
//       item->fileBucket.reset();
//       return _fb;
//     }
//   }
//   lock.unlock();

//   return this->create(true, owned, this->defaultBucketSize, types, tags);
// };

// template <typename StorageBackend>
std::unique_ptr<FileBucket> FileBucketRegistry::create(
    Size size,
    std::vector<std::string> types,
    //    std::string fileType,
    std::vector<std::string> tags) {
  std::cout << "Registry creating new bucket" << std::endl;

  std::unique_lock lock(this->mutex);
  // Create the bucket, create its required directories, and assign the location

  auto bucket = std::make_unique<FileBucket>(this->getUniqueFileBucketId(), size, types);

  // TODO Check if storage cluster has enough space for the FileBucket
  // TODO assign virtual volume to FileBucket

  std::cout << "Bucket storage created" << std::endl;

  // Save file bucket information to REGISTRY
  this->registerItem(bucket);
  return bucket;

  // Create MANIFEST file for content files
  // fs::create_directory(this->location, );

  // TODO make this asynchronous
  // TODO notify nameserver?
}

// FileBucket::FileBucket (Size size, std::vector<std::string> types)
//   : size(size), types(types)
// {

  // TODO: Make sure there is enough space on storage cluster for this size
  // But put this in the Master node, client will check beforehand
  // auto availableSpace = fs::space(registryLocation).available;

  // if (availableSpace < size) {
  //   throw FileBucketException(*this, 0, "FileBucket cannot be created as filesystem has no space.");
  // }
// }

FileBucket::FileBucket (FileBucketId id, Size size, std::vector<std::string> types)
  : id(id), size(size), types(types)
{

  // TODO: Make sure there is enough space on storage cluster for this size
  // But put this in the Master node, client will check beforehand
  // auto availableSpace = fs::space(location).available;

  // storage = std::make_unique<FileStorage::FilesystemStorage>(size, location, true);

  // if (availableSpace < size) {
  //   throw FileBucketException(*this, 0, "FileBucket cannot be created as filesystem has no space.");
  // }
}

//FileBucket::removeFromRegistry () {
//  // Remove all symlinks to this bucket from MANIFEST}
//}

FileBucketRegistryItem::FileBucketRegistryItem (std::unordered_map<std::string, std::string> fields) {
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
  if (field == "virtualVolumeId") {
    VolumeId id;
    id = value;
    params->virtualVolumeId = id;
  }
  else if (field == "id") {
    FileBucketId id;
    id = value;
    params->id = id;
  }
  else if (field == "size") {
    char* nptr;
    params->size = std::strtoumax(value.c_str(), &nptr, 10);
  }
  else if (field == "types") {
    params->types = Utility::fromCSV(value);
  }
}

template <typename T>
std::unique_ptr<T> FileBucketRegistryItemConverter::convertToValue() {
  auto item = std::make_unique<T>(
	params->id,
	Size{params->size},
	// params->virtualVolumeId
	params->types);

  return std::move(item);
}

template <typename StreamType>
StreamType FileBucketRegistry::getRegistry() {}

template<>
std::ifstream FileBucketRegistry::getRegistry() {
  std::ifstream registryFile(this->location / this->registryFileName);

  if (!registryFile.is_open() || registryFile.bad()) {
    throw FileBucketRegistryException(*this, 0, "Registry file could not be opened for reading");
  }

  return registryFile;
}

template<>
std::ofstream FileBucketRegistry::getRegistry() {
  std::ofstream registryFile(this->location / this->registryFileName, std::ios::out | std::ios::app);

  if (!registryFile.is_open() || registryFile.bad()) {
    throw FileBucketRegistryException(*this, 0, "Registry file could not be opened for writing");
  }

  return registryFile;
}

std::optional<std::shared_ptr<FileBucketRegistryItem>> FileBucketRegistry::getItem(FileBucketId fbId) {
  std::optional<std::shared_ptr<FileBucketRegistryItem>> item;

  for (int i = 0; i < registry.size(); i++) {
    auto& _item = registry[i];
    auto& fbMutex = bucketMutexes[i];
    auto& fb = _item->getBucket<std::shared_lock<std::shared_mutex>>(fbMutex);
    if (fb->id == fbId) {
      item = _item;
      // item->fileBucket = fb;
      break;
    }
  }
  return item;
}

void FileBucketRegistry::registerItem(std::unique_ptr<FileBucket>& fb) {
  std::unordered_map<std::string, std::string> input = {
    {"virtualVolumeId", fb->virtualVolumeId.str()},
    {"id", fb->id.str()},
    {"size", std::to_string(fb->size)},
    {"types", Utility::asCSV<std::vector<std::string>>(fb->types)}
  };
  auto item = std::make_shared<FileBucketRegistryItem>(input);

  {
    auto registryFile = this->getRegistry<std::ofstream>();
    registryFile << item->contents << "\n";
  }

  this->registry.push_back(item);
}

std::unique_ptr<FileBucket> FileBucketRegistryItem::convert(std::unique_ptr<FileBucketRegistryItemConverter>& converter) {
  // Extract contents of this registry item
  // For each field, try to find the persisted value of that field
  for (auto field : {"id", "virtualVolumeId", "size", "types"}) {
    auto const assignment = this->assignmentToken(field);
    auto const n = this->contents.find(assignment);
    if (n == std::string::npos) {
      continue;
    }
    auto const valueEnd = this->contents.find(";", n);
    if (valueEnd == std::string::npos) {
      continue;
    }

    auto const len = n + assignment.length();
    const auto value = this->contents.substr(len, valueEnd-len);
    std::cout << assignment << value << "\n";
    std::cout << std::flush;
    // TODO dispatch table...
    try {
      converter->convertField(field, value);
    }
    catch (std::invalid_argument e) {
      throw FileBucketRegistryItemException(*this, 1, std::string{"Failed to convert field "}.append(field).append(" with value ").append(value).append(" ").append(e.what()));
    }
  }

  return converter->convertToValue<FileBucket>();
}

void FileBucketRegistry::loadRegistry() {
  std::unique_lock lock(this->mutex);

  std::string line;
  std::vector<std::unique_ptr<FileBucket>> fbs;

  auto registryFile = this->getRegistry<std::ifstream>();

  auto converter = std::make_unique<FileBucketRegistryItemConverter>();
  std::ios::sync_with_stdio();
  while (getline(registryFile, line)) {
    std::cout << line << "\n";
    auto item = converter->convertInput(line);

    // Set the bucket to the registry item's converted FileBucket
    try {
      item->fileBucket = item->convert(converter);
      registry.emplace_back(std::move(item));
    }
    catch (FileBucketRegistryItemException e) {
      converter->reset();
      continue;
    }

    std::cout << "fb types: " << registry[0]->fileBucket.value()->types[0] << std::endl;

    converter->reset();
  }
}

FileBucketId FileBucketRegistry::getUniqueFileBucketId() {
  auto* id64 = idGenerator(64);

  FileBucketId output;
  std::string s(id64);
  output = s;

  delete id64;
  return output;
}

FileBucketRegistry::FileBucketRegistry(fs::path location, std::string registryFileName)
  : location(location), registryFileName(registryFileName) {}

}
