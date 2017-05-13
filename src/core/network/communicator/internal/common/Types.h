#ifndef NETWORK_COMMON_TYPES_H
#define NETWORK_COMMON_TYPES_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>

#include <string>
#include <chrono>


using IOService = boost::asio::io_service;

using UDPSocket = boost::asio::ip::udp::socket;
using UDPEndpoint = boost::asio::ip::udp::endpoint;

using TCPSocket = boost::asio::ip::tcp::socket;
using TCPEndpoint = boost::asio::ip::tcp::endpoint;
using TCPResolver = boost::asio::ip::tcp::resolver;

namespace signals = boost::signals2;


using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;


using Host = std::string;
using Port = uint16_t;

#endif // NETWORK_COMMON_TYPES_H
