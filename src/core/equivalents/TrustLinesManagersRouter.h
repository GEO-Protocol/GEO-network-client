#ifndef GEO_NETWORK_CLIENT_TRUSTLINESMANAGERSROUTER_H
#define GEO_NETWORK_CLIENT_TRUSTLINESMANAGERSROUTER_H

#include "../trust_lines/manager/TrustLinesManager.h"

class TrustLinesManagersRouter {

public:
    TrustLinesManagersRouter(
        StorageHandler *storageHandler,
        Logger &logger);

private:
    StorageHandler *mStorageHandler;
    Logger &mLogger;
    vector<SerializedEquivalent> mEquivalents;
    map<SerializedEquivalent, unique_ptr<TrustLinesManager>> mTrustLinesManagers;
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESMANAGERSROUTER_H
