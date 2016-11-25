#ifndef GEO_NETWORK_CLIENT_COMMUNICATOR_H
#define GEO_NETWORK_CLIENT_COMMUNICATOR_H

#include "internal/OutgoingMessagesHandler.h"
#include "internal/IncomingMessagesHandler.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <string>
#include <stdlib.h>


using namespace std;

namespace as = boost::asio;
namespace ip = boost::asio::ip;


class Communicator {
public:
    explicit Communicator(as::io_service &ioService,
                          const string &interface, const uint16_t port);

    ~Communicator();

    void beginAcceptMessages();

private:
    void asyncReceiveData();

    void handleReceivedInfo(const boost::system::error_code &error, size_t bytesTransferred);

    void sendData();

    void handleSentInfo(const boost::system::error_code &error, size_t bytesTransferred);

private:
    static constexpr const uint16_t kMaxIncomingBufferSize = 1024;

private:
    as::io_service &mIOService;
    udp::socket *mSocket;

    boost::array<char, kMaxIncomingBufferSize> mRecvBuffer;
    udp::endpoint mRemoteEndpointBuffer;

    IncomingMessagesHandler *mIncomingMessagesHandler;
    OutgoingMessagesHandler *mOutgoingMessagesHandler;
};


#endif //GEO_NETWORK_CLIENT_COMMUNICATOR_H
