#include <iostream>

#include "master.hpp"

const std::string version = "0.0.1";

int main() {

  // BASIC interface
  std::cout << "TinyCDN v" << version << "\n";
  std::cout << "Commands:" << "\n";
  std::cout << "create for create CDN in current directory" << "\n";
  std::cout << "spawn for hosting existing CDN" << "\n";

  std::cout << "> ";

  std::string command;
  std::cin >> command;

  if (command == "create") {
    std::cout << "Public CDN name: ";
    std::string CDNName;
    std::cin >> CDNName;
    

    std::cout << "Domain/host (example: cdn.example.com): ";
    std::string CDNDomain;
    std::cin >> CDNDomain;

    CDNMaster master(false);
    master.spawnCDN();

    std::cout << "Master spawned. No slaves have been created.";
  }
  else if (command == "spawn") {
    // TODO take in the registry filename
//    //
//    master.spawnCDN();
//    std::cout << "Master spawned.";
  }

  do {
    std::cout << "> ";
    std::cin >> command;

    if (command == "exit") {
      break;
    }
  } while (true);

  return 0;
}
