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
    IPv4WithPortAddress::Shared contractorIPAddress;
    auto contractorAddress = mContractorsManager->contractor(addressee)->mainAddress();
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    mLog.debug("OutgoingMessagesHandler::sendMessage")
            << "Send message to contractor main address " << contractorAddress->fullAddress();
#endif

    MsgEncryptor::Buffer sendingData;
    if(!message->isEncrypted()) {
        sendingData = message->serializeToBytes();
    } else {
        sendingData = MsgEncryptor(
            mContractorsManager->contractor(message->contractorId())->cryptoKey()->contractorPublicKey
        ).encrypt(message);
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        mLog.debug("OutgoingMessagesHandler::sendMessage") << "Message encrypted";
#endif
    }

    if (contractorAddress->typeID() == BaseAddress::IPv4_IncludingPort) {
        contractorIPAddress = static_pointer_cast<IPv4WithPortAddress>(
            contractorAddress);
    } else if (contractorAddress->typeID() == BaseAddress::GNS) {
        auto gnsAddress = static_pointer_cast<GNSAddress>(contractorAddress);
        contractorIPAddress = mProvidingHandler->getIPv4AddressForGNS(
            gnsAddress);
        if (contractorIPAddress == nullptr) {
            auto provider = mProvidingHandler->getProviderForAddress(gnsAddress);
            if (provider == nullptr) {
                provider = mProvidingHandler->mainProvider();
            }
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
            mLog.debug("OutgoingMessagesHandler::sendMessage")
                    << "send request to provider: " << provider->name();
#endif
            mPostponedMessages.insert(
                make_pair(
                    gnsAddress->fullAddress(),
                    sendingData));

            auto node = mNodes.handler(provider);
            auto sendingProviderData = getRemoteNodeAddressMessage(
                provider,
                gnsAddress);
            node->sendMessage(sendingProviderData);
            return;
        }
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        mLog.debug("OutgoingMessagesHandler::sendMessage")
                << "ipv4: " << contractorIPAddress->fullAddress();
#endif
    } else {
        mLog.error("OutgoingMessagesHandler::sendMessage")
            << "Unsupported address type " << contractorAddress->typeID();
        return;
    }
    try {
        auto node = mNodes.handler(contractorIPAddress);
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

    auto sendingData = message->serializeToBytes();

    IPv4WithPortAddress::Shared contractorIPAddress;
    try {
        if (address->typeID() == BaseAddress::IPv4_IncludingPort) {
            contractorIPAddress = static_pointer_cast<IPv4WithPortAddress>(address);
        } else if (address->typeID() == BaseAddress::GNS) {
            auto gnsAddress = static_pointer_cast<GNSAddress>(address);
            contractorIPAddress = mProvidingHandler->getIPv4AddressForGNS(
                gnsAddress);
            if (contractorIPAddress == nullptr) {
                auto provider = mProvidingHandler->getProviderForAddress(gnsAddress);
                if (provider == nullptr) {
                    provider = mProvidingHandler->mainProvider();
                }
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
                mLog.debug("OutgoingMessagesHandler::sendMessage")
                        << "send request to provider: " << provider->name();
#endif
                mPostponedMessages.insert(
                    make_pair(
                        gnsAddress->fullAddress(),
                    sendingData));

                auto node = mNodes.handler(provider);
                auto sendingProviderData = getRemoteNodeAddressMessage(
                    provider,
                    gnsAddress);
                node->sendMessage(sendingProviderData);
                return;
            }
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
            mLog.debug("OutgoingMessagesHandler::sendMessage")
                    << " ipv4: " << contractorIPAddress->fullAddress();
#endif
        }

        auto node = mNodes.handler(contractorIPAddress);
        node->sendMessage(sendingData);

    } catch (exception &e) {
        mLog.error("OutgoingMessagesHandler::sendMessage")
                << "Attempt to send message to the node (" << address->fullAddress() << ") failed with exception. "
                << "Details are: " << e.what() << ". "
                << "Message type: " << message->typeID();
    }
}

