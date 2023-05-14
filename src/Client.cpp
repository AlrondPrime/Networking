#include "net/Client.hpp"
#include "net/Message.hpp"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: Client.exe <path to folder that will be used as client root>\n";
        return 1;
    }
    std::string host{"localhost"};
    uint16_t port{60000};
    std::string root_dir{argv[1]};
    /*if (argc != 3) {
        std::cerr << "Usage: SimpleClient.exe <host> <port>\n";
        return 1;
    }
    uint16_t port{static_cast<uint16_t>(strtol(argv[2], nullptr, 10))};*/

    net::Client client;
    client.root(root_dir);
    client.connectToServer(host, port);

    client.sendFile("Data.txt");
    client.mainLoop();

    return 0;
}