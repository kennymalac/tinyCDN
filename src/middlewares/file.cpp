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

namespace TinyCDN::Middleware::File {

auto FileUploadingService::requestFileBucket(
  std::unique_ptr<FileStorage::StoredFile>& tmpFile,
  std::string contentType,
  std::string fileType,
  std::vector<std::string> tags,
  bool wantsOwned)
  -> std::future<std::unique_ptr<FileBucket>> {
  auto fileSize = tmpFile->size;

  std::ios::sync_with_stdio();

  // TODO If wantsOwned, determine if the user has existing FileBuckets that take these types
  if (wantsOwned) {
    std::cout << "wantsOwned is true" << std::endl;
    try {
      auto assignedBucket = registry->findOrCreate(true, wantsOwned, fileSize, std::vector<std::string>{contentType}, tags);
      std::promise<std::unique_ptr<FileBucket>> test;
      test.set_value(std::move(assignedBucket));
      return test.get_future();
    }
    catch (FileBucketException e) {
      std::cerr << e.what();
    }
  }
  else {
    // First remove all full FileBuckets
    {
      std::unique_lock wLock(registry->mutex);
      std::remove_if(registry->currentFileBuckets.begin(), registry->currentFileBuckets.end(), [](auto const& fb) {
          std::cout << fb->size.size << fb->storage->getAllocatedSize().size << "\n";
          return fb->size >= fb->storage->getAllocatedSize();
        });
    }

    std::shared_lock lock(registry->mutex);
    // Check if there is a ready bucket that supports this file
    auto maybeBucket = std::find_if(registry->currentFileBuckets.begin(), registry->currentFileBuckets.end(), [contentType, fileType, fileSize](const auto& b) {
        return
          // If this FileBucket has enough free space for this file,
          (b->storage->getAllocatedSize() - b->size) > fileSize
          // supports this file's ContentType,
          && std::find(b->types.begin(), b->types.end(), contentType) != b->types.end();
        // and supports this file's FileType.
        //&& std::find(b->fileTypes.begin(), b->fileTypes.end(), fileType) != b=>fileTypes.end();
      });

    if (maybeBucket == registry->currentFileBuckets.end()) {
      std::cout << "No bucket available for content type: " << contentType << std::endl;
      try {
        lock.unlock();
        std::promise<std::unique_ptr<FileBucket>> test;

        test.set_value(registry->findOrCreate(true, wantsOwned, fileSize, std::vector<std::string>{contentType}, tags));
        return test.get_future();
      }
      catch (FileBucketException e) {
        std::cerr << e.what();
      }
    }
    else {
      std::promise<std::unique_ptr<FileBucket>> test;
      test.set_value(std::move(*maybeBucket));
      return test.get_future();
    }
  }
}

auto FileUploadingService::uploadFile(
  std::unique_ptr<FileBucket> bucket,
  std::unique_ptr<FileStorage::StoredFile> tmpFile,
  std::string contentType,
  std::string fileType,
  std::vector<std::string> tags)
-> std::future<std::tuple<Storage::fileId, std::string>> {
  // Great! Now generate an id for this file and store it
  //    auto ks = assignedBucket->retrieveProperKeystore();
  //    auto key = ks->generateKey();
  auto storedFile = bucket->storage->add(std::move(tmpFile));
  // TODO actually make this async
  std::promise<std::tuple<Storage::fileId, std::string>> test;

  // Copy id and throw away unique_ptr
  auto const fbId = bucket->id;
  std::unique_lock lock(registry->mutex);
  registry->currentFileBuckets.push_back(std::move(bucket));

  test.set_value({fbId, std::to_string(storedFile->id.value())});
  return test.get_future();
}


std::future<std::tuple<std::optional<std::unique_ptr<FileBucket>>, std::optional<std::shared_ptr<FileBucketRegistryItem>>>> FileHostingService::obtainFileBucket(Storage::fileId fbId) {
  // TODO const
  // TODO actually make this async
  std::promise<std::tuple<std::optional<std::unique_ptr<FileBucket>>, std::optional<std::shared_ptr<FileBucketRegistryItem>>>> test;
  std::optional<std::unique_ptr<FileBucket>> bucket;

  // TODO C++20: use atomic_shared_ptr
  // TODO std::scoped_lock registryLock();

  auto item = registry->getItem(fbId);
  if (item.has_value()) {
    bucket = std::move(item.value()->fileBucket.value());
  }

  test.set_value(std::make_tuple(std::move(bucket), item));
  return test.get_future();
}

std::future<std::tuple<std::optional<std::unique_ptr<FileStorage::StoredFile>>, bool>> FileHostingService::obtainStoredFile(std::unique_ptr<FileBucket>& bucket, Storage::fileId cId, std::string fileName) {
  // TODO actually make this async
  std::promise<std::tuple<std::optional<std::unique_ptr<FileStorage::StoredFile>>, bool>> test;

  std::optional<std::unique_ptr<FileStorage::StoredFile>> maybeFile;
  auto exists = false;

  try {
    auto storedFile = bucket->storage->lookup(cId);
    exists = true;

    if (storedFile->location.filename() == fileName) {
      maybeFile = std::move(storedFile);
    }
  }
  catch (File::FileStorageException& e) {
    std::ios::sync_with_stdio();
    std::cout << e.what() << std::endl;
  }

  test.set_value(std::make_tuple(std::move(maybeFile), exists));
  return test.get_future();
}

int FileHostingService::hostFile(std::ifstream& stream, std::unique_ptr<FileStorage::StoredFile> file, std::unique_ptr<FileBucket> bucket, std::shared_ptr<FileBucketRegistryItem>& item) {
  // TODO safely obtain a lock to the file from the bucket?
  stream = file->getStream<std::ifstream>();
  auto const fbId = bucket->id;

  item->fileBucket = std::move(bucket);

  return fbId;
}

bool inspectBucket(std::unique_ptr<FileBucket>& fb, Size minimumSize, std::vector<std::string> types) {
  std::cout << "filebucket types: ";
  for (auto const ctype : fb->types) {
    std::cout << ctype << ", ";
  }
  std::cout << std::endl;

  // If this FileBucket has enough free space for this file,
  auto fbSize = fb->storage->getAllocatedSize();
  if (fb->size - fbSize >= minimumSize) {
    // supports the specified ContentTypes,
    auto const notSupportsCtypes = std::any_of(types.cbegin(), types.cend(), [&fb](auto const contentType) {
      return std::find(fb->types.cbegin(), fb->types.cend(), contentType) == fb->types.end();
    });
    //std::cout << notSupportsCtypes << "\n";
    // and supports this file's FileType.
    // && std::find(b.fileTypes.begin(), b.fileTypes.end(), fileType) != b.fileTypes.end()
    if (!notSupportsCtypes) {
      return true;
    }
  }
  return false;
}

template <typename LockType>
std::unique_ptr<FileBucket>& FileBucketRegistryItem::getBucket(std::shared_mutex& mutex) {
  LockType lock(mutex);
  if (fileBucket.has_value()) {
    return fileBucket.value();
  }

  // The FileBucket object was GHOSTED
  // TODO use atomic_shared_ptr to make this type of cloning unnecessary
  std::cout << "Making new FileBucket in FileBucketRegistryItem getBucket" << std::endl;
  auto converter = std::make_unique<FileBucketRegistryItemConverter>();
  fileBucket = this->convert(converter);
  return fileBucket.value();
}

std::unique_ptr<FileBucket> FileBucketRegistry::findOrCreate(
    bool copyable,
    bool owned,
    Size minimumSize,
    std::vector<std::string> types,
    //    std::string fileType,
    std::vector<std::string> tags) {
  // Introspective sort
  // TODO optimize
  std::shared_lock lock(mutex);

  for (int i = 0; i < registry.size(); i++) {
    auto& item = registry[i];
    auto& fbMutex = bucketMutexes[i];
    auto& fb = item->getBucket<std::shared_lock<std::shared_mutex>>(fbMutex);

    if (inspectBucket(fb, minimumSize, types)) {
      auto _fb = std::move(fb);
      // We need to make sure the optional isn't storing a nullptr
      item->fileBucket.reset();
      return _fb;
    }
  }
  lock.unlock();

  return this->create(true, owned, this->defaultBucketSize, types, tags);
};

// template <typename StorageBackend>
std::unique_ptr<FileBucket> FileBucketRegistry::create(
    bool copyable,
    bool owned,
    Size size,
    std::vector<std::string> types,
    //    std::string fileType,
    std::vector<std::string> tags) {
  std::cout << "Registry creating new bucket" << std::endl;
  auto const fbId = this->getUniqueFileBucketId();

  std::unique_lock lock(this->mutex);
  // Create the bucket, create its required directories, and assign the location
  auto bucket = std::make_unique<FileBucket>(size, this->location, types);
  bucket->id = fbId;

  // Assign semi-permanent location
  auto fbLocation = this->location / fs::path(std::to_string(fbId));
  fs::create_directory(fbLocation);

  bucket->location = fbLocation;
  bucket->storage = std::make_unique<FileStorage::FilesystemStorage>(bucket->size, bucket->location, false);

  std::cout << "Bucket storage created" << std::endl;

  // Save file bucket information to REGISTRY
  this->registerItem(bucket);
  return bucket;

  // Create MANIFEST file for content files
  // fs::create_directory(this->location, );

  // TODO make this asynchronous
  // TODO notify nameserver?
}

FileBucket::FileBucket (Size size, fs::path registryLocation, std::vector<std::string> types)
  : size(size), types(types)
{

  // Make sure there is enough space for this size
  auto availableSpace = fs::space(registryLocation).available;

  if (availableSpace < size.size) {
    throw FileBucketException(*this, 0, "FileBucket cannot be created as filesystem has no space.");
  }
}

FileBucket::FileBucket (Storage::fileId id, Size size, fs::path location, std::vector<std::string> types)
  : id(id), size(size), location(location), types(types)
{

  // Make sure there is enough space for this size
  auto availableSpace = fs::space(location).available;

  storage = std::make_unique<FileStorage::FilesystemStorage>(size, location, true);

  if (availableSpace < size.size) {
    throw FileBucketException(*this, 0, "FileBucket cannot be created as filesystem has no space.");
  }
}

//FileBucket::removeFromRegistry () {
//  // Remove all symlinks to this bucket from MANIFEST}
//}


// TODO figure out return type
//auto FileBucket::getFile(std::size_t position, std::size_t fileSize) {
//  std::vector<std::unique_ptr<FileStorage::HaystackBlock>> test;
//  for (auto it = this->storage->read(position); it <= it.end(position + fileSize.size); ++it) {

//  }
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
  if (field == "location") {
    params->location = fs::path{value};
  }
  else if (field == "id") {
    params->id = std::stoul(value);
  }
  else if (field == "size") {
    char* nptr;
    params->size = std::strtoumax(value.c_str(), &nptr, 10);
  }
  else if (field == "types") {
    params->types = fromCSV(value);
  }
}

