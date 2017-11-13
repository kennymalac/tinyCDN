#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;


auto FileHostingSession::uploadFile(auto file, auto contentType, auto fileType, auto tags, auto wantsOwned) {
  
  // fileBucket bucket;
  // this->assignBucket();

  // If wantsOwned, determine if the user has existing FileBuckets that take these types
  if (wantsOwned) {
    
  }
  else {
    // First remove all full FileBuckets
    std::remove_if(currentFileBuckets.begin(), currentFileBuckets.end(), fb->size >= fb->maxSize);

    // Check if there is a ready bucket that supports this file
    auto& assignedBucket = std::find_if(currentFileBuckets.begin(), currentFileBuckets.end(), [contentType, fileType](auto const& b) {
        return
        // If this FileBucket has enough free space for this file,
        b.maxSize - b.size > file.size
        // supports this file's ContentType,
        && std::find(b.contentTypes.begin(), b.contentTypes.end(), contentType) != b.contentTypes.end()
        // and supports this file's FileType.
        && std::find(b.fileTypes.begin(), b.fileTypes.end(), fileType) != b.fileTypes.end()
      });

    if (assignedBucket != currentFileBuckets.end()) {
      // Great! Now generate an id for this file and store it
      auto ks = assignedBucket->retrieveProperKeystore();
      auto key = ks->generateKey();
      // TODO authenticated files
      assignedBucket->storeFile(file, contentType, fileType, tags);

      return {assignedBucket->uid, key};
    }
    else {
      // i.e. Audio -> read MANIFEST.in in Public/Audio/
      FileBucketAllocator allocator;
      try {
        auto bucket = allocator.findOrCreate(file, contentType, fileType, tags, wantsOwned);
      }
    }
  }
};

FileBucketAllocator::findOrCreate(auto& newFile) {
  // Introspective sort 
};

FileBucket::FileBucket (size : size, types : types) {
  // Create symlink for each mediatype this bucket supports
  // "Audio/"; // 
  // "Video/"; // 
  // "Image/"; // 
};

FileBucket::removeFromRegistry () {
  // Remove all symlinks to this bucket from MANIFEST.in}
};

// auto location = bucket.uid / name;
// if (fs::exists(location)) {
//   auto ftype = location.extension();
//   std::ifstream (location);
//   // TODO async load file
//  };
