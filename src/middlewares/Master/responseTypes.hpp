#pragma once

#include "../../hashing.hpp"

namespace TinyCDN::Middleware::Master {

using namespace TinyCDN::Utility::Hashing;

struct MasterStorageNodeAcknowledgmentResponse {
  Id<64> authKey;
};

// TODO each error should have different response
struct MasterErrorResponse {

};

//! Improperly formatted request or missing fields(?)
struct MasterInvalidRequestResponse {
};

// class MasterResponseInitResponsePacket : MasterResponse {
//   // static std::string name = "INIT_RESPONSE";
//   std::variant<Success, Error> callerStatus;
// };

using MasterResponse = std::variant<MasterStorageNodeAcknowledgmentResponse, MasterErrorResponse, MasterInvalidRequestResponse>;
using MaybeMasterResponse = std::variant<std::monostate, MasterStorageNodeAcknowledgmentResponse, MasterErrorResponse, MasterInvalidRequestResponse>;

}