void OutgoingMessagesHandler::processProviderResponse(
    ProvidingAddressResponseMessage::Shared providerResponse)
{
    auto gnsAndIPv4Addresses = deserializeProviderResponse(
        providerResponse->buffer());
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    mLog.debug("OutgoingMessagesHandler") << "Provider response "
        << gnsAndIPv4Addresses.first->fullAddress() << " "
        << gnsAndIPv4Addresses.second->fullAddress();
#endif
    mProvidingHandler->setCachedIPv4AddressForGNS(
        gnsAndIPv4Addresses.first,
        gnsAndIPv4Addresses.second);

    auto postponedMessagesRange = mPostponedMessages.equal_range(
        gnsAndIPv4Addresses.first->fullAddress());
    for (auto postponedMessageIt = postponedMessagesRange.first;
        postponedMessageIt != postponedMessagesRange.second; ++postponedMessageIt) {

        try {
            auto node = mNodes.handler(gnsAndIPv4Addresses.second);
            node->sendMessage(postponedMessageIt->second);
        } catch (exception &e) {
            mLog.error("OutgoingMessagesHandler::processProviderResponse")
                    << "Attempt to send message to the node (" << gnsAndIPv4Addresses.first->fullAddress()
                    << ") failed with exception. Details are: " << e.what();
            continue;
        }
    }
    mPostponedMessages.erase(
        gnsAndIPv4Addresses.first->fullAddress());
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    mLog.debug("OutgoingMessagesHandler::processProviderResponse")
        << "Postponed messages count " << mPostponedMessages.size();
#endif
}

void OutgoingMessagesHandler::onPingMessageToProviderReady(
    Provider::Shared provider)
{
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    mLog.debug("OutgoingMessagesHandler") << "Provider ping " <<
        provider->name() << " " << provider->mainAddress()->fullAddress();
#endif

//    try {
//        auto node = mNodes.handler(provider);
//        auto sendingData = pingMessage(provider);
//        node->sendMessage(sendingData);
//    } catch (exception &e) {
//        mLog.error("OutgoingMessagesHandler::onPingMessageToProviderReady")
//                << "Attempt to send message to the provider (" << provider->name() << ") failed with exception. "
//                << "Details are: " << e.what();
//    }
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
     SerializedProtocolVersion kProtocolVersion = ProvidingHandler::ProtocolVersion::Latest;
     const Message::SerializedType kMessageType = Message::ProvidingAddressResponse;
     const auto kMessageSize = sizeof(SerializedProtocolVersion) + sizeof(kMessageType) +
             sizeof(uint16_t) + gnsAddress->fullAddress().size();
     auto buffer = tryMalloc(kMessageSize);
     size_t dataBytesOffset = 0;

     memcpy(
         buffer.get(),
         &kProtocolVersion,
         sizeof(SerializedProtocolVersion));
     dataBytesOffset += sizeof(SerializedProtocolVersion);

     memcpy(
         buffer.get() + dataBytesOffset,
         &kMessageType,
         sizeof(kMessageType));
     dataBytesOffset += sizeof(kMessageType);

     auto gnsAddressStr = gnsAddress->fullAddress();
     auto gnsAddressStrLength = gnsAddressStr.size();

     memcpy(
         buffer.get() + dataBytesOffset,
         &gnsAddressStrLength,
         sizeof(uint16_t));
     dataBytesOffset += sizeof(uint16_t);

     memcpy(
         buffer.get() + dataBytesOffset,
         gnsAddressStr.c_str(),
         gnsAddressStrLength);

     return make_pair(
         buffer,
         kMessageSize);
 }

pair<GNSAddress::Shared, IPv4WithPortAddress::Shared> OutgoingMessagesHandler::deserializeProviderResponse(
    BytesShared buffer)
{
//    return make_pair(
//        make_shared<GNSAddress>(
//            "address@geo.pay"),
//        make_shared<IPv4WithPortAddress>(
//            "127.0.0.1:2008"));

    auto bytesBufferOffset = sizeof(SerializedProtocolVersion) + sizeof(ContractorID) + sizeof(Message::SerializedType);
    auto gnsAddress = deserializeAddress(
        buffer.get() + bytesBufferOffset);
    bytesBufferOffset += gnsAddress->serializedSize();
    auto ipAddress = deserializeAddress(
        buffer.get() + bytesBufferOffset);
    return make_pair(
        static_pointer_cast<GNSAddress>(gnsAddress),
        static_pointer_cast<IPv4WithPortAddress>(ipAddress));
}
