#include <string>

#include "utility.hpp"
#include "middlewares/file_interop.h"
#include "middlewares/file.hpp"
#include "master.hpp"

namespace TinyCDN {

auto getUploadInfo(FileUploadInfo* _info) {
  auto info = reinterpret_cast<FileUploadInfo*>(_info);

  std::string temporaryLocation(info->temporaryLocation);
  std::string contentType(info->contentType);
  std::string fileType(info->fileType);
  // TODO tags
  std::vector<std::string> tags;
  bool wantsOwned(info->wantsOwned);

  return std::make_tuple(temporaryLocation, contentType, fileType, tags, wantsOwned);
}

extern "C" {
  // enum ContentType {
  //   Image,
  //   Video
  // };

  // struct Session {
  // };


  FileUploadingSession* FileUploadingSession_new () {
    auto* master = new CDNMasterSingleton;
    auto* session = new FileUploadingSession;

    session->uploadService = std::make_unique<Middleware::File::FileUploadingService>(master->getInstance(true)->session->registry);

    return reinterpret_cast<FileUploadingSession*>(session);
  }

  // FileUploadingSession* requestFileUploadingSession (SessionProvisioner* master) {
  //   return reinterpret_cast<SessionProvisioner*>(master)->getUploadingSession();
  // }

  void FileUploadingSession_delete(FileUploadingSession* session) {
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

  int FileUploadingSession_getBucket (struct FileUploadingSession* _session, FileUploadInfo* info) {
    auto session = reinterpret_cast<FileUploadingSession*>(_session);

    auto [temporaryLocation, contentType, fileType, tags, wantsOwned] = getUploadInfo(info);

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

  void FileUploadingSession_finishFileUpload (FileUploadingSession* _session, const char* cffiResult[], FileUploadInfo* info) {
    // uploadService
    auto session = reinterpret_cast<FileUploadingSession*>(_session);

    auto [temporaryLocation, contentType, fileType, tags, wantsOwned] = getUploadInfo(info);

    auto _result = session->uploadService->uploadFile(std::move(session->bucket), std::move(session->uploadingFile), contentType, fileType, tags);
    _result.wait();

    auto result = _result.get();
    // C-ify the result
    cffiResult[0] = std::to_string(std::get<0>(result)).c_str();
    cffiResult[1] = std::get<1>(result).c_str();
  }
}
}
