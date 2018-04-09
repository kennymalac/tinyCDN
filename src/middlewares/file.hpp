#pragma once

#include <vector>
#include <memory>
#include <experimental/filesystem>
#include <unordered_map>
#include <string>
#include <iostream>
#include <fstream>
#include <variant>

#include "src/utility.hpp"
#include "src/storedfile.h"

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


// An individual entry in the File Bucket REGISTRY file

//template <typename tokenType>
struct FileBucket {
  // const size;
  // TypedFileKeystore retrieveKeystore();
  // FileContainer<FileType>& retrieveFile();
  // void stFile(tokenType key, FileContainer<FileType>& file);
  //
  // FileBucket(size )
  // copy

  // TODO gensym
  int id;
  Size size;
  Size allocatedSize;
  std::shared_ptr<FileBucketRegistry> registry;
  fs::path location;
  // NOTE stringly typed for now
  std::vector<std::string> types;

  std::vector<std::shared_ptr<StoredFile>> openFiles;

  // distributionPolicy
  auto inline assignStoredFileSize(StoredFile newContentFile) {
    std::ifstream f;
    f.open(newContentFile.location, std::ios_base::binary | std::ios_base::in);
    if (!f.good() || f.eof() || !f.is_open()) {
      throw FileBucketException(*this, 2);
    }
    f.seekg(0, std::ios_base::beg);
    std::ifstream::pos_type begin_pos = f.tellg();
    f.seekg(0, std::ios_base::end);
    return Size{static_cast<uintmax_t>(f.tellg() - begin_pos)};
  }

  std::shared_ptr<StoredFile> createStoredFile(fs::path tmpfile, Size fileSize, bool temporary);

  FileBucket(int id, Size allocatedSize, std::vector<std::string> types, std::shared_ptr<FileBucketRegistry> registry);
  inline FileBucket (int id, Size allocatedSize, fs::path location, std::vector<std::string> types, std::shared_ptr<FileBucketRegistry> registry)
    : id(id), allocatedSize(allocatedSize), location(location), size(Size{0}), types(types), registry(registry)
  {}

};

struct FileBucketRegistryItem {
  std::string contents = "";

  inline std::string assignmentToken(std::string fieldName) {
    return fieldName + "=";
  }

  // std::
  inline void replaceFieldValues(std::unordered_map<std::string, std::string> fields) {
    // ;id=ID;location=PATH;size=SIZE;types=Audio,Video,Image;
    for (std::pair<std::string, std::string> pair : fields) {
      auto const assignment = assignmentToken(pair.first);
      auto const n = contents.find(assignment);
      // example: types=AUDIO; -> types=AUDIO,VIDEO;
      contents.replace(n + assignment.length(), contents.find(";", n), pair.second);
    }
  }

  FileBucketRegistryItem(std::unordered_map<std::string, std::string> fields);
  inline FileBucketRegistryItem(std::string contents) : contents(contents) {}
};

struct FileBucketRegistry {
  fs::path location;
  std::string registryFileName;
  Size defaultBucketSize{10000000000000000};
  std::vector<std::unique_ptr<FileBucketRegistryItem>> registry;

  inline void registerItem(std::unique_ptr<FileBucket>& fb) {
    std::unordered_map<std::string, std::string> input = {
      {"location", static_cast<std::string>(fb->location)},
      {"id", std::to_string(fb->id)},
      {"size", std::to_string(fb->size.size)},
      {"types", asCSV<std::vector<std::string>>(fb->types)}
    };
    auto item = std::make_unique<FileBucketRegistryItem>(input);

    {
      std::ofstream registryFile(this->location / this->registryFileName);
      registryFile << item->contents << "\n";
    }

    this->registry.push_back(std::move(item));
  }

  auto loadRegistry();

  FileBucketRegistry(fs::path location, std::string registryFileName)
    : location(location), registryFileName(registryFileName) {}
};

struct FileBucketParams {
  int id;
  uintmax_t assignedSize;
  fs::path location;
  // NOTE stringly typed for now
  std::vector<std::string> types;
};

struct FileBucketRegistryItemConverter {
  std::shared_ptr<FileBucketRegistry> registry;
  std::unique_ptr<FileBucketParams> params;

  auto convertInput(std::string input) {
    // TODO sanitize the input

    return std::make_unique<FileBucketRegistryItem>(input);
  }

  auto convertField(std::string field, std::string value);

  auto convertToValue();

  inline void reset() {
    params = std::make_unique<FileBucketParams>();
  }
};

// typedef FileBucket<BLAKEKey> BLAKEBucket;

struct FileUploadingSession {
  std::shared_ptr<FileBucketRegistry>& registry;
  std::tuple<int, std::string> obtainStoredFileUpload(fs::path temporaryLocation, Size fileSize, std::unique_ptr<FileBucket> assignedBucket);
  // StatusField
  auto uploadFile (std::string temporaryLocation, Size fileSize, std::string contentType, std::string fileType, std::vector<std::string> tags, bool wantsOwned);
  // Resides in memory as "active" public buckets
  std::vector<std::unique_ptr<FileBucket>> currentFileBuckets;
};

struct FileHostingSession {
  // std::vector<std::unique_ptr<FileBucket>> currentFileBuckets;

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


struct FileBucketAllocator {
  std::shared_ptr<FileBucketRegistry> registry;
  std::unique_ptr<FileBucket> findOrCreate(
      bool copyable,
      bool owned,
      Size size,
      std::vector<std::string> types,
      //    std::string fileType,
      std::vector<std::string> tags);
  std::unique_ptr<FileBucket> createBucket(
      bool copyable,
      bool owned,
      Size size,
      std::vector<std::string> types,
      //    std::string fileType,
      std::vector<std::string> tags);

  FileBucketAllocator(std::shared_ptr<FileBucketRegistry> registry);
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