template <typename T>
std::unique_ptr<T> FileBucketRegistryItemConverter::convertToValue() {
  auto item = std::make_unique<T>(
        params->id,
        Size{params->size},
        params->location,
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

std::optional<std::shared_ptr<FileBucketRegistryItem>> FileBucketRegistry::getItem(Storage::fileId fbId) {
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
    {"location", static_cast<std::string>(fb->location)},
    {"id", std::to_string(fb->id)},
    {"size", std::to_string(fb->size.size)},
    {"types", asCSV<std::vector<std::string>>(fb->types)}
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
  for (auto field : {"id", "location", "size", "types"}) {
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

Storage::fileId FileBucketRegistry::getUniqueFileBucketId() {
  std::unique_lock<std::shared_mutex> lock(mutex);
  auto const id = ++fileBucketUniqueId;
  // Persist incremented id to META
  META.clear();
  META.seekp(0);
  META << fileBucketUniqueId;
  META.flush();
  return id;
}

FileBucketRegistry::FileBucketRegistry(fs::path location, std::string registryFileName)
  : location(location), registryFileName(registryFileName) {

  if (fs::exists(this->location / "META")) {
    try {
      std::ifstream _meta(this->location / "META");
      if (!_meta.is_open() || _meta.bad()) {
        throw File::FileBucketRegistryException(*this, 2, "META file does not exist or is not available");
      }

      std::string id((std::istreambuf_iterator<char>(_meta)),
                     std::istreambuf_iterator<char>());
      _meta.close();

      fileBucketUniqueId = static_cast<Storage::fileId>(std::stoul(id));
    }
    catch (std::invalid_argument e) {
      throw File::FileBucketRegistryException(*this, 2, std::string{"META file cannot be parsed: "}.append(e.what()));
    }
  }
  else {
    fileBucketUniqueId = 0;
  }

  META = std::ofstream(this->location / "META");
  META.seekp(0);
  META << fileBucketUniqueId;
  META.flush();
}

}
