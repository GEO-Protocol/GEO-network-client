#include "TrustLinesManagersRouter.h"

TrustLinesManagersRouter::TrustLinesManagersRouter(
    StorageHandler *storageHandler,
    Logger &logger):
    mStorageHandler(storageHandler),
    mLogger(logger)
{
    auto ioTransaction = storageHandler->beginTransaction();
    auto allEquivalents = ioTransaction->trustLinesHandler()->equivalents();
    for (const auto &equivalent : allEquivalents) {
        mTrustLinesManagers.insert(
            make_pair(
                equivalent,
                make_unique<TrustLinesManager>(
                    equivalent,
                    mStorageHandler,
                    mLogger)));
    }
}
