#pragma once

#include <iostream>
#include <optional>
#include <exception>
#include <stdexcept>
#include <sstream>
#include <string>
#include <stdexcept>

#include "FileStorage/storedfile.hpp"
#include "../hashing.hpp"

namespace TinyCDN::Utility::Exceptions {
class NotImplementedException : public std::logic_error
{
public:
  NotImplementedException() : std::logic_error("Function not yet implemented.") {}
};
}

// TODO migrate namespace
namespace TinyCDN::Middleware::File {
using namespace TinyCDN::Utility::Hashing;

struct FileBucket;
struct FileBucketRegistry;
struct FileBucketRegistryItem;

namespace Storage = TinyCDN::Middleware::FileStorage;

inline void logStoredFile(const std::optional<Storage::StoredFile>& maybeStoredFile, std::ostream& errorLog) {
  if (!maybeStoredFile.has_value()) {
    return;
  }
  auto const file = maybeStoredFile.value();
  errorLog << "id: " << file.id.value_or(Id<64>{std::string{""}}) << "\n";
  errorLog << "location: " << file.location << "\n";
  errorLog << "temporary: " << file.temporary << "\n";
  errorLog << "size: " << file.size << "\n";
}

class FileStorageException : public std::runtime_error {
public:

  FileStorageException(int code, std::string explanation, const Storage::StoredFile storedFile);
  FileStorageException(int code, std::string explanation, const std::optional<Storage::StoredFile> storedFile);
  FileStorageException(int code, std::string explanation);

  virtual const char* what() {
    std::ostringstream errorLog;

    errorLog << std::runtime_error::what() << ": ";

    // TODO make this an enum
    switch (code) {
    case 0:
      errorLog << "invalid StoredFile location or broken file: \n";
      logStoredFile(storedFile, errorLog);
      break;
    case 1:
      errorLog << "invalid persisted filename";
      break;
    case 2:
      errorLog << "Loading from persisted state failed";
      break;
    case 3:
      errorLog << "Allocation step failed";
      break;
    case 4:
      errorLog << "improper StoredFile access: \n";
    default:
      errorLog << "";
      break;
    }

    return errorLog.str().c_str();
  }

private:
  int code;
  const std::optional<Storage::StoredFile> storedFile;
};

class FileBucketException : public std::runtime_error {
public:

  FileBucketException(const FileBucket &fb, int code, std::string explanation);

  virtual const char* what() {
    std::ostringstream errorLog;

    errorLog << std::runtime_error::what() << ": ";

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

    return errorLog.str().c_str();
  }

private:
  int code;
  const FileBucket& fileBucket;
};


class FileBucketRegistryItemException : public std::runtime_error {
public:

  FileBucketRegistryItemException(const FileBucketRegistryItem &fbri, int code, std::string explanation);

  virtual const char* what() {
    std::ostringstream errorLog;

    errorLog << std::runtime_error::what() << ": ";

    switch (code) {
    case 1:
      errorLog << "Conversion step failed to convert field";
      break;
    default:
      errorLog << "";
      break;
    }

    return errorLog.str().c_str();
  }

private:
  int code;
  const FileBucketRegistryItem& item;

};

class FileBucketRegistryException : public std::runtime_error {
public:

  FileBucketRegistryException(const FileBucketRegistry &fbr, int code, std::string explanation);

  virtual const char* what() {
    std::ostringstream errorLog;

    errorLog << std::runtime_error::what() << ": ";

    switch (code) {
    case 0:
      errorLog << "loading registry file failed!";
      break;
    case 2:
      errorLog << "loading from persistent state failed";
      break;
    default:
      errorLog << "";
      break;
    }

    return errorLog.str().c_str();
  }

private:
  int code;
  const FileBucketRegistry& registry;
};
}
