#include "ProvidingHandler.h"

ProvidingHandler::ProvidingHandler(
    json providersConf,
    IOService &ioService,
    Contractor::Shared selfContractor,
    Logger &logger) :
    LoggerMixin(logger),
    mUpdatingAddressTimer(ioService),
    mSelfContractor(selfContractor)
{
    if (providersConf == nullptr) {
        info() << "There are no providers in config";
        return;
    }
    for (const auto &providerConf : providersConf) {
        vector<pair<string, string>> providerAddressesStr;
        for (const auto &providerAddressConf : providerConf.at("addresses")) {
            providerAddressesStr.emplace_back(
                providerAddressConf.at("type").get<string>(),
                providerAddressConf.at("address").get<string>());
        }
        mProviders.push_back(
            make_shared<Provider>(
                providerConf.at("name").get<string>(),
                providerConf.at("key").get<string>(),
                providerAddressesStr));

    }
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    info() << "Providers:";
    for (const auto &provider : mProviders) {
        info() << provider->info();
    }
#endif

    for (const auto &selfAddress : mSelfContractor->addresses()) {
        if (selfAddress->typeID() == BaseAddress::GNS) {
            auto provider = getProviderForAddress(static_pointer_cast<GNSAddress>(selfAddress));
            if (provider == nullptr) {
                throw ValueError("There is no provider configuration for address " + selfAddress->fullAddress());
            }
            mProvidersForPing.push_back(provider);
        }
    }

    if (!mProvidersForPing.empty()) {
        mUpdatingAddressTimer.expires_from_now(
            std::chrono::seconds(
                +kUpdatingAddressPeriodSeconds));
        mUpdatingAddressTimer.async_wait(
            boost::bind(
                &ProvidingHandler::updateAddressForProviders,
                this,
                as::placeholders::error));
    }
}

void ProvidingHandler::updateAddressForProviders(
    const boost::system::error_code &errorCode)
{
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    debug() << "updateAddressForProviders";
#endif
    if (errorCode) {
        warning() << errorCode.message().c_str();
    }
    mUpdatingAddressTimer.cancel();

    for (const auto &provider : mProvidersForPing) {
        sendPingMessageSignal(
            provider);
    }

    mUpdatingAddressTimer.expires_from_now(
        std::chrono::seconds(
            +kUpdatingAddressPeriodSeconds));
    mUpdatingAddressTimer.async_wait(
        boost::bind(
            &ProvidingHandler::updateAddressForProviders,
            this,
            as::placeholders::error));
}

Provider::Shared ProvidingHandler::getProviderForAddress(
    GNSAddress::Shared gnsAddress)
{
    for (const auto &provider : mProviders) {
        if (gnsAddress->provider() == provider->name()) {
            return provider;
        }
    }
    return nullptr;
}

const string ProvidingHandler::logHeader() const
{
    return "[ProvidingHandler]";
}