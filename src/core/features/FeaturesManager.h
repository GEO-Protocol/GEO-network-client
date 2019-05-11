#ifndef GEO_NETWORK_CLIENT_FEATURESMANAGER_H
#define GEO_NETWORK_CLIENT_FEATURESMANAGER_H

#include "../settings/Settings.h"
#include "../io/storage/StorageHandler.h"
#include "../logger/LoggerMixin.hpp"

#include <boost/asio/steady_timer.hpp>
#include <boost/signals2.hpp>
#include <boost/asio.hpp>

using namespace std;
namespace as = boost::asio;
namespace signals = boost::signals2;

class FeaturesManager : protected LoggerMixin {

public:
    typedef signals::signal<void()> SendAddressesSignal;

public:
    FeaturesManager(
        as::io_service &ioService,
        const string& equivalentsRegistryAddress,
        const string& ownAddresses,
        StorageHandler *storageHandler,
        Logger& logger);

    string getEquivalentsRegistryAddress() const;

public:
    mutable SendAddressesSignal sendAddressesSignal;

protected:
    const string logHeader() const override;

private:
    void runSignalNotify(
        const boost::system::error_code &error);

private:
    const string kEquivalentsRegistryAddressFieldName = "EQUIVALENTS_REGISTRY_ADDRESS";
    const string kOwnAddressesFieldName = "OWN_ADDRESSES";
    static const uint32_t kSignalTimerPeriodSeconds = 15;

private:
    StorageHandler *mStorageHandler;
    string mEquivalentsRegistryAddressValue;
    as::io_service &mIOService;
    unique_ptr<as::steady_timer> mNotificationTimer;
};


#endif //GEO_NETWORK_CLIENT_FEATURESMANAGER_H
