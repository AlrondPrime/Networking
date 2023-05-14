#ifndef NETWORKING_PCH_H
#define NETWORKING_PCH_H

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <string>
#include <string_view>
#include <initializer_list>
#include <regex>
#include <set>
#include <chrono>
#include <functional>
#include <array>
#include <thread>
#include <filesystem>
#include <bitset>
#include <cassert>
#include <deque>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include <boost/filesystem.hpp>
//using namespace boost;
using namespace std::literals::string_literals;

#endif //NETWORKING_PCH_H