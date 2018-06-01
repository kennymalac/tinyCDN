#pragma once

#include <iostream>
#include <optional>
#include <exception>
#include <stdexcept>
#include <sstream>
#include <string>

#include "FileStorage/storedfile.hpp"

namespace TinyCDN::Middleware::File {
struct FileBucket;
struct FileBucketRegistry;

namespace Storage = TinyCDN::Middleware::FileStorage;

void logStoredFile(const std::optional<Storage::StoredFile>& maybeStoredFile, std::ostream& errorLog) {
  auto const file = maybeStoredFile.value();
  errorLog << "id: " << file.id.value_or(0) << "\n";
  errorLog << "location: " << file.location << "\n";
  errorLog << "temporary: " << file.temporary << "\n";
  errorLog << "size: " << file.size.size << "\n";
}

class FileStorageException : public std::runtime_error {
public:

  FileStorageException(int code, std::string explanation, const std::optional<Storage::StoredFile> storedFile);

  virtual const char* what() {
    errorResponse.str("");

    std::string error;
    std::ostringstream errorLog;

    switch (code) {
    case 0:
      errorLog << "invalid StoredFile location or broken file: \n";
      logStoredFile(storedFile, errorLog);
      break;
    case 1:
      error = "invalid persisted filename";
      break;
    default:
      error = "";
      break;
    }

    errorResponse << std::runtime_error::what() << ": " << errorLog.str() << "\n";
    return errorResponse.str().c_str();
  }

private:
  int code;
  const std::optional<Storage::StoredFile> storedFile;

  static std::ostringstream errorResponse;
};

class FileBucketException : public std::runtime_error {
public:

  FileBucketException(const FileBucket &fb, int code, std::string explanation);

  virtual const char* what() {
    errorResponse.str("");

    std::ostringstream errorLog;

    switch (code) {
    case 0:
      errorLog << "FileBucket ran out of space";
      break;
    case 1:
      errorLog << "invalid persisted filename";
      break;
    default:
      errorLog << "";
      break;
    }

    errorResponse << std::runtime_error::what() << ": " << errorLog.str() << "\n";
    return errorResponse.str().c_str();
  }

private:
  int code;
  const FileBucket& fileBucket;

  static std::ostringstream errorResponse;
};

class FileBucketRegistryException : public std::runtime_error {
public:

  FileBucketRegistryException(const FileBucketRegistry &fbr, int code, std::string explanation);

  virtual const char* what() {
    errorResponse.str("");

    std::string error;
    switch (code) {
    case 0:
      error = "loading registry file failed!";
      break;
    default:
      error = "";
      break;
    }

    errorResponse << std::runtime_error::what() << ": " << error << "\n";
    return errorResponse.str().c_str();
  }

private:
  int code;
  const FileBucketRegistry& registry;

  static std::ostringstream errorResponse;
};
}
