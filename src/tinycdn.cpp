#include "pistache/pistache.h"
#include "pistache/endpoint.h"
#include "http.h"
#include "description.h"
#include "endpoint.h"
#include "serializer/rapidjson.h"


using namespace Net;


class CDNService {
public:
  CDNService(Net::Address addr)
    : httpEndpoint(std::make_shared<Http::Endpoint>(addr)),
      desc("tinyCDN", "0.1")
  { }

  void init(size_t threads = 2) {
    auto options = Http::Endpoint::options()
      .threads(threads)
      .flags(Tcp::Options::InstallSignalHandler);

    httpEndpoint->init(options);
    createDescription();
  }

  void start() {
    router
  }

private:
  void createDescription() {
    desc
      .info()
      .license("AGPLv3", "https://www.gnu.org/licenses/agpl-3.0.en.html");

    desc
      .schemes(Rest::Scheme::Http)
      .basePath("/v1")
      .consumes(MIME(Multipart, ))
      .produces(MIME(Application, Json))
  }
}

class CDNSlave : public Http::Handler {
public:
  HTTP_PROTOTYPE(CDNSlave)

  void onRequest(auto request, auto response) {
    // response.send(Http::Code::Ok, "hello, world!");
    if (request.method == Http::Method::Get) {
      Http::serveFile()
    }

    // Returns key for file
  }
};

int main(int argc, char *argv[]) {
  Net::Port port(4241);

  std::shared_ptr<Http::Router> router;

  

  Http::listenAndServe<CDNSlave>("*:9080");
}
