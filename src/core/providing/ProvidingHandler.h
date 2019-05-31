#ifndef GEO_NETWORK_CLIENT_PROVIDINGHANDLER_H
#define GEO_NETWORK_CLIENT_PROVIDINGHANDLER_H

#include "Provider.h"
#include "../contractors/Contractor.h"
#include "../logger/LoggerMixin.hpp"

#include "../../libs/json/json.h"

using json = nlohmann::json;
namespace as = boost::asio;
namespace signals = boost::signals2;

class ProvidingHandler : protected LoggerMixin {

public:
    typedef signals::signal<void(Provider::Shared)> SendPingMessageSignal;
    typedef uint8_t SerializedType;
    typedef uint32_t MessageSize;

    enum ProtocolVersion {
        Latest = 0,
    };

    enum MessageType {
        Providing_Ping = 1,
        Providing_Pong = 2,
    };

public:
    ProvidingHandler(
        json providersConf,
        IOService &ioService,
        Contractor::Shared selfContractor,
        Logger &logger);

    Provider::Shared getProviderForAddress(
        GNSAddress::Shared gnsAddress);

    IPv4WithPortAddress::Shared getIPv4AddressForGNS(
        GNSAddress::Shared gnsAddress);

    void setIPv4AddressForGNS(
        GNSAddress::Shared gnsAddress,
        IPv4WithPortAddress::Shared ipv4Address);

    Provider::Shared mainProvider() const;

protected:
    const string logHeader() const override;

private:
    void updateAddressForProviders(
        const boost::system::error_code &errorCode);

public:
    mutable SendPingMessageSignal sendPingMessageSignal;

private:
    static const uint16_t kUpdatingAddressPeriodSeconds = 20;

private:
    vector<Provider::Shared> mProviders;
    vector<Provider::Shared> mProvidersForPing;
    Contractor::Shared mSelfContractor;
    as::steady_timer mUpdatingAddressTimer;

    map<string, IPv4WithPortAddress::Shared> mCachedAddresses;
};


#endif //GEO_NETWORK_CLIENT_PROVIDINGHANDLER_H
