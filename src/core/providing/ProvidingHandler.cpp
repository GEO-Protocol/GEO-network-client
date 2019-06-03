#include "ProvidingHandler.h"

ProvidingHandler::ProvidingHandler(
    json providersConf,
    IOService &ioService,
    Contractor::Shared selfContractor,
    Logger &logger) :
    LoggerMixin(logger),
    mUpdatingAddressTimer(ioService),
    mCacheCleaningTimer(ioService),
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

IPv4WithPortAddress::Shared ProvidingHandler::getIPv4AddressForGNS(
    GNSAddress::Shared gnsAddress)
{
    if (0 == mCachedAddresses.count(gnsAddress->fullAddress())) {
        return nullptr;
    }
    return mCachedAddresses[gnsAddress->fullAddress()];
}

void ProvidingHandler::setCachedIPv4AddressForGNS(
    GNSAddress::Shared gnsAddress,
    IPv4WithPortAddress::Shared ipv4Address)
{
    mCachedAddresses.insert(
        make_pair(
            gnsAddress->fullAddress(),
            ipv4Address));
    mTimesCache.emplace_back(
        utc_now() + kResetCacheAddressDuration(),
        gnsAddress->fullAddress());

    if (mTimesCache.size() == 1) {
        rescheduleCleaning();
    }
}

Provider::Shared ProvidingHandler::mainProvider() const
{
    return mProviders.at(0);
}

void ProvidingHandler::rescheduleCleaning()
{
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    debug() << "rescheduleCleaning";
#endif
    if (mTimesCache.empty()) {
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        debug() << "There are no cached addresses";
#endif
        return;
    }
    const auto kCleaningTimeout = mTimesCache.at(0).first - utc_now();
    mCacheCleaningTimer.expires_from_now(
        chrono::milliseconds(
            kCleaningTimeout.total_milliseconds()));
    mCacheCleaningTimer.async_wait(
        boost::bind(
            &ProvidingHandler::clearCahedAddresses,
            this));
}

void ProvidingHandler::clearCahedAddresses()
{
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    debug() << "clearCahedAddresses " << mCachedAddresses.size() << " " << mTimesCache.size();
#endif
    auto now = utc_now();
    auto it = mTimesCache.begin();
    while (it != mTimesCache.end()) {
        if (it->first > now) {
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
            if (mCachedAddresses.count(it->second) != 0) {
                debug() << "remove cache " << it->second << " " << mCachedAddresses[it->second]->fullAddress();
            }
#endif
            mCachedAddresses.erase(it->second);
            mTimesCache.erase(it);
        } else {
            it++;
        }
    }

    if (!mTimesCache.empty()) {
        rescheduleCleaning();
    }
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    debug() << "after cleaning " << mCachedAddresses.size() << " " << mTimesCache.size();
#endif
}

const string ProvidingHandler::logHeader() const
{
    return "[ProvidingHandler]";
}