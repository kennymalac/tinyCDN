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

//! A container representing a FileBucket's persistent state as stored in the FileBucketRegistry
struct FileBucketRegistryItem {
  //! The FileBucket serialized as a CSV
  std::string contents = "";

  //! A reference to the registry item's bucket that was resolved from this item's persisted state
  std::optional<std::unique_ptr<FileBucket>> fileBucket;

  //! Convenience helper method for generating fieldName=
  inline std::string assignmentToken(std::string fieldName) {
    return fieldName + "=";
  }

  FileBucketRegistryItem(std::unordered_map<std::string, std::string> fields);
  inline FileBucketRegistryItem(std::string contents) : contents(contents) {}
};

/*!
 * \brief A container that represents all FileBucket persistent state in a single REGISTRY file comprised of FileBucket CSVs.
 * A REGISTRY file lives in the same directory as the FileBucket. A FileBucketRegistry is merely the the record of said REGISTRY being loaded into memory.
 * The FileBucket creation or modification procedure should trigger an update to FileBucketRegistry to persist its creation/modification into the CDN's shared state.
 * Eventually the REGISTRY will have to live in several files due to the fact that this data structure is inefficient for a large amount of FileBucket creations, modifications, and deletions.
 */
struct FileBucketRegistry {
  fs::path location;
  std::string registryFileName;
  std::ofstream META;
  std::atomic<Storage::fileId> fileBucketUniqueId;
  Size defaultBucketSize{2_mB};
  std::vector<std::unique_ptr<FileBucketRegistryItem>> registry;

  /*!
   * \brief registerItem converts an assumingly newly-created FileBucket and appends its configuration as a FileBucketRegistryItem into REGISTRY
   * \param fb a FileBucket instance
   */
  void registerItem(std::unique_ptr<FileBucket>& fb);
  Storage::fileId getUniqueFileBucketId();

  //! Safely returns a stream handle for the Registry
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
  FileBucketRegistry(const FileBucketRegistry&) = delete;
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

struct FileUploadingService {
  std::future<std::unique_ptr<FileBucket>> requestFileBucket(std::unique_ptr<FileStorage::StoredFile>& tmpFile, std::string contentType, std::string fileType, std::vector<std::string> tags, bool wantsOwned);

  std::future<std::tuple<Storage::fileId, std::string>> uploadFile (std::unique_ptr<FileBucket> bucket, std::unique_ptr<FileStorage::StoredFile> tmpFile, std::string contentType, std::string fileType, std::vector<std::string> tags);

  // TODO make this a mutex
  std::unique_ptr<FileBucketRegistry>& registry;
  // StatusField

  // TODO active buckets should be on REGISTRY lookup table - not uploading session
  //! "active" public FileBuckets that reside in memory until full
  std::vector<std::unique_ptr<FileBucket>> currentFileBuckets;

  inline FileUploadingService(
      std::unique_ptr<FileBucketRegistry>& registry)
    : registry(registry)
  {}
};

struct FileHostingService {
  // Tries to obtain a FileBucket given an id
  std::future<std::optional<std::unique_ptr<FileBucket>>> obtainFileBucket(Storage::fileId fbId);

  //! Tries to obtain a StoredFile from a bucket given an id
  // Returns a tuple of the StoredFile and a boolean value that denotes if the file exists
  std::future<std::tuple<std::optional<std::unique_ptr<FileStorage::StoredFile>>, bool>> obtainStoredFile(std::unique_ptr<FileBucket>& bucket, Storage::fileId cId, std::string fileName);

  //! Returns a stream to the files contents and destroys the StoredFile instance
  void hostFile(std::ifstream& stream, std::unique_ptr<FileBucket> bucket, std::unique_ptr<FileStorage::StoredFile> file);

  // TODO make this a mutex
  std::unique_ptr<FileBucketRegistry>& registry;

  inline FileHostingService(
      std::unique_ptr<FileBucketRegistry>& registry)
    : registry(registry)
  {}
};

struct FileHostingSession {
  // auto findToken(auto token, FileType ft) {
  //   // DFS - Depth first search
  //   for (directory)
  //   ->doesStore
  // }

  FileHostingSession (int threadCount) {
    // TODO decide thread pool
    // thread_pool<threadCount> availThreads;
  }

  ~FileHostingSession () {
    // delete all buckets
    // save buckets to session storen
  }
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
