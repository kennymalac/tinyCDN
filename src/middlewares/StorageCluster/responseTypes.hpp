#pragma once

#include "../../hashing.hpp"

namespace TinyCDN::Middleware::StorageCluster {

using namespace TinyCDN::Utility::Hashing;

// TODO each error should have different response
struct StorageClusterErrorResponse {

};

//! Improperly formatted request or missing fields(?)
struct StorageClusterInvalidRequestResponse {
};

using StorageClusterResponse = std::variant<StorageClusterErrorResponse, StorageClusterInvalidRequestResponse>;
using MaybeStorageClusterResponse = std::variant<std::monostate, StorageClusterErrorResponse, StorageClusterInvalidRequestResponse>;

}
