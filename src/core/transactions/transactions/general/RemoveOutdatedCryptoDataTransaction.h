#ifndef GEO_NETWORK_CLIENT_REMOVEOUTDATEDCRYPTODATATRANSACTION_H
#define GEO_NETWORK_CLIENT_REMOVEOUTDATEDCRYPTODATATRANSACTION_H

#include "../base/BaseTransaction.h"

#include "../../../equivalents/EquivalentsSubsystemsRouter.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../crypto/keychain.h"

#include "../../../interface/commands_interface/commands/general/RemoveOutdatedCryptoDataCommand.h"

class RemoveOutdatedCryptoDataTransaction : public BaseTransaction {

public:
    typedef shared_ptr<RemoveOutdatedCryptoDataTransaction> Shared;

public:
    RemoveOutdatedCryptoDataTransaction(
        EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
        StorageHandler *storageHandler,
        Keystore *keystore,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    const string logHeader() const override;

private:
    static const AuditNumber kOutdatedDifference = 2;

private:
    EquivalentsSubsystemsRouter *mEquivalentsSubsystemsRouter;
    StorageHandler *mStorageHandler;
    Keystore *mKeysStore;
};


#endif //GEO_NETWORK_CLIENT_REMOVEOUTDATEDCRYPTODATATRANSACTION_H
