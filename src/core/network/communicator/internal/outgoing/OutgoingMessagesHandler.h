#ifndef GEO_NETWORK_CLIENT_OUTGOINGMESSAGESHANDLER_H
#define GEO_NETWORK_CLIENT_OUTGOINGMESSAGESHANDLER_H

#include "OutgoingNodesHandler.h"
#include "../../../../providing/ProvidingHandler.h"
#include "../../../../contractors/ContractorsManager.h"
#include "../../../messages/providing/ProvidingAddressResponseMessage.h"

#include "../common/Types.h"


class OutgoingMessagesHandler {
public:
    OutgoingMessagesHandler(
        IOService &ioService,
        UDPSocket &socket,
        ContractorsManager *contractorsManager,
        ProvidingHandler *providingHandler,
        Logger &log)
        noexcept;

    void sendMessage(
        const Message::Shared message,
        const ContractorID addressee);

    void sendMessage(
        const Message::Shared message,
        const BaseAddress::Shared address);

    void processProviderResponse(
        ProvidingAddressResponseMessage::Shared providerResponse);

private:
    void onPingMessageToProviderReady(
        Provider::Shared provider);

    MsgEncryptor::Buffer pingMessage(
        Provider::Shared provider) const;

    MsgEncryptor::Buffer getRemoteNodeAddressMessage(
        Provider::Shared provider,
        GNSAddress::Shared gnsAddress) const;

    pair<GNSAddress::Shared, IPv4WithPortAddress::Shared> deserializeProviderResponse(
        BytesShared buffer);

protected:
    Logger &mLog;
    OutgoingNodesHandler mNodes;
    ContractorsManager *mContractorsManager;
    ProvidingHandler *mProvidingHandler;

    multimap<string, MsgEncryptor::Buffer> mPostponedMessages;
};


#endif //GEO_NETWORK_CLIENT_OUTGOINGMESSAGESHANDLER_H
