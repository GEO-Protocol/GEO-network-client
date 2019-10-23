#ifndef GEO_NETWORK_CLIENT_REMOVETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_REMOVETRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_lines/RemoveTrustLineCommand.h"
#include "../../../contractors/ContractorsManager.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../crypto/keychain.h"

class RemoveTrustLineTransaction : public BaseTransaction {

public:
    typedef shared_ptr<RemoveTrustLineTransaction> Shared;

public:
    RemoveTrustLineTransaction(
        RemoveTrustLineCommand::Shared command,
        ContractorsManager *contractorsManager,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    const string logHeader() const override;

protected:
    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultProtocolError();

private:
    RemoveTrustLineCommand::Shared mCommand;
    ContractorsManager *mContractorsManager;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    Keystore *mKeysStore;

    ContractorID mContractorID;
};


#endif //GEO_NETWORK_CLIENT_REMOVETRUSTLINETRANSACTION_H
