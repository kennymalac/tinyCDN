#include <fstream>
#include <memory>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
#include "master.hpp"
#include "requestTypes.hpp"
#include "requestParser.hpp"
#include "services.hpp"

namespace TinyCDN::Middleware::Master {

bool MasterNode::inspectFileBucket(std::unique_ptr<FileBucket>& fb, Size minimumSize, std::vector<std::string> types) {

  // If this FileBucket has enough free space for this file,
  // TODO master should update filebucket size when upload transaction is approved.
  auto fbAvailableSize = fb->allocatedSize;
  if (fb->size - fbAvailableSize >= minimumSize) {
    // supports the specified ContentTypes,
    auto const notSupportsCtypes = std::any_of(types.cbegin(), types.cend(), [&fb](auto const contentType) {
      return std::find(fb->types.cbegin(), fb->types.cend(), contentType) == fb->types.end();
    });
    //std::cout << notSupportsCtypes << "\n";
    // and supports this file's FileType.
    // && std::find(b.fileTypes.begin(), b.fileTypes.end(), fileType) != b.fileTypes.end()
    if (!notSupportsCtypes) {
      return true;
    }
  }

  return false;
}

std::unique_ptr<MasterFileHostingService> MasterNode::getHostingService() {
  hostingServiceMutex.lock();
  return std::move(hostingService);
}

std::unique_ptr<MasterFileUploadingService> MasterNode::getUploadingService() {
  uploadingServiceMutex.lock();
  return std::move(uploadingService);
}

MasterNode::MasterNode(fs::path location, bool existing) : existing(existing) {
  std::cout << "spawning master, existing: " << existing << " " << "\n";
  registry = std::make_unique<Middleware::File::FileBucketRegistry>(
    fs::current_path(), "REGISTRY");

  if (!existing || !fs::exists("REGISTRY")) {
    // initialize a first-time registry
    std::ofstream registryFile("REGISTRY");
    registryFile << "";
  }
  else {
    registry->loadRegistry();
  }
}


MasterNode* MasterNodeSingleton::initInstance(MasterNodeSingleton singleton, fs::path location, bool existing) {
  static MasterNode newInstance(location, existing);
  singleton.instance = &newInstance;
  return singleton.instance;
}

MasterParams MasterSession::loadConfig(fs::path location) {
  // Marshal config into MasterParams
  // MasterNodeJsonMarshaller marshaller;

  return MasterParams{};
}

void MasterSession::spawn(MasterParams params, bool existing) {
  if (started) {
    throw std::logic_error("MasterSession already started!");
  }
  // Marshal the configuration into a object
  // try {
  //   // singleton.instance = marshaller.getInstance(config);
  //   started = true;
  // }
  // catch (MarshallerException e) {
  //   std::cerr << e.what();
  // }

  // TODO: Networking - initialize TCP server

  // TODO: Networking - Ping storage cluster nodes??? or no

  // Until this point is reached, no request was parsed
  startEventLoop();
}

void MasterSession::startEventLoop() {
  auto parser = MasterRequestParser{};
  // auto visitor = new MasterNodeRequestVisitor;

  // TODO
  // // TODO Separate asynchronous thread
  // while (isRunning) {
  //   auto request = parser.parse(await tcpServer.nextMessage());
  //   queue.push(request)
  // }
  // while (isRunning) {
  // auto maybeRequest = queue.pop();
  // if (maybeRequest.has_value()) {
  //   maybeResponse = receiveRequest(maybeRequest.get())
  // }
}

MasterResponse MasterSession::receiveRequest(MaybeMasterRequest request) {
  //   std::visit([](auto&& request) {
  //     if constexpr(std::is_same_v<decltype(request), MasterStorageNodeAcknowledgmentRequest&>) {
  //       // Acknowledged.
  //       auto [lock, node] = getNode();
  // // Make sure that cluster node id exists, hostname matches, master key is correct
  // // Add StorageClusterNode to the active storage node list
  //	node->doSomething();
  //	lock.unlock();
  //     }
  //  if constexpr(std::is_same_v<decltype(request), std::monostate) {
  //    return MasterResponse{MasterInvalidRequestResponse{}};
  //  }
  //   }, request);

  return MasterResponse{MasterStorageNodeAcknowledgmentResponse{}};
}

void MasterSession::sendStorageClusterRequest(UUID4 id, StorageClusterRequest request) {
  // TODO handle not found case
  // auto socket = storageClusterSockets.get(id);
  // socket.send(getRequestPacket(request));
  // TODO anticipate Response - add response handle to response handling queue
  // In a transaction with multiple request/responses, there is a callback in the event loop for each step.
}


}
