#include "FeaturesManager.h"

FeaturesManager::FeaturesManager(
    as::io_service &ioService,
    const string& equivalentsRegistryAddress,
    const string& ownAddresses,
    StorageHandler *storageHandler,
    Logger &logger):
    LoggerMixin(logger),
    mIOService(ioService),
    mStorageHandler(storageHandler)
{
    if (equivalentsRegistryAddress.empty()) {
        throw ValueError("Equivalents registry address is empty");
    }
    mEquivalentsRegistryAddressValue = equivalentsRegistryAddress;
    info() << "Equivalents registry address: " << mEquivalentsRegistryAddressValue;
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        auto dataBaseEquivalentRegistryAddress = ioTransaction->featuresHandler()->getFeature(
            kEquivalentsRegistryAddressFieldName);
        if (dataBaseEquivalentRegistryAddress != mEquivalentsRegistryAddressValue) {
            throw ValueError("There is different value of " +
                kEquivalentsRegistryAddressFieldName + " feature in database: " + dataBaseEquivalentRegistryAddress);
        }
    } catch (NotFoundError &e) {
        info() << "There is no feature " << kEquivalentsRegistryAddressFieldName << " yet. Add it";
        ioTransaction->featuresHandler()->saveFeature(
            kEquivalentsRegistryAddressFieldName,
            mEquivalentsRegistryAddressValue);
    } catch (IOError &e) {
        warning() << "Can't read " << kEquivalentsRegistryAddressFieldName << " feature from storage";
        throw e;
    }

    if (ownAddresses.empty()) {
        throw ValueError("Own addresses are empty");
    }
    try {
        auto dataBaseOwnAddresses = ioTransaction->featuresHandler()->getFeature(
            kOwnAddressesFieldName);
        if (dataBaseOwnAddresses != ownAddresses) {
            info() << "There is different value of " << kOwnAddressesFieldName
                   << " feature in database: " << dataBaseOwnAddresses;
            ioTransaction->featuresHandler()->saveFeature(
                kOwnAddressesFieldName,
                ownAddresses);
            mNotificationTimer = make_unique<as::steady_timer>(
                mIOService);
            mNotificationTimer->expires_from_now(
                chrono::seconds(
                    kSignalTimerPeriodSeconds));
            mNotificationTimer->async_wait(
                boost::bind(
                    &FeaturesManager::runSignalNotify,
                    this,
                    as::placeholders::error));
        }
    } catch (NotFoundError &e) {
        info() << "There is no feature " << kOwnAddressesFieldName << " yet. Add it";
        ioTransaction->featuresHandler()->saveFeature(
            kOwnAddressesFieldName,
            ownAddresses);
    } catch (IOError &e) {
        warning() << "Can't read " << kOwnAddressesFieldName << " feature from storage";
        throw e;
    }
}

string FeaturesManager::getEquivalentsRegistryAddress() const
{
    return mEquivalentsRegistryAddressValue;
}

void FeaturesManager::runSignalNotify(
    const boost::system::error_code &errorCode)
{
    if (errorCode) {
        warning() << errorCode.message().c_str();
    }
    info() << "run address update notification signal";
    mNotificationTimer->cancel();
    sendAddressesSignal();
}

const string FeaturesManager::logHeader() const
{
    return "[FeaturesManager]";
}