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
    mProvidingHandler(providingHandler),
    mPostponedMessagesCleaningTimer(ioService)
{
    mProvidingHandler->sendPingMessageSignal.connect(
        boost::bind(
            &OutgoingMessagesHandler::onPingMessageToProviderReady,
            this,
            _1));

    mPostponedMessagesCleaningTimer.expires_from_now(
        std::chrono::seconds(
            +kPostponedMessagesClearingPeriodSeconds));
    mPostponedMessagesCleaningTimer.async_wait(
        boost::bind(
            &OutgoingMessagesHandler::clearUndeliveredMessages,
            this,
            as::placeholders::error));
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
        if (!mProvidingHandler->isProvidersPresent()) {
            mLog.warning("OutgoingMessagesHandler::sendMessage")
                    << "Attempt to send message to the node (" << addressee << " " << contractorAddress->fullAddress()
                    << ") failed due provider absence";
            return;
        }
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
                    make_pair(
                        sendingData,
                        utc_now())));

            auto node = mNodes.providerHandler(
                provider->lookupAddress());
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
            if (!mProvidingHandler->isProvidersPresent()) {
                mLog.warning("OutgoingMessagesHandler::sendMessage")
                        << "Attempt to send message to the node (" << address->fullAddress()
                        << ") failed due provider absence";
                return;
            }
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
                        make_pair(
                            sendingData,
                            utc_now())));

                auto node = mNodes.providerHandler(
                    provider->lookupAddress());
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
    pair<GNSAddress::Shared, IPv4WithPortAddress::Shared> gnsAndIPv4Addresses;
    try {
        gnsAndIPv4Addresses = deserializeProviderResponse(
            providerResponse->buffer());
    } catch (ValueError &e) {
        mLog.error("OutgoingMessagesHandler::processProviderResponse")
            << "Can't deserialize provider response. Details " << e.what();
        return;
    }
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
            node->sendMessage(
                postponedMessageIt->second.first);
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
        provider->name() << " " << provider->pingAddress()->fullAddress();
#endif

    try {
        auto node = mNodes.providerHandler(provider->pingAddress());
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
    const auto kMessageSize = sizeof(SerializedProtocolVersion) +
                              sizeof(ProviderParticipantID) + sizeof(GEOEpochTimestamp);
    auto buffer = tryMalloc(kMessageSize);

    SerializedProtocolVersion kProtocolVersion = ProvidingHandler::Latest;
    memcpy(
        buffer.get(),
        &kProtocolVersion,
        sizeof(SerializedProtocolVersion));

    auto participantID = provider->participantID();
    memcpy(
        buffer.get() + sizeof(SerializedProtocolVersion),
        &participantID,
        sizeof(ProviderParticipantID));

    auto now = utc_now();
    memcpy(
        buffer.get() + sizeof(SerializedProtocolVersion) + sizeof(ProviderParticipantID),
        &now,
        sizeof(GEOEpochTimestamp));

    return make_pair(
        buffer,
        kMessageSize);
}

MsgEncryptor::Buffer OutgoingMessagesHandler::getRemoteNodeAddressMessage(
     Provider::Shared provider,
     GNSAddress::Shared gnsAddress) const
 {
     const auto kMessageSize = sizeof(SerializedProtocolVersion) +
             sizeof(uint16_t) + gnsAddress->fullAddress().size();
     auto buffer = tryMalloc(kMessageSize);
     size_t dataBytesOffset = 0;

     SerializedProtocolVersion kProtocolVersion = ProvidingHandler::ProtocolVersion::Latest;
     memcpy(
         buffer.get(),
         &kProtocolVersion,
         sizeof(SerializedProtocolVersion));
     dataBytesOffset += sizeof(SerializedProtocolVersion);

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
    auto bytesBufferOffset = sizeof(SerializedProtocolVersion) + sizeof(ContractorID) + sizeof(Message::SerializedType);

    auto *gnsAddressLength = new (buffer.get() + bytesBufferOffset) uint16_t;
    if (*gnsAddressLength == 0) {
        throw ValueError("GNSAddress: can't read 0 length address");
    }
    bytesBufferOffset += sizeof(uint16_t);
    string gnsAddressStr = string(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + *gnsAddressLength);
    bytesBufferOffset += *gnsAddressLength;
    auto gnsAddress = make_shared<GNSAddress>(gnsAddressStr);

    auto *ipv4AddressLength = new (buffer.get() + bytesBufferOffset) uint16_t;
    if (*ipv4AddressLength == 0) {
        throw ValueError("IPv4Address: can't read 0 length address");
    }
    bytesBufferOffset += sizeof(uint16_t);
    string ipv4AddressStr = string(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + *ipv4AddressLength);
    auto ipv4Address = make_shared<IPv4WithPortAddress>(ipv4AddressStr);

    return make_pair(
        gnsAddress,
        ipv4Address);
}

void OutgoingMessagesHandler::clearUndeliveredMessages(
    const boost::system::error_code &errorCode)
{
    if (errorCode) {
        mLog.warning("OutgoingMessagesHandler::clearUndeliveredMessages") << errorCode.message().c_str();
    }
    mPostponedMessagesCleaningTimer.cancel();
    auto postponedMessageIt = mPostponedMessages.begin();
    auto now = utc_now();
    while (postponedMessageIt != mPostponedMessages.end()) {
        if (postponedMessageIt->second.second < now) {
            mPostponedMessages.erase(
                postponedMessageIt);
        } else {
            postponedMessageIt++;
        }
    }

    mPostponedMessagesCleaningTimer.expires_from_now(
        std::chrono::seconds(
            +kPostponedMessagesClearingPeriodSeconds));
    mPostponedMessagesCleaningTimer.async_wait(
        boost::bind(
            &OutgoingMessagesHandler::clearUndeliveredMessages,
            this,
            as::placeholders::error));
}
