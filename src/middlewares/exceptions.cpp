#include "middlewares/exceptions.hpp"
#include "middlewares/file.hpp"

FileBucketException::FileBucketException(const FileBucket &fb, int code) : std::runtime_error(""), filebucket(&fb), code(code)
{}

FileBucketRegistryException::FileBucketRegistryException(FileBucketRegistry &fbr, int code, std::string explanation) : std::runtime_error(""), registry(&fbr), code(code), explanation(explanation)
{}
