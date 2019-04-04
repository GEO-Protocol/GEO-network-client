#ifndef GEO_NETWORK_CLIENT_FEATURESMANAGER_H
#define GEO_NETWORK_CLIENT_FEATURESMANAGER_H

#include "../settings/Settings.h"
#include "../io/storage/StorageHandler.h"
#include "../logger/LoggerMixin.hpp"

class FeaturesManager : protected LoggerMixin {

public:
    FeaturesManager(
        const string& equivalentsRegistryAddress,
        StorageHandler *storageHandler,
        Logger& logger);

    string getEquivalentsRegistryAddress() const;

protected:
    const string logHeader() const override;

private:
    const string kEquivalentsRegistryAddressFieldName = "EQUIVALENTS_REGISTRY_ADDRESS";

private:
    StorageHandler *mStorageHandler;
    string mEquivalentsRegistryAddressValue;
};


#endif //GEO_NETWORK_CLIENT_FEATURESMANAGER_H
