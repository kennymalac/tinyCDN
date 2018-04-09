#pragma once

#include <iostream>
#include <exception>
#include <stdexcept>
#include <sstream>
#include <string>

namespace TinyCDN::Middleware::File {
struct FileBucket;
struct FileBucketRegistry;

class FileBucketException : public std::runtime_error {
public:

  FileBucketException(const FileBucket &fb, int code);

  virtual const char* what() const throw() {
    errorResponse.str("");

    std::string error;
    switch (code) {
    case 0:
      error = "filebucket storage has ran out of space";
      break;
    case 1:
      error = "invalid persisted filename";
      break;
    default:
      error = "";
      break;
    }

    errorResponse << std::runtime_error::what() << ": " << error;
    return errorResponse.str().c_str();
  }

private:
  int code;
  FileBucket& filebucket;

  static std::ostringstream errorResponse;
};

class FileBucketRegistryException : public std::runtime_error {
public:

  FileBucketRegistryException(FileBucketRegistry &fbr, int code, std::string explanation);

  virtual const char* what() const throw() {
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

    errorResponse << std::runtime_error::what() << ": " << error << " " << explanation;
    return errorResponse.str().c_str();
  }

private:
  int code;
  std::string explanation;
  FileBucketRegistry& registry;

  static std::ostringstream errorResponse;
};
}
