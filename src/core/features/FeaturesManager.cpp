#include "FeaturesManager.h"

FeaturesManager::FeaturesManager(
    const string& equivalentsRegistryAddress,
    StorageHandler *storageHandler,
    Logger &logger):
    LoggerMixin(logger),
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
}

string FeaturesManager::getEquivalentsRegistryAddress() const
{
    return mEquivalentsRegistryAddressValue;
}

const string FeaturesManager::logHeader() const
{
    return "[FeaturesManager]";
}