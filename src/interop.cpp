#ifdef __cplusplus
#include "middlewares/file_interop.h"
#include "master.hpp"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <memory>
#include "utility.hpp"
#include "middlewares/file.hpp"

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
  struct FileUploadInfo* cc_FileUploadInfo_new (char* temporaryLocation,
                                                char* contentType,
                                                char* fileType,
                                                char* tags,
                                                int wantsOwned) {
    auto* info = new FileUploadInfo(temporaryLocation, contentType, fileType, tags, wantsOwned);
    std::ios::sync_with_stdio();
    std::cout << "cc_FileUploadInfo_new | temporaryLocation: " << info->temporaryLocation << " " << "contentType: " << info->contentType << " " << "fileType: " << info->fileType << std::endl;

    return info;
  }


  FileUploadingSession* cc_FileUploadingSession_new () {
    auto* master = new CDNMasterSingleton;
    auto* session = new FileUploadingSession;

    std::ios::sync_with_stdio();
    std::cout << "cc_FileUploadingSession_new" << std::endl;

    session->uploadService = std::make_unique<Middleware::File::FileUploadingService>(master->getInstance(true, true)->session->registry);

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

  int cc_FileUploadingSession_getBucket (struct FileUploadingSession* _session, FileUploadInfo* info) {
    auto session = reinterpret_cast<FileUploadingSession*>(_session);
    std::ios::sync_with_stdio();
    std::cout << "cc_FileUploadingSession_getBucket" << std::endl;

    auto [temporaryLocation, contentType, fileType, tags, wantsOwned] = getUploadInfo(reinterpret_cast<FileUploadInfo*>(info));

    session->uploadingFile = std::make_unique<Middleware::FileStorage::StoredFile>(temporaryLocation, true);

    session->bucket = session->uploadService->requestFileBucket(session->uploadingFile, contentType, fileType, tags, wantsOwned).get();

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

    auto [temporaryLocation, contentType, fileType, tags, wantsOwned] = getUploadInfo(info);

    std::cout << "starting upload" << std::endl;
    auto _result = session->uploadService->uploadFile(std::move(session->bucket), std::move(session->uploadingFile), contentType, fileType, tags);
    std::cout << "waiting.." << std::endl;
    _result.wait();

    auto result = _result.get();
    std::cout << "upload done, returning" << std::endl;

    // C-ify the result

    auto const [_fbId, _storedFileId] = result;
    auto const fbId = std::to_string(_fbId).c_str();
    auto const storedFileId = _storedFileId.c_str();

    cffiResult[0] = new char [sizeof(fbId)];
    strcpy(cffiResult[0], fbId);
    cffiResult[1] = new char [sizeof(storedFileId)];
    strcpy(cffiResult[1], storedFileId);
  }

}
}
#endif
