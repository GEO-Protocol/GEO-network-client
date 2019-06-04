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

    void setCachedIPv4AddressForGNS(
        GNSAddress::Shared gnsAddress,
        IPv4WithPortAddress::Shared ipv4Address);

    Provider::Shared mainProvider() const;

    bool isProvidersPresent() const;

protected:
    const string logHeader() const override;

private:
    void updateAddressForProviders(
        const boost::system::error_code &errorCode);

    void rescheduleCleaning();

    void clearCahedAddresses();

public:
    mutable SendPingMessageSignal sendPingMessageSignal;

private:
    static const uint16_t kUpdatingAddressPeriodSeconds = 20;

    static const byte kResetCacheAddressHours = 0;
    static const byte kResetCacheAddressMinutes = 0;
    static const byte kResetCacheAddressSeconds = 10;

    static Duration& kResetCacheAddressDuration() {
        static auto duration = Duration(
                kResetCacheAddressHours,
                kResetCacheAddressMinutes,
                kResetCacheAddressSeconds);
        return duration;
    }

private:
    vector<Provider::Shared> mProviders;
    vector<Provider::Shared> mProvidersForPing;
    Contractor::Shared mSelfContractor;
    as::steady_timer mUpdatingAddressTimer;
    as::steady_timer mCacheCleaningTimer;

    map<string, IPv4WithPortAddress::Shared> mCachedAddresses;
    vector<pair<DateTime, string>> mTimesCache;
};


#endif //GEO_NETWORK_CLIENT_PROVIDINGHANDLER_H
