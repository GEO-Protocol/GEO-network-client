#include "OutgoingMessagesHandler.h"


OutgoingMessagesHandler::OutgoingMessagesHandler(
    IOService &ioService,
    UDPSocket &socket,
    ContractorsManager *contractorsManager,
    ProvidingHandler *providingHandler,
    Logger &log)
    noexcept :

    mLog(log),
    mNodes(
        ioService,
        socket,
        contractorsManager,
        log),
    mContractorsManager(contractorsManager),
    mProvidingHandler(providingHandler)
{
    mProvidingHandler->sendPingMessageSignal.connect(
        boost::bind(
            &OutgoingMessagesHandler::onPingMessageToProviderReady,
            this,
            _1));
}

void OutgoingMessagesHandler::sendMessage(
    const Message::Shared message,
    const ContractorID addressee)
{
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    mLog.debug("OutgoingMessagesHandler::sendMessage")
            << "Send message to the node (" << addressee << ") "
            << "Message type: " << message->typeID() << " encrypted: " << message->isEncrypted();
#endif
    try {
        MsgEncryptor::Buffer sendingData;
        auto node = mNodes.handler(addressee);
        if(!message->isEncrypted()) {
            sendingData = message->serializeToBytes();
        } else {
            sendingData = MsgEncryptor(
                mContractorsManager->contractor(message->contractorId())->cryptoKey()->contractorPublicKey
            ).encrypt(message);
        }
        node->sendMessage(sendingData);

    } catch (exception &e) {
        mLog.error("OutgoingMessagesHandler::sendMessage")
            << "Attempt to send message to the node (" << addressee << ") failed with exception. "
            << "Details are: " << e.what() << ". "
            << "Message type: " << message->typeID();
    }
}

void OutgoingMessagesHandler::sendMessage(
    const Message::Shared message,
    const BaseAddress::Shared address)
{
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    mLog.debug("OutgoingMessagesHandler::sendMessage")
            << "Send message to the node (" << address->fullAddress() << ") "
            << "Message type: " << message->typeID();
#endif
    try {
        if (address->typeID() == BaseAddress::GNS) {
            auto gnsAddress = static_pointer_cast<GNSAddress>(address);
            auto ipv4Address = mProvidingHandler->getIPv4AddressForGNS(
                gnsAddress);
            if (ipv4Address != nullptr) {
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
                mLog.debug("OutgoingMessagesHandler::sendMessage")
                    << " ipv4: " << ipv4Address->fullAddress();
#endif
                auto node = mNodes.handler(ipv4Address);
                auto sendingData = message->serializeToBytes();
                node->sendMessage(sendingData);
                return;
            } else {
                auto provider = mProvidingHandler->getProviderForAddress(gnsAddress);
                if (provider == nullptr) {
                    provider = mProvidingHandler->mainProvider();
                }
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
                mLog.debug("OutgoingMessagesHandler::sendMessage")
                        << " send request to provider: " << provider->name();
#endif
                auto node = mNodes.handler(provider);
                auto sendingData = getRemoteNodeAddressMessage(
                    provider,
                    gnsAddress);
                node->sendMessage(sendingData);
                return;
            }
        }

        auto node = mNodes.handler(address);
        auto sendingData = message->serializeToBytes();
        node->sendMessage(sendingData);

    } catch (exception &e) {
        mLog.error("OutgoingMessagesHandler::sendMessage")
                << "Attempt to send message to the node (" << address->fullAddress() << ") failed with exception. "
                << "Details are: " << e.what() << ". "
                << "Message type: " << message->typeID();
    }
}

void OutgoingMessagesHandler::onPingMessageToProviderReady(
    Provider::Shared provider)
{
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    mLog.debug("OutgoingMessagesHandler") << "Provider ping " <<
        provider->name() << " " << provider->mainAddress()->fullAddress();
#endif

    try {
        auto node = mNodes.handler(provider);
        auto sendingData = pingMessage(provider);
        node->sendMessage(sendingData);
    } catch (exception &e) {
        mLog.error("OutgoingMessagesHandler::onPingMessageToProviderReady")
                << "Attempt to send message to the provider (" << provider->name() << ") failed with exception. "
                << "Details are: " << e.what();
    }
}

MsgEncryptor::Buffer OutgoingMessagesHandler::pingMessage(
    Provider::Shared provider) const
{
    SerializedProtocolVersion kProtocolVersion = ProvidingHandler::ProtocolVersion::Latest;
    const ProvidingHandler::SerializedType kMessageType = ProvidingHandler::Providing_Ping;
    const auto kMessageSize = sizeof(ProvidingHandler::MessageSize) +
                              sizeof(SerializedProtocolVersion) +
                              sizeof(ProvidingHandler::SerializedType);;
    auto buffer = tryMalloc(kMessageSize);

    auto dataSize = kMessageSize - sizeof(ProvidingHandler::MessageSize);
    memcpy(
        buffer.get(),
        &dataSize,
        sizeof(ProvidingHandler::MessageSize));

    memcpy(
        buffer.get() + sizeof(ProvidingHandler::MessageSize),
        &kProtocolVersion,
        sizeof(SerializedProtocolVersion));

    memcpy(
        buffer.get() + sizeof(ProvidingHandler::MessageSize) + sizeof(SerializedProtocolVersion),
        &kMessageType,
        sizeof(ProvidingHandler::SerializedType));

    return make_pair(
        buffer,
        kMessageSize);
}

MsgEncryptor::Buffer OutgoingMessagesHandler::getRemoteNodeAddressMessage(
     Provider::Shared provider,
     GNSAddress::Shared gnsAddress) const
 {
     const auto kMessageSize = 0;
     auto buffer = tryMalloc(kMessageSize);
     return make_pair(
         buffer,
         kMessageSize);
 }
