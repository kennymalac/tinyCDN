#include "middlewares/file_interop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct FileUploadInfo* FileUploadInfo_new (char* temporaryLocation,
                                           char* contentType,
                                           char* fileType,
                                           char* tags,
                                           int wantsOwned) {
  return cc_FileUploadInfo_new(temporaryLocation, contentType, fileType, tags, wantsOwned);
};

struct HostedFileInfo* HostedFileInfo_new (int id,
                                           char* fName,
                                           int owned) {
  return cc_HostedFileInfo_new(id, fName, owned);
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

int FileHostingSession_yieldChunk (struct FileHostingSession* session, unsigned char* cffiResult) {
  return cc_FileHostingSession_yieldChunk(session, cffiResult);
};


void testUpload(int arr[2]) {
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
  FileUploadingSession_delete(session);

  arr[0] = atoi(cffiResult[0]);
  arr[1] = atoi(cffiResult[1]);
}

void testHost(int bucketId, int fileId) {
  printf("bucketId: %d | fileId: %d", bucketId, fileId);
  fflush(stdout);
  struct FileHostingSession* session = FileHostingSession_new();

  struct HostedFileInfo* info = HostedFileInfo_new (fileId, "aloepizza.jpg", 0);

  FileHostingSession_getBucket(session, bucketId);
  FileHostingSession_getContentFile(session, info);
  FileHostingSession_getChunkingHandle(session);

  unsigned char buffer[32768+10390];
  int shouldContinue = FileHostingSession_yieldChunk(session, buffer);
  printf("should continue?: %d", shouldContinue);

  unsigned char buffer2[10390];
  int shouldContinue2 = FileHostingSession_yieldChunk(session, buffer2);
  printf("should continue 2?: %d", shouldContinue2);

  memcpy(buffer+32768, buffer2, 10390);

  // Output copy to file
  FILE* test = fopen("test.jpg", "wb");
  fwrite(&buffer, sizeof(buffer[0]), sizeof(buffer)/sizeof(buffer[0]), test);
  fclose(test);

  FileHostingSession_delete(session);
}

int main() {
  int cffiResult[2];
  testUpload(cffiResult);
  testHost(cffiResult[0], cffiResult[1]);
}
