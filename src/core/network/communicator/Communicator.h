#ifndef GEO_NETWORK_CLIENT_COMMUNICATOR_H
#define GEO_NETWORK_CLIENT_COMMUNICATOR_H

#include "../../common/Types.h"

#include "../service/UUID2Address.h"
#include "../channels/manager/ChannelsManager.h"
#include "../internal/OutgoingMessagesHandler.h"
#include "../internal/IncomingMessagesHandler.h"

#include "../../common/exceptions/RuntimeError.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>

#include <memory>
#include <string>
#include <vector>
#ifdef MAC_OS
#include <stdlib.h>
#endif

#ifdef LINUX
#include <malloc.h>
#endif


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

    void sendMessage (
        const Message::Shared kMessage,
        const NodeUUID &contractorUUID);

private:
    void connectIncomingMessagesHandlerSignals();

    void onMessageParsedSlot(
        Message::Shared message);

    void asyncReceiveData();

    void handleReceivedInfo(
        const boost::system::error_code &error,
        size_t bytesTransferred);

    void sendData(
        udp::endpoint &address,
        vector<byte> buffer,
        Channel::Shared channel);

    void handleSend(
        const boost::system::error_code &error,
        size_t bytesTransferred,
        udp::endpoint endpoint,
        Channel::Shared channel);

private:
    // ToDo: consider using dynamic buffers for better memory usage.
    // ToDo: tune this parameter in according to the tests for huge size RT exchanging.
    static const constexpr size_t kMaxIncomingBufferSize = 508 * 16;

    as::io_service &mIOService;
    NodeUUID &mNodeUUID;
    const string mInterface;
    const uint16_t mPort;
    Logger *mLog;

    unique_ptr<udp::socket> mSocket;

    unique_ptr<UUID2Address> mUUID2AddressService;
    unique_ptr<ChannelsManager> mChannelsManager;
    unique_ptr<IncomingMessagesHandler> mIncomingMessagesHandler;
    unique_ptr<OutgoingMessagesHandler> mOutgoingMessagesHandler;

    boost::array<byte, kMaxIncomingBufferSize> mReceiveBuffer;
    udp::endpoint mRemoteEndpointBuffer;
};



#endif //GEO_NETWORK_CLIENT_COMMUNICATOR_H
