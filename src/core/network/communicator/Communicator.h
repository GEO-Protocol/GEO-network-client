#ifndef GEO_NETWORK_CLIENT_COMMUNICATOR_H
#define GEO_NETWORK_CLIENT_COMMUNICATOR_H

#include "../../common/Types.h"

#include "../service/UUID2Address.h"
#include "../channels/manager/ChannelsManager.h"
#include "../internal/OutgoingMessagesHandler.h"
#include "../internal/IncomingMessagesHandler.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>

#include <stdlib.h>
#include <string>
#include <vector>


using namespace std;

namespace as = boost::asio;
namespace ip = boost::asio::ip;
namespace signals = boost::signals2;

class Communicator {
public:
    signals::signal<void(Message::Shared)> messageReceivedSignal;

public:
    explicit Communicator(
        as::io_service &ioService,
        NodeUUID &nodeUUID,
        const string &nodeInterface,
        const uint16_t nodePort,
        const string &uuid2AddressHost,
        const uint16_t uuid2AddressPort,
        Logger *logger);

    ~Communicator();

    const NodeUUID &nodeUUID() const;

    void beginAcceptMessages();

    void sendMessage(
        Message::Shared message,
        const NodeUUID &contractorUUID);

private:
    void connectIncomingMessagesHanlderSignals();

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

    void onMessageParsedSlot(
        Message::Shared message);

    void zeroPointers();

    void cleanupMemory();

private:
    static const constexpr size_t kMaxIncomingBufferSize = 1000;

    as::io_service &mIOService;
    NodeUUID &mNodeUUID;
    const string mInterface;
    const uint16_t mPort;
    Logger *mLog;

    udp::socket *mSocket;

    UUID2Address *mUUID2AddressService;
    ChannelsManager *mChannelsManager;
    IncomingMessagesHandler *mIncomingMessagesHandler;
    OutgoingMessagesHandler *mOutgoingMessagesHandler;

    boost::array<byte, kMaxIncomingBufferSize> mRecvBuffer;
    udp::endpoint mRemoteEndpointBuffer;
};



#endif //GEO_NETWORK_CLIENT_COMMUNICATOR_H
