auto FileUploadingService::uploadFile(
  std::unique_ptr<FileBucket> bucket,
  std::unique_ptr<FileStorage::StoredFile> tmpFile,
  std::string contentType,
  std::string fileType,
  std::vector<std::string> tags)
-> std::future<std::tuple<FileBucketId, std::string>> {
  // TODO Ask master to store this file
  auto storedFile = bucket->storage->add(std::move(tmpFile));

  // TODO actually make this async
  std::promise<std::tuple<FileBucketId, std::string>> test;

  // Copy id and throw away unique_ptr
  auto const fbId = bucket->id;
  std::unique_lock lock(registry->mutex);
  registry->currentFileBuckets.push_back(std::move(bucket));

  test.set_value({fbId, std::to_string(storedFile->id.value())});
  return test.get_future();
}
