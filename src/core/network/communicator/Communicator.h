#ifndef GEO_NETWORK_CLIENT_COMMUNICATOR_H
#define GEO_NETWORK_CLIENT_COMMUNICATOR_H

#include "../../common/Types.h"

#include "../../Core.h"

#include "../service/UUID2Address.h"
#include "../channels/manager/ChannelsManager.h"
#include "../internal/OutgoingMessagesHandler.h"
#include "../internal/IncomingMessagesHandler.h"

#include "../../common/exceptions/RuntimeError.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <stdlib.h>
#include <string>
#include <vector>


using namespace std;

namespace as = boost::asio;
namespace ip = boost::asio::ip;

typedef boost::system::error_code err;

class Core;
class IncomingMessagesHandler;
class Communicator {
public:
    explicit Communicator(
        Core *core,
        as::io_service &ioService,
        NodeUUID &nodeUUID,
        const string &nodeInterface,
        const uint16_t nodePort,
        const string &uuid2AddressHost,
        const uint16_t uuid2AddressPort,
        Logger *logger);

    ~Communicator();

    NodeUUID &nodeUUID() const;

    void beginAcceptMessages();

    void sendMessage(
        Message::Shared message,
        const NodeUUID &contractorUUID);

private:
    void asyncReceiveData();

    void handleReceivedInfo(
        const boost::system::error_code &error,
        size_t bytesTransferred);

    void sendData(
        vector<byte> buffer,
        pair <string, uint16_t> address);

    void handleSend(
        const boost::system::error_code &error,
        size_t bytesTransferred);

private:
    static constexpr const size_t kMaxIncomingBufferSize = 500*2;

    Core *mCore;
    as::io_service &mIOService;
    NodeUUID &mNodeUUID;
    const string mInterface;
    const uint16_t mPort;

    UUID2Address *mUUID2AddressService;

    ChannelsManager *mChannelsManager;
    udp::socket *mSocket;
    IncomingMessagesHandler *mIncomingMessagesHandler;
    OutgoingMessagesHandler *mOutgoingMessagesHandler;

    boost::array<byte, kMaxIncomingBufferSize> mRecvBuffer;
    udp::endpoint mRemoteEndpointBuffer;

    Logger *mLog;
};


#endif //GEO_NETWORK_CLIENT_COMMUNICATOR_H
