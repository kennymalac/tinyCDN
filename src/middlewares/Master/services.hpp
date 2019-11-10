#pragma once

#include <experimental/filesystem>
#include <memory>
#include <mutex>
#include <shared_mutex>
namespace fs = std::experimental::filesystem;

#include "../file.hpp"

namespace TinyCDN::Middleware::Master {
using namespace Middleware::File;

struct MasterFileHostingService {
};

struct MasterFileUploadingService {
};


}
