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

public:
    ProvidingHandler(
        json providersConf,
        IOService &ioService,
        Contractor::Shared selfContractor,
        Logger &logger);

protected:
    const string logHeader() const override;

private:
    void updateAddressForProviders(
        const boost::system::error_code &errorCode);

    Provider::Shared getProviderForAddress(
        GNSAddress::Shared gnsAddress);

public:
    mutable SendPingMessageSignal sendPingMessageSignal;

private:
    static const uint16_t kUpdatingAddressPeriodSeconds = 20;

private:
    vector<Provider::Shared> mProviders;
    vector<Provider::Shared> mProvidersForPing;
    Contractor::Shared mSelfContractor;
    as::steady_timer mUpdatingAddressTimer;
};


#endif //GEO_NETWORK_CLIENT_PROVIDINGHANDLER_H
