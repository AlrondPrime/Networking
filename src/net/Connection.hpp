#ifndef NETWORKING_CONNECTION_HPP
#define NETWORKING_CONNECTION_HPP

#include <utility>

#include "../pch.h"
#include "ts_deque.hpp"

namespace net {
    using namespace boost;

    class Connection :
            public std::enable_shared_from_this<Connection> {
    public:
        explicit Connection(asio::ip::tcp::socket socket, asio::io_context &io_context) :
                _socket(std::move(socket)), _io_context(io_context) {
        }

        bool connected() const {
            return _socket.is_open();
        }

        /// @details Asynchronous function
        void disconnect() {
            if (_socket.is_open()) {
                asio::post(_io_context, [this]() { _socket.close(); });
            }
        }

        asio::ip::tcp::socket &socket() {
            return _socket;
        }

        /// @details Asynchronous function
        void sendMsg(const Message &msg) {
            asio::post(_io_context,
                       [this, msg]() {

                           bool writeInProcess = !_msg_queue_out.empty();
                           _msg_queue_out.push_back(msg);
                           if (!writeInProcess)
                               writeHeader();
                       });
        }

        /// @details Asynchronous function
        void sendMsg(Message &&msg) {
            asio::post(_io_context,
                       [this, msg]() {
                           bool writeInProcess = !_msg_queue_out.empty();
                           _msg_queue_out.push_back(msg);
                           if (!writeInProcess)
                               writeHeader();
                       });
        }

        void sendFile(const boost::filesystem::path &path) {
            if (!exists(path)) {
                std::cerr << "[Connection] File is not found" << std::endl;
                return;
            }

            writeFileHeader(path.string());
        }

        void writeFileHeader(const boost::filesystem::path &path) {
            auto file_size = boost::filesystem::file_size(path);
            std::string body{path.filename().string() + "\n" + std::to_string(file_size)};
            Message msg{Message::MessageHeader{MsgType::FileHeader, MAX_BODY_SIZE},
                        std::move(body)};
            sendMsg(std::move(msg));

            writeFileBody(path);
        }

        void writeFileBody(const boost::filesystem::path &path) {
            std::ifstream ifs{path.string()};

            if (!ifs.is_open()) {
                std::cerr << "[Connection] File cannot be opened." << std::endl;
                return;
            }

            std::string buffer;
            buffer.resize(MAX_BODY_SIZE);
            Message msg{Message::MessageHeader{MsgType::FileTransfer, MAX_BODY_SIZE}};

            while (ifs) {
                ifs.read(buffer.data(), MAX_BODY_SIZE);
                msg.body() = buffer;
                std::fill(buffer.begin(), buffer.end(), '\0');
                sendMsg(msg);
            }
        }

        /// @details Asynchronous function
        void readHeader() {
            asio::async_read(_socket,
                             asio::buffer(&_tempMsgIn.header(), net::HEADER_SIZE),
                             [this](system::error_code ec, std::size_t length) {
                                 if (!ec) {
                                     std::clog << "[Connection] Read Header Done.\n";
                                     if (_tempMsgIn.bodyLength() > 0) {
                                         _tempMsgIn.resize(_tempMsgIn.bodyLength());
                                         readBody();
                                     } else {
                                         _msg_queue_in.push_back(_tempMsgIn);
                                         readHeader();
                                     }
                                 } else {
                                     std::clog << "[Connection] Read Header Fail.\n";
                                     _socket.close();
                                 }
                             });
        }

        /// @details Asynchronous function
        void readBody() {
            asio::async_read(_socket,
                             asio::buffer(_tempMsgIn.data(), _tempMsgIn.bodyLength()),
                             [this](system::error_code ec, std::size_t length) {
                                 if (!ec) {
                                     std::clog << "[Connection] Read Body Done.\n";
                                     _msg_queue_in.push_back(_tempMsgIn);
                                     readHeader();
                                 } else {
                                     std::clog << "[Connection] Read Body Fail.\n";
                                     _socket.close();
                                 }
                             });
        }

        /// @details Asynchronous function
        void writeHeader() {
            asio::async_write(_socket,
                              asio::buffer(&_msg_queue_out.front().header(),
                                           net::HEADER_SIZE),
                              [this](system::error_code ec, std::size_t length) {
                                  if (!ec) {
//                                      std::clog << "[Connection] Write Header Done.\n";
                                      std::clog << "[Connection] Write Header Done"
                                                << " with length = " << length << ".\n";
                                      if (_msg_queue_out.front().bodyLength() > 0) {
                                          writeBody();
                                      } else {
                                          _msg_queue_out.pop_front();
                                          if (!_msg_queue_out.empty())
                                              writeHeader();
                                      }
                                  } else {
                                      std::clog << "[Connection] Write Header Fail.\n";
                                      _socket.close();
                                  }
                              });
        }

        /// @details Asynchronous function
        void writeBody() {
            asio::async_write(_socket,
                              asio::buffer(_msg_queue_out.front().data(),
                                           _msg_queue_out.front().bodyLength()),
                              [this](system::error_code ec, std::size_t length) {
                                  if (!ec) {
//                                      std::clog << "[Connection] Write Body Done.\n";
                                      std::clog << "[Connection] Write Body Done"
                                                << " with length = " << length << ".\n";
                                      _msg_queue_out.pop_front();
                                      if (!_msg_queue_out.empty())
                                          writeHeader();
                                  } else {
                                      std::clog << "[Connection] Write Body Fail.\n";
                                      _socket.close();
                                  }
                              });
        }

        void setOnMessageHandler(std::function<void(const Message &)> onMessageHandler) {
            _onMessageHandler = std::move(onMessageHandler);
        }

        void resetOnMessageHandler() {
            _onMessageHandler = nullptr;
        }

        [[noreturn]] void processIncoming() {
//            _connections.wait();
            _msg_queue_in.wait();
            while (true)
//                if (!_connections.back()->incoming().empty()) {
                if (!_msg_queue_in.empty()) {
//                    auto msg = _connections.back()->incoming().pop_front();
                    auto msg = _msg_queue_in.pop_front();

                    if (_onMessageHandler) {
                        _onMessageHandler(msg);
                    } else
                        std::clog << "[Connection] Got no handler" << std::endl;
                }
        }

    private:
        ts_deque<Message> _msg_queue_in;
        ts_deque<Message> _msg_queue_out;
        Message _tempMsgIn;
        asio::ip::tcp::socket _socket;
        asio::io_context &_io_context;
        std::function<void(const Message &)> _onMessageHandler;
    };
}

#endif //NETWORKING_CONNECTION_HPP