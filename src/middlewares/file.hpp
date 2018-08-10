#pragma once

#include <vector>
#include <memory>
#include <experimental/filesystem>
#include <unordered_map>
#include <string>
#include <iostream>
#include <fstream>
#include <variant>
#include <future>
#include <mutex>
#include <shared_mutex>
#include <cinttypes>

#include "../utility.hpp"
#include "FileStorage/filesystem.hpp"

namespace fs = std::experimental::filesystem;

namespace TinyCDN::Middleware::File {

// struct TypedFileKeystore {
//   // std::unordered_map<tokenType, std::ifstream& fileData> ;
// };


// template <typename fileType>
// struct FileContainer {
//   std::location location;
// };


// struct FileType;
// struct StatusField;

// struct BLAKEKey {
//   // static constexpr  gensym() {
//   //   // return blake2::s3erverseed=>create
//   //   return ''
//   // };
// };



//template <typename tokenType>
/*!
 * \brief The FileBucket represents a generic persistent data store for any file data and has a fixed size.
 * A FileBucket is persisted through serialization of its fields copied into the FileBucketRegistry's REGISTRY file.
 * It can be configured to only accept a certain set of allowable filetypes as part of its contents.
 */
// template <typename StorageBackend>
struct FileBucket {
  // const size;
  // TypedFileKeystore retrieveKeystore();
  // FileContainer<FileType>& retrieveFile();
  // void stFile(tokenType key, FileContainer<FileType>& file);

  // TODO gensym
  //! The "unique" id
  Storage::fileId id;
  //! The maximum Size allotted for storage
  Size size;
  //! A file storage driver that provides methods to retrieve, modify, and delete files
  std::unique_ptr<FileStorage::FilesystemStorage> storage;
  //! The location of where this FileBucket's storage is present
  fs::path location;
  //! Currently a stringly-typed set of categories of allowable content (i.e. Audio, Video, etc.)
  std::vector<std::string> types;

  // distributionPolicy

  FileBucket(Size size, fs::path registryLocation, std::vector<std::string> types);
  FileBucket(Storage::fileId id, Size size, fs::path location, std::vector<std::string> types);
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
  std::ofstream META;
  std::atomic<Storage::fileId> fileBucketUniqueId;

public:
  const std::string registryFileName;
  mutable std::shared_mutex mutex;

  std::vector<std::shared_ptr<FileBucketRegistryItem>> registry;

  //! FileBuckets are used as unique_ptrs with cloning capabilities - all FileBucket instances act on the same mutexes as to prevent locks and data races
  std::map<int, std::shared_mutex> bucketMutexes;

  //! "active" public FileBuckets that reside in memory until full
  std::vector<std::unique_ptr<FileBucket>> currentFileBuckets;

  fs::path location;
  Size defaultBucketSize{2_mB};

  // std::vector<std::shared_mutex> registryMutexes;

  // template <typename LockType>
  // getRegistryItem(int index) {
  //   LockType lock(registry[i]->mutex);
  // }

  std::optional<std::shared_ptr<FileBucketRegistryItem>> getItem(Storage::fileId fbId);
  /*!
   * \brief registerItem converts an assumingly newly-created FileBucket and appends its configuration as a FileBucketRegistryItem into REGISTRY
   * \param fb a FileBucket instance
   */
  void registerItem(std::unique_ptr<FileBucket>& fb);
  Storage::fileId getUniqueFileBucketId();

  //! Safely returns a stream handle for the Registry
  // TODO: implement registry as a ring buffer
  template <typename StreamType>
  StreamType getRegistry();


  //!
  void loadRegistry();

  std::unique_ptr<FileBucket> findOrCreate(
      bool copyable,
      bool owned,
      Size size,
      std::vector<std::string> types,
      //    std::string fileType,
      std::vector<std::string> tags);

  //! Creates FileBuckets registered with this
  std::unique_ptr<FileBucket> create(
      bool copyable,
      bool owned,
      Size size,
      std::vector<std::string> types,
      //    std::string fileType,
      std::vector<std::string> tags);

  FileBucketRegistry(fs::path location, std::string registryFileName);
  inline FileBucketRegistry(const FileBucketRegistry& r, std::shared_lock<std::shared_mutex> lock)
    : location(r.location), fileBucketUniqueId(r.fileBucketUniqueId.load()), defaultBucketSize(r.defaultBucketSize), registry(r.registry) {};

  inline FileBucketRegistry(const FileBucketRegistry& r) : FileBucketRegistry(r, std::shared_lock<std::shared_mutex>(r.mutex)) {};
};

//! A FileBucket CSV gets converted into this POD and subsequently this data is assigned to a FileBucket instance
struct FileBucketParams {
  Storage::fileId id{0};
  uintmax_t size;
  fs::path location;
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

  //! Takes a FileBucket "field" (location, id, size, or types) and assigns it to its deduced conversion value
  auto convertField(std::string field, std::string value);

  //! Creates a FileBucket or FileBucketRegistry instance by taking params and creating a FileBucket instance from it
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

// typedef FileBucket<BLAKEKey> BLAKEBucket;

class FileUploadingService {
private:
  std::shared_ptr<FileBucketRegistry> registry;

public:
  std::future<std::unique_ptr<FileBucket>> requestFileBucket(std::unique_ptr<FileStorage::StoredFile>& tmpFile, std::string contentType, std::string fileType, std::vector<std::string> tags, bool wantsOwned);

  std::future<std::tuple<Storage::fileId, std::string>> uploadFile (std::unique_ptr<FileBucket> bucket, std::unique_ptr<FileStorage::StoredFile> tmpFile, std::string contentType, std::string fileType, std::vector<std::string> tags);

  // StatusField
  inline FileUploadingService(
      std::shared_ptr<FileBucketRegistry> registry)
    : registry(registry)
  {}
};

class FileHostingService {
private:
  std::shared_ptr<FileBucketRegistry> registry;

public:
  // Tries to obtain a FileBucket given an id
  std::future<std::tuple<std::optional<std::unique_ptr<FileBucket>>, std::optional<std::shared_ptr<FileBucketRegistryItem>>>> obtainFileBucket(Storage::fileId fbId);

  //! Tries to obtain a StoredFile from a bucket given an id
  // Returns a tuple of the StoredFile and a boolean value that denotes if the file exists
  std::future<std::tuple<std::optional<std::unique_ptr<FileStorage::StoredFile>>, bool>> obtainStoredFile(std::unique_ptr<FileBucket>& bucket, Storage::fileId cId, std::string fileName);

  //! Safely obtains a stream to the files contents and destroys the StoredFile instance, readds the bucket to the registry, returns the bucket id
  int hostFile(std::ifstream& stream, std::unique_ptr<FileStorage::StoredFile> file, std::unique_ptr<FileBucket> bucket, std::shared_ptr<FileBucketRegistryItem>& item);

  inline FileHostingService(
      std::shared_ptr<FileBucketRegistry> registry)
    : registry(registry)
  {}
};
}

// this should be a concept
// struct Keystore {
  
// };

// TODO Encryption (v0.3)
// struct BLAKEid {
//   char[] id;
//   inline decrypt (b64 primary_key) {
    
//   };
//   inline encrypt (b64 primary_key) {
    
//   };
// };

// TODO Advanced tokenization (v0.2)
// // unique identifier token class and filetype/object
// template <typename uid, typename m>
// struct TokenRetrieverFactory {
// typedef uid token_type;
// typedef keystore<m> instance_type;

// TokenRetrieverFactory(const TokenFactoryInstanceID) : token_type(token_type), {
    
// }};
