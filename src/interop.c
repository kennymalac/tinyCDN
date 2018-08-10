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

int FileUploadingSession_fetchBucket (struct FileUploadingSession* session, struct FileUploadInfo* info) {
  return cc_FileUploadingSession_fetchBucket(session, info);
};

void FileUploadingSession_finishFileUpload (struct FileUploadingSession* session, char* cffiResult[], struct FileUploadInfo* info) {
  return cc_FileUploadingSession_finishFileUpload(session, cffiResult, info);
};

struct FileHostingSession* FileHostingSession_new () {
  return cc_FileHostingSession_new();
};
void FileHostingSession_delete (struct FileHostingSession* session) {
  return cc_FileHostingSession_delete(session);
};
int FileHostingSession_getBucket (struct FileHostingSession* session, int id) {
  return cc_FileHostingSession_getBucket(session, id);
};

int FileHostingSession_getContentFile (struct FileHostingSession* _session, struct HostedFileInfo* info) {
  return cc_FileHostingSession_getContentFile(_session, info);
};

void FileHostingSession_getChunkingHandle (struct FileHostingSession* session) {
  cc_FileHostingSession_getChunkingHandle(session);
};

void FileHostingSession_yieldChunk (struct FileHostingSession* session, char* cffiResult[]) {
  cc_FileHostingSession_yieldChunk(session, cffiResult);
};


void testUpload() {
  struct FileUploadingSession* session = FileUploadingSession_new();
  char* cffiResult[100];
  char* tags = {"tag"};
  struct FileUploadInfo* info = FileUploadInfo_new ("/home/ken/Programming/cl/simple-content-host/src/program/tmp/aloepizza.jpg",
                                                    "image",
                                                    "image/jpg",
                                                    tags,
                                                    0);
  int bucketId = FileUploadingSession_fetchBucket(session, info);
  FileUploadingSession_finishFileUpload(session, cffiResult, info);

  printf("%d", bucketId);
}

int main() {
  testUpload();
}
