#pragma once

#include "requestTypes.hpp"

namespace TinyCDN::Middleware::Master {

class MasterRequestParser {
public:
  MaybeMasterRequest request;
  //! Parses Packet type and deduces which Command needs to execute
  MaybeMasterRequest parse(std::string contents) {
    // packetType = "StorageNodeAcknowledgment";
    // requestBody = "<NodeId> <AcknowledgementKey>";

    // if (packetType == "Acknowledgment") {
    //   request = AcknowledgmentRequest{body};
    // }

    // return packet;
    return MaybeMasterRequest{};
  }

  MasterRequestParser() = default;
};

std::string getRequestPacket(MasterRequest request) {
  //   std::visit([](auto&& request) {
  //     if constexpr(std::is_same_v<decltype(request), MasterStorageNodeAcknowledgmentRequest&>) {
  // return packet;

  return "";
}

}
