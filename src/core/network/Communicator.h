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

typedef boost::system::error_code err;


class Communicator {
public:
    explicit Communicator(
        as::io_service &ioService,
        const string &interface,
        const uint16_t port);

    ~Communicator();

    void beginAcceptMessages();

private:
    static constexpr const size_t kMaxIncomingBufferSize = 1024;

private:
    as::io_service &mIOService;
    udp::socket *mSocket;

    boost::array<char, kMaxIncomingBufferSize> mRecvBuffer;
    udp::endpoint mRemoteEndpointBuffer;

    IncomingMessagesHandler *mIncomingMessagesHandler;
    OutgoingMessagesHandler *mOutgoingMessagesHandler;

private:
    // todo: move to the incoming handler
    void asyncReceiveData();
    void handleReceivedInfo(
        const boost::system::error_code &error,
        size_t bytesTransferred);

    // todo: move to the outgoing messages handler
    // todo: review this method.
    // why it accepts buffer as parameter?
    // is it normal?
    void sendData(
        as::mutable_buffer buffer,
        size_t bufferSize,
        pair <string, uint16_t> address);

    void handleSentInfo(
        const boost::system::error_code &error,
        size_t bytesTransferred);
};


#endif //GEO_NETWORK_CLIENT_COMMUNICATOR_H
