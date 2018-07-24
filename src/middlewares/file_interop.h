#include "../master.hpp"
#ifdef __cplusplus
#include <memory>
#include "file.hpp"
#include "FileStorage/storedfile.hpp"
#endif

#ifdef __cplusplus
namespace TinyCDN {
extern "C" {
  struct FileUploadingSession {
    std::unique_ptr<Middleware::File::FileBucket> bucket;
    std::unique_ptr<Middleware::FileStorage::StoredFile> uploadingFile;
    std::unique_ptr<Middleware::File::FileUploadingService> uploadService;
  };
#else
typedef struct FileUploadingSession FileUploadingSession;
#endif

  struct FileUploadInfo {
    char* temporaryLocation;
    char* contentType;
    char* fileType;
    char* tags;
    int wantsOwned;
  };

  struct FileUploadingSession* FileUploadingSession_new ();
  void FileUploadingSession_delete (struct FileUploadingSession* session);
  int FileUploadingSession_getBucket (struct FileUploadingSession* session, struct FileUploadInfo* info);
  void FileUploadingSession_finishFileUpload (struct FileUploadingSession* session, const char* cffiResult[], struct FileUploadInfo* info);
#ifdef __cplusplus
}
}
#endif
