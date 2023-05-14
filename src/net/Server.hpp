#ifndef NETWORKING_SERVER_HPP
#define NETWORKING_SERVER_HPP

#include "../pch.h"
#include "Message.hpp"
#include "Connection.hpp"

namespace net {
    using namespace boost;

    class Server {
    public:
        explicit Server(const uint16_t port) :
                _endpoint{asio::ip::tcp::v4(), port},
                _acceptor{_io_context, _endpoint} {

            buffer.resize(MAX_BODY_SIZE);
        }

        virtual ~Server() {
            _io_context.stop();
            if (_context_thread.joinable())
                _context_thread.join();
            std::cout << "[Server] Stopped!\n";
        }

        // TODO: remove later
        void mainLoop() {
            _io_context.run();
//            _context_thread = std::thread([this]() { _io_context.run(); });
            _connection->processIncoming();
        }

        void Start() {
            waitForClients();

            std::cout << "[Server] Started!\n";
        }

        void waitForClients() {
            _acceptor.async_accept(
                    [this](boost::system::error_code ec, asio::ip::tcp::socket socket) {
                        if (!ec) {
                            std::cout << "[Server] New Connection: " << socket.remote_endpoint() << "\n";
                            _connection = std::make_unique<Connection>(std::move(socket), _io_context);
                            _connection->setOnMessageHandler(
                                    [this](const Message &message) { msgHandler(message); });
                            _connection->readHeader();
//                            _connection->processIncoming();
//                            _connections.push_back(std::make_shared<Connection>(std::move(socket), _io_context));
//                            _connections.back()->readHeader();
                        } else {
                            std::clog << "[Server] New Connection Error: " << ec.message() << "\n";
                        }
//                        waitForClients();
                    });
        }

        void msgHandler(const Message &msg) {
            std::clog << msg << std::endl;

            switch (msg.header().msgType()) {
                case MsgType::FileHeader: {
                    std::clog << "[Server] Handling " << to_string(msg.header().msgType()) << std::endl;
                    auto pos = msg.body().rfind('\n');
                    if (pos == std::string::npos) {
                        std::cerr << "[Server] Corrupted File Header" << std::endl;
                    }

                    auto file_size = strtol(msg.body().c_str() + pos, nullptr, 10);
                    packages_to_wait = file_size / MAX_BODY_SIZE + 1;
                    std::string path{_root_dir.path().string() + "\\" +
                                     msg.body().substr(0, pos)};

                    ofs.open(path);
                    if (!ofs.is_open()) {
                        std::cerr << "[Server] Can't open ofstream" << std::endl;
                    }

                    break;
                }
                case MsgType::FileTransfer: {
                    std::clog << "[Server] Handling " << to_string(msg.header().msgType()) << std::endl;
                    if (!ofs.is_open()) {
                        std::cerr << "[Server] Ofstream isn't opened" << std::endl;
                    }
                    ofs << msg.body();
                    --packages_to_wait;
                    if (packages_to_wait <= 0) {
                        std::clog << "[Server] Whole file transfered" << std::endl;
                        ofs.close();
                    }

                    break;
                }
                default: {
                    std::clog << "[Server] Handling " << to_string(msg.header().msgType()) << std::endl;
                    break;
                }
            }
        }

        const filesystem::directory_entry &root() {
            return _root_dir;
        }

        void root(const filesystem::path &root) {
            _root_dir.assign(root);
        }

    private:
        asio::io_context _io_context;
        asio::ip::tcp::endpoint _endpoint;
        asio::ip::tcp::acceptor _acceptor;
        std::thread _context_thread;
//        Connection _connection;
        std::unique_ptr<Connection> _connection;
//        ts_deque<std::shared_ptr<Connection>> _connections;
        size_t packages_to_wait = 0;
        std::ofstream ofs;
        std::string buffer;
        filesystem::directory_entry _root_dir;
    };
}

#endif //NETWORKING_SERVER_HPP