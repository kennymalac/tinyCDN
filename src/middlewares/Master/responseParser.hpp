#pragma once

#include "responseTypes.hpp"

namespace TinyCDN::Middleware::Master {

class MasterResponseParser {
public:
  MaybeMasterResponse response;
  //! Parses Packet type and deduces which Command needs to execute
  MaybeMasterResponse parse(std::string contents) {
    // packetType = "StorageNodeAcknowledgment";
    // responseBody = "<NodeId> <AcknowledgementKey>";

    // if (packetType == "Acknowledgment") {
    //   response = AcknowledgmentResponse{body};
    // }

    // return packet;
    return MaybeMasterResponse{};
  }

  MasterResponseParser() = default;
};

}
