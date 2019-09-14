#ifdef __cplusplus
#include "client.h"
#include "middlewares/Master/master.hpp"
// Everywhere a Master is used, use a Client
#include "services.hpp"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <memory>
#include "utility.hpp"

#include "../file.hpp"

using TinyCDN::Utility::operator""_kB;
namespace TinyCDN {

auto getUploadInfo(FileUploadInfo* info) {
  std::string temporaryLocation(info->temporaryLocation);
  std::string contentType(info->contentType);
  std::string fileType(info->fileType);
  // TODO tags
  std::vector<std::string> tags;
  bool wantsOwned(info->wantsOwned);

  std::ios::sync_with_stdio();
  std::cout << "getUploadInfo | temporaryLocation: " << temporaryLocation << " " << "contentType: " << contentType << " " << "fileType: " << fileType << std::endl;

  return std::make_tuple(temporaryLocation, contentType, fileType, tags, wantsOwned);
}

extern "C" {
  // enum ContentType {
  //   Image,
  //   Video
  // };

  // struct Session {
  // };
  struct FileUploadInfo* cc_FileUploadInfo_new (char* fileBucketId,
						char* temporaryLocation,
						char* contentType,
						char* fileType,
						char* tags,
						int wantsOwned) {
    auto* info = new FileUploadInfo(fileBucketId, temporaryLocation, contentType, fileType, tags, wantsOwned);
    std::ios::sync_with_stdio();
    std::cout << "cc_FileUploadInfo_new | temporaryLocation: " << info->temporaryLocation << " " << "contentType: " << info->contentType << " " << "fileType: " << info->fileType << std::endl;

    return info;
  }

  struct HostedFileInfo* cc_HostedFileInfo_new (char* id,
						char* fName,
						int owned) {
    auto* info = new HostedFileInfo(id, fName, owned);

    std::ios::sync_with_stdio();
    std::cout << "cc_HostedFileInfo_new | id: " << info->id << " " << "fileName: " << info->fileName << " " << "owned: " << info->owned << std::endl;

    return info;
  };

  FileUploadingSession* cc_FileUploadingSession_new () {
    auto* master = new Middleware::Master::MasterNodeSingleton;
    auto* session = new FileUploadingSession;

    std::ios::sync_with_stdio();
    std::cout << "cc_FileUploadingSession_new" << std::endl;

    // TODO don't use a singleton - use an object pool
    auto* service = new FileUploadingServiceSingleton;
    session->uploadService = service->getInstance();

    return session;
  }

 // FileUploadingSession* requestFileUploadingSession (SessionProvisioner* master) {
  //   return reinterpret_cast<SessionProvisioner*>(master)->getUploadingSession();
  // }

  void cc_FileUploadingSession_delete(FileUploadingSession* session) {
    delete reinterpret_cast<FileUploadingSession*>(session);
  }
  // char* getUniqueFileID (FileUploadingSession* session) {
  //   // TODO gensym
  //   return reinterpret_cast<FileUploadingSession*>(session)->;
  // }

  // uid getFileBucketId (FileUploadingSession* session, ContentType ctype, char* fileType) {
  //   // TODO gensym
  //   return ;
  // }

  // char* getFileBucketLocation (FileUploadingSession* session) {
  //   // TODO gensym
  //   return;
  // }

  int cc_FileUploadingSession_fetchBucket (struct FileUploadingSession* _session, FileUploadInfo* info) {
    auto session = reinterpret_cast<FileUploadingSession*>(_session);
    std::ios::sync_with_stdio();
    std::cout << "cc_FileUploadingSession_fetchBucket" << std::endl;

    auto [fileBucketId, temporaryLocation, contentType, fileType, tags, wantsOwned] = getUploadInfo(reinterpret_cast<FileUploadInfo*>(info));

    session->uploadingFile = std::make_unique<Middleware::FileStorage::StoredFile>(
      temporaryLocation,
      true,
      std::make_unique<std::unique_lock<std::shared_mutex>>(session->uploadingFileMutex));

    session->bucket = session->uploadService->requestFileBucket(fileBucketId, session->uploadingFile, contentType, fileType, tags, wantsOwned).get();

    return static_cast<int>(session->bucket->id);
  };


  // void beginFileUpload (FileUploadingSession* session) {
  //   return
  // }

  // void FileUploadingSession_startFileUpload (FileUploadingSession* session) {
  //   session->file = reinterpret_cast<StoredFile*>(new StoredFile{
  //   });
  // }

  void cc_FileUploadingSession_finishFileUpload (FileUploadingSession* _session, char* cffiResult[], FileUploadInfo* info) {
    // uploadService
    auto session = reinterpret_cast<FileUploadingSession*>(_session);

    std::ios::sync_with_stdio();
    std::cout << "cc_FileUploadingSession_finishFileUpload" << std::endl;

    auto [fileBucketId, temporaryLocation, contentType, fileType, tags, wantsOwned] = getUploadInfo(info);

    std::cout << "starting upload" << std::endl;
    auto _result = session->uploadService->uploadFile(std::move(session->bucket), std::move(session->uploadingFile), contentType, fileType, tags);
    std::cout << "waiting.." << std::endl;
    _result.wait();

    auto result = _result.get();
    std::cout << "upload done, returning" << std::endl;

    // C-ify the result

    auto const _storedFileId = result;
    auto const storedFileId = _storedFileId.c_str();

    cffiResult[0] = new char [sizeof(fbId)];
    strcpy(cffiResult[0], fileBucketId);
    cffiResult[1] = new char [sizeof(storedFileId)];
    strcpy(cffiResult[1], storedFileId);
  }

  struct FileHostingSession* cc_FileHostingSession_new () {
    auto* master = new Middleware::Master::MasterNodeSingleton;
    auto* session = new FileHostingSession;

    std::ios::sync_with_stdio();
    std::cout << "cc_FileHostingSession_new" << std::endl;

    // TODO don't use a singleton - use an object pool
    auto* service = new FileHostingServiceSingleton;
    session->hostingService = service->getInstance();

    return session;
  }

  void cc_FileHostingSession_delete (struct FileHostingSession* session) {
    delete reinterpret_cast<FileHostingSession*>(session);
   }

  /*
    TODO: fix this shit ! Has to return char*, because id is char*.
    -1 not a sufficient error code. Ideally an std::optional response

    Returns a non-0 value bucket id
    Returns a -1 if the bucket could not be obtained
   */
  int cc_FileHostingSession_getBucket (struct FileHostingSession* _session, char* id) {
    auto session = reinterpret_cast<FileHostingSession*>(_session);
    std::ios::sync_with_stdio();
    std::cout << "cc_FileHostingSession_getBucket" << std::endl;

    FileBucket fbId_instance;
    fbId_instance = std::string(id);
    auto [maybeBucket, maybeItem] = session->hostingService->obtainFileBucket(fbId_instance).get();

    if (!maybeBucket.has_value() || !maybeItem.has_value()) {
      return -1;
    }

    session->bucket = std::move(maybeBucket.value());
    session->registryItem = maybeItem.value();
    return static_cast<int>(session->bucket->id);
  }

  /*!
    Returns a 1 for if the file was successfully obtained
    Returns a 0 if the file could not be obtained
    Returns a -1 if the file exists, but the file cannot be obtained, possibly because the filename was incorrect
  */
  int cc_FileHostingSession_getContentFile (struct FileHostingSession* _session, struct HostedFileInfo* info) {
    auto session = reinterpret_cast<FileHostingSession*>(_session);

    std::ios::sync_with_stdio();
    std::cout << "cc_FileHostingSession_getContentFile" << std::endl;

    StoredFileId fileId;
    fileId = std::string(info->id);

    // NOTE: the session's bucket afterwards
    auto [maybeStoredFile, exists] = session->hostingService->
      obtainStoredFile(session->bucket,
		       fileId,
		       info->fileName).get();

    if (maybeStoredFile.has_value()) {
      session->hostingFile = std::move(maybeStoredFile.value());
      return 1;
    }
    else if (exists) {
      // Move the filebucket back into the registry item
      session->registryItem->fileBucket = std::move(session->bucket);
      return -1;
    }
    // Move the filebucket back into the registry item
    session->registryItem->fileBucket = std::move(session->bucket);
    return 0;
  }

  void cc_FileHostingSession_getChunkingHandle (struct FileHostingSession* _session) {
    auto session = reinterpret_cast<FileHostingSession*>(_session);
    std::ios::sync_with_stdio();

    auto size = session->hostingFile->getRealSize();
    std::cout << "cc_FileHostingSession_chunkFile | size: " << size << std::endl;

    session->cursor = std::make_unique<Utility::ChunkedCursor>(
      32_kB,
      size,
      0,
      [&session](std::ifstream& stream) {
	session->hostingService->hostFile(
	  stream,
	  std::move(session->hostingFile),
	  std::move(session->bucket),
	  session->registryItem);
      });
  }

  /*
    Returns a 0 if the chunking should continue
    Returns a non-0 chunk length if this is the last chunk
  */
  int cc_FileHostingSession_yieldChunk (struct FileHostingSession* _session, unsigned char* cffiResult) {
    std::cout << "cc_FileHostingSession_yieldChunk" << std::endl;

    auto session = reinterpret_cast<FileHostingSession*>(_session);
    session->cursor->nextChunk(cffiResult);

    std::cout << "cc_FileHostingSession_yieldChunk finished | forwardsAmount: " << session->cursor->forwardsAmount << std::endl;
    if (session->cursor->isLastChunk) {
      return session->cursor->forwardsAmount;
    }

    return 0;
  }
}
}
#endif
