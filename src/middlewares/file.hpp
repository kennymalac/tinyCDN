#pragma once

#include <vector>
#include <memory>
#include <experimental/filesystem>
#include <unordered_map>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <variant>
#include <mutex>
#include <shared_mutex>
#include <cinttypes>

#include "../utility.hpp"
#include "../hashing.hpp"

namespace fs = std::experimental::filesystem;

using namespace TinyCDN::Utility::Hashing;
using TinyCDN::Utility::Size;
using TinyCDN::Utility::operator""_mB;

namespace TinyCDN::Middleware::File {

using FileBucketId = Id<64>;
using VolumeId = Id<64>;

/*!
 * \brief The FileBucket represents a generic persistent data store for any file data and has a fixed size.
 * A FileBucket is persisted through serialization of its fields copied into the FileBucketRegistry's REGISTRY file.
 * It can be configured to only accept a certain set of allowable filetypes as part of its contents.
 */
struct FileBucket {
  //! The unique id
  FileBucketId id;
  //! The maximum Size allotted for storage
  Size size;
  //! How much has been allocated already
  uintmax_t allocatedSize;

  //! The bucket's assigned virtual volume is responsible for retrieving, modifying, and deleting files
  VolumeId virtualVolumeId;
  //! Currently a stringly-typed set of categories of allowable content (i.e. Audio, Video, etc.)
  std::vector<std::string> types;
  // TODO remove this in migration
  //std::unique_ptr<FileStorage::FilesystemStorage> storage;


  // distributionPolicy

  FileBucket(Size size, std::vector<std::string> types);
  FileBucket(FileBucketId id, Size size, std::vector<std::string> types);
};

struct FileBucketRegistryItemConverter;

//! A container representing a FileBucket's persistent state as stored in the FileBucketRegistry
struct FileBucketRegistryItem {
  //! The FileBucket serialized as a CSV
  std::string contents = "";

  //! A reference to the registry item's bucket that was resolved from this item's persisted state
  std::optional<std::unique_ptr<FileBucket>> fileBucket;

  //! Converts a FileBucketRegistryItem into a FileBucket instance
  std::unique_ptr<FileBucket> convert(std::unique_ptr<FileBucketRegistryItemConverter>& converter);

  //! Safely obtains a unique_ptr to a FileBucket with the specified lock
  template <typename LockType>
  std::unique_ptr<FileBucket>& getBucket(std::shared_mutex& mutex);

  //! Convenience helper method for generating fieldName=
  inline std::string assignmentToken(std::string fieldName) {
    return fieldName + "=";
  }

  FileBucketRegistryItem(std::unordered_map<std::string, std::string> fields);
  inline FileBucketRegistryItem(const FileBucketRegistryItem& i)
    : contents(i.contents) {};

  inline FileBucketRegistryItem(std::string contents) : contents(contents) {}
};

/*!
 * \brief A container that represents all FileBucket persistent state in a single REGISTRY file comprised of FileBucket CSVs.
 * A REGISTRY file lives in the same directory as the FileBucket. A FileBucketRegistry is merely the the record of said REGISTRY being loaded into memory.
 * The FileBucket creation or modification procedure should trigger an update to FileBucketRegistry to persist its creation/modification into the CDN's shared state.
 * Eventually the REGISTRY will have to live in several files due to the fact that this data structure is inefficient for a large amount of FileBucket creations, modifications, and deletions.
 */
class FileBucketRegistry {
private:
  PseudoRandomHexFactory idGenerator;
  // TODO move to metadata node
  std::unordered_map<FileBucketId, std::vector<FileId>, IdHasher> fbFileDb;

public:
  const std::string registryFileName;
  mutable std::shared_mutex mutex;

  std::vector<std::shared_ptr<FileBucketRegistryItem>> registry;

  //! FileBuckets are used as unique_ptrs with cloning capabilities - all FileBucket instances act on the same mutexes as to prevent locks and data races
  std::map<int, std::shared_mutex> bucketMutexes;

  //! "active" public FileBuckets that reside in memory until full
  // std::vector<std::unique_ptr<FileBucket>> currentFileBuckets;

  fs::path location;

  // std::vector<std::shared_mutex> registryMutexes;

  std::optional<std::shared_ptr<FileBucketRegistryItem>> getItem(FileBucketId fbId);
  //std::unique_ptr<FileBucket> getBucket(FileBucketId fbId); // use std::future?

  /*!
   * \brief registerItem converts an assumingly newly-created FileBucket and appends its configuration as a FileBucketRegistryItem into REGISTRY
   * \param fb a FileBucket instance
   */
  void registerItem(std::unique_ptr<FileBucket>& fb);

  FileBucketId getUniqueFileBucketId();

  //! Safely returns a stream handle for the Registry
  // TODO: implement registry as a ring buffer
  template <typename StreamType>
  StreamType getRegistry();

  //!
  void loadRegistry();

  //! Creates FileBuckets registered with this
  std::unique_ptr<FileBucket> create(
      Size size,
      std::vector<std::string> types,
      //    std::string fileType,
      std::vector<std::string> tags);

  //! Registers a file id to a filebucket
  void registerFile(FileBucketId fbId, FileId fileId);

  FileBucketRegistry(fs::path location, std::string registryFileName);

  FileBucketRegistry(const FileBucketRegistry& r) = delete;
};

//! A FileBucket CSV gets converted into this POD and subsequently this data is assigned to a FileBucket instance
struct FileBucketParams {
  FileBucketId id;
  uintmax_t size;
  VolumeId virtualVolumeId;
  // NOTE stringly typed for now
  std::vector<std::string> types;
};

//! Converts a FileBucketParams into a FileBucketRegistryItem
struct FileBucketRegistryItemConverter {
  std::unique_ptr<FileBucketParams> params;

  //! Simply takes a string and deduces a FileBucketRegistryItem
  inline auto convertInput(std::string input) {
    // TODO sanitize the input

    return std::make_unique<FileBucketRegistryItem>(input);
  }

  //! Takes a FileBucket "field" (virtualVolumeId, id, size, or types) and assigns it to its deduced conversion value
  auto convertField(std::string field, std::string value);

  //! Creates a FileBucket or FileBucketRegistryItem instance by taking params and creating a FileBucket instance from it
  template <typename T>
  std::unique_ptr<T> convertToValue();

  //! Resets the converter by emptying the current FileBucketParams
  inline void reset() {
    params = std::make_unique<FileBucketParams>();
  }

  inline FileBucketRegistryItemConverter() {
    params = std::make_unique<FileBucketParams>();
  }
};

}
