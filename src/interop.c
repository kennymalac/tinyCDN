#include "middlewares/file_interop.h"
#include <stdio.h>

struct FileUploadInfo* FileUploadInfo_new (char* temporaryLocation,
                                           char* contentType,
                                           char* fileType,
                                           char* tags,
                                           int wantsOwned) {
  return cc_FileUploadInfo_new(temporaryLocation, contentType, fileType, tags, wantsOwned);
};

struct FileUploadingSession* FileUploadingSession_new () {
  return cc_FileUploadingSession_new();
};

void FileUploadingSession_delete (struct FileUploadingSession* session) {
  return cc_FileUploadingSession_delete(session);
};

int FileUploadingSession_getBucket (struct FileUploadingSession* session, struct FileUploadInfo* info) {
  return cc_FileUploadingSession_getBucket(session, info);
};

void FileUploadingSession_finishFileUpload (struct FileUploadingSession* session, char* cffiResult[], struct FileUploadInfo* info) {
  return cc_FileUploadingSession_finishFileUpload(session, cffiResult, info);
};

int main() {
  struct FileUploadingSession* session = FileUploadingSession_new();
  char* cffiResult[100];
  char* tags = {"tag"};
  struct FileUploadInfo* info = FileUploadInfo_new ("/home/ken/Programming/cl/simple-content-host/src/program/tmp/aloepizza.jpg",
                                                    "image",
                                                    "image/jpg",
                                                    tags,
                                                    0);
  int bucketId = FileUploadingSession_getBucket(session, info);
  FileUploadingSession_finishFileUpload(session, cffiResult, info);

  printf("%d", bucketId);
}
