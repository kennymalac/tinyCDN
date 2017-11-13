#include <vector>
#include <memory>
#include <experimental/filesystem>


struct TypedFileKeystore {
  // std::unordered_map<tokenType, std::ifstream& fileData> ;
};


template <typename fileType>
struct FileContainer {

};


struct FileType;
struct StatusField;

struct BLAKEKey {
  // static constexpr  gensym() {
  //   // return blake2::s3erverseed=>create
  //   return ''
  // };
};


template <typename tokenType>
struct FileBucket {
  // const size;
  TypedFileKeystore retrieveKeystore();
  FileContainer<FileType>& retrieveFile();
  void stFile(tokenType key, FileContainer<FileType>& file);
  //
  // FileBucket(size ) 
  // copy
};

typedef FileBucket<BLAKEKey> BLAKEBucket;

struct FileHostingSession {
  // Resides in memory as "active" public buckets
  std::vector<std::unique_ptr<BLAKEBucket>> currentFileBuckets;

  // auto findToken(auto token, FileType ft) {
  //   // DFS - Depth first search
  //   for (directory)
  //   ->doesStore
  // }

  StatusField uploadFile ();
  FileHostingSession (int threadCount) {
    // TODO decide thread pool
    // thread_pool<threadCount> availThreads;
  };

  ~FileHostingSession () {
    // delete all buckets
    // save buckets to session store
  };
};


// template <typename bucketType>
// struct FileBucketAllocator {
//   std::unique_ptr<bucketType> findOrCreate();
//   std::unique_ptr<bucketType> createBucket(bool copyable);
// };

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
