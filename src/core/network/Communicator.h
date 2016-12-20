#ifndef GEO_NETWORK_CLIENT_COMMUNICATOR_H
#define GEO_NETWORK_CLIENT_COMMUNICATOR_H

#include "UUID2Address.h"
#include "internal/OutgoingMessagesHandler.h"
#include "internal/IncomingMessagesHandler.h"
#include "../common/exceptions/RuntimeError.h"

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
        const NodeUUID &nodeUUID,
        const string &nodeInterface,
        const uint16_t nodePort,
        const string &uuid2AddressHost,
        const uint16_t uuid2AddressPort);

    ~Communicator();

    void beginAcceptMessages();

private:
    static constexpr const size_t kMaxIncomingBufferSize = 1024;

private:
    const string mInterface;
    const uint16_t mPort;

    // external
    const NodeUUID &mNodeUUID;
    UUID2Address *mUUID2AddressService;

    // internal
    udp::socket *mSocket;
    IncomingMessagesHandler *mIncomingMessagesHandler;
    OutgoingMessagesHandler *mOutgoingMessagesHandler;




    as::io_service &mIOService;


    boost::array<char, kMaxIncomingBufferSize> mRecvBuffer;
    udp::endpoint mRemoteEndpointBuffer;



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
