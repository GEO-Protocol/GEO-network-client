#ifndef GEO_NETWORK_CLIENT_PROVIDINGHANDLER_H
#define GEO_NETWORK_CLIENT_PROVIDINGHANDLER_H

#include "Provider.h"
#include "../contractors/Contractor.h"
#include "../logger/LoggerMixin.hpp"

#include <boost/signals2.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace as = boost::asio;
namespace signals = boost::signals2;

class ProvidingHandler : protected LoggerMixin {

public:
    typedef signals::signal<void(Provider::Shared)> SendPingMessageSignal;

    enum ProtocolVersion {
        Latest = 0,
    };

public:
    ProvidingHandler(
        vector<Provider::Shared> &providers,
        uint32_t updatingAddressPeriodSeconds,
        uint32_t cachedAddressTTLSeconds,
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

    void clearCachedAddresses();

public:
    mutable SendPingMessageSignal sendPingMessageSignal;

private:
    static const uint16_t kStartingAddressPeriodSeconds = 5;

private:
    vector<Provider::Shared> mProviders;
    vector<Provider::Shared> mProvidersForPing;
    Contractor::Shared mSelfContractor;
    as::steady_timer mUpdatingAddressTimer;
    as::steady_timer mCacheCleaningTimer;

    uint32_t mUpdatingAddressPeriodSeconds;
    uint32_t mCachedAddressTTLSeconds;

    map<string, IPv4WithPortAddress::Shared> mCachedAddresses;
    vector<pair<DateTime, string>> mTimesCache;
};


#endif //GEO_NETWORK_CLIENT_PROVIDINGHANDLER_H
