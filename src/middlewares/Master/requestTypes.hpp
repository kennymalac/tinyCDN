#pragma once

#include "../../hashing.hpp"

namespace TinyCDN::Middleware::Master {

using namespace TinyCDN::Utility::Hashing;

class Success {};
class Error {
  std::string reason;
};

struct MasterStorageNodeAcknowledgmentRequest {
  Id<64> authKey;
  Id<64> storageNodeId;
};

// class MasterRequestInitResponsePacket : MasterRequest {
//   // static std::string name = "INIT_RESPONSE";
//   std::variant<Success, Error> callerStatus;
// };

using MasterRequest = std::variant<MasterStorageNodeAcknowledgmentRequest>;
using MaybeMasterRequest = std::variant<std::monostate, MasterStorageNodeAcknowledgmentRequest>;

}
