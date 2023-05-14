#ifndef NETWORKING_CLIENT_HPP
#define NETWORKING_CLIENT_HPP

#include "../pch.h"
#include "Message.hpp"
#include "Connection.hpp"

namespace net {
    using namespace boost;

    class Client {
    public:
        Client() :
                _connection(asio::ip::tcp::socket(_io_context), _io_context) {

            _connection.setOnMessageHandler(
                    [this](const Message &message) { msgHandler(message); });
        }

        ~Client() {
            _connection.disconnect();
            _io_context.stop();
            if (_context_thread.joinable())
                _context_thread.join();
            std::clog << "[Client] Disconnected!\n";
        }

        void connectToServer(const std::string &host, const uint16_t port) {
            asio::ip::tcp::resolver resolver(_io_context);
            _endpoints = resolver.resolve(host, std::to_string(port));

            asio::async_connect(_connection.socket(), _endpoints,
                                [this](std::error_code ec, const asio::ip::tcp::endpoint &endpoint) {
                                    if (!ec) {
                                        std::clog << "[Client] Connected to Server!\n";
//                                        _connection.readHeader();
                                    }
                                });
        }

        // TODO: remove later
        void mainLoop() {
            _io_context.run();
//          _context_thread = std::thread([this]() { _io_context.run(); });

        }

        void sendMsg(const Message &msg) {
            _connection.sendMsg(msg);
        }

        void sendFile(const boost::filesystem::path &path) {
            _connection.sendFile(_root_dir / path);
        }

        void msgHandler(const Message &msg) {
            std::clog << "[Client]" << msg << std::endl;
        }

        const filesystem::directory_entry &root() {
            return _root_dir;
        }

        void root(const filesystem::path &root) {
            _root_dir.assign(root);
        }

    private:
        asio::io_context _io_context;
        asio::ip::tcp::resolver::results_type _endpoints;
        std::thread _context_thread;
        Connection _connection;
        filesystem::directory_entry _root_dir;
    };
}

#endif //NETWORKING_CLIENT_HPP