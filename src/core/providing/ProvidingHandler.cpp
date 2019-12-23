#include "ProvidingHandler.h"

ProvidingHandler::ProvidingHandler(
    vector<Provider::Shared> &providers,
    IOService &ioService,
    Contractor::Shared selfContractor,
    Logger &logger) :
    LoggerMixin(logger),
    mProviders(providers),
    mUpdatingAddressTimer(ioService),
    mCacheCleaningTimer(ioService),
    mSelfContractor(selfContractor)
{
#ifdef DEBUG_LOG_PROVIDING_HANDLER
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
                +kStartingAddressPeriodSeconds));
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
#ifdef DEBUG_LOG_PROVIDING_HANDLER
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
#ifdef DEBUG_LOG_PROVIDING_HANDLER
    debug() << "setCachedIPv4AddressForGNS " << gnsAddress->fullAddress() << " "
            << ipv4Address->fullAddress() << " " << mTimesCache.at(0).first;
#endif

    if (mTimesCache.size() == 1) {
        rescheduleCleaning();
    }
}

Provider::Shared ProvidingHandler::mainProvider() const
{
    return mProviders.at(0);
}

bool ProvidingHandler::isProvidersPresent() const
{
    return !mProviders.empty();
}

void ProvidingHandler::rescheduleCleaning()
{
#ifdef DEBUG_LOG_PROVIDING_HANDLER
    debug() << "rescheduleCleaning";
#endif
    if (mTimesCache.empty()) {
#ifdef DEBUG_LOG_PROVIDING_HANDLER
        debug() << "There are no cached addresses";
#endif
        return;
    }
    const auto kCleaningTimeout = mTimesCache.at(0).first - utc_now();
    mCacheCleaningTimer.expires_from_now(
        chrono::microseconds(
            kCleaningTimeout.total_microseconds()));
    mCacheCleaningTimer.async_wait(
        boost::bind(
            &ProvidingHandler::clearCachedAddresses,
            this));
}

void ProvidingHandler::clearCachedAddresses()
{
#ifdef DEBUG_LOG_PROVIDING_HANDLER
    debug() << "clearCahedAddresses " << mCachedAddresses.size() << " " << mTimesCache.size();
#endif
    auto now = utc_now();
    auto it = mTimesCache.begin();
    while (it != mTimesCache.end()) {
        if (now >= it->first) {
#ifdef DEBUG_LOG_PROVIDING_HANDLER
            if (mCachedAddresses.count(it->second) != 0) {
                debug() << "remove cache " << it->second << " " << mCachedAddresses[it->second]->fullAddress();
            }
#endif
            mCachedAddresses.erase(it->second);
            mTimesCache.erase(it);
        } else {
            break;
        }
    }

    if (!mTimesCache.empty()) {
        rescheduleCleaning();
    }
#ifdef DEBUG_LOG_PROVIDING_HANDLER
    debug() << "after cleaning " << mCachedAddresses.size() << " " << mTimesCache.size();
#endif
}

const string ProvidingHandler::logHeader() const
{
    return "[ProvidingHandler]";
}