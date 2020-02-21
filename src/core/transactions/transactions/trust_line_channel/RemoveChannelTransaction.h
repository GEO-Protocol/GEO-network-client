#ifndef GEO_NETWORK_CLIENT_REMOVECHANNELTRANSACTION_H
#define GEO_NETWORK_CLIENT_REMOVECHANNELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_line_channels/RemoveChannelCommand.h"

#include "../../../contractors/ContractorsManager.h"

class RemoveChannelTransaction : public BaseTransaction {

public:
    typedef shared_ptr<RemoveChannelTransaction> Shared;

public:
    RemoveChannelTransaction(
        RemoveChannelCommand::Shared command,
        ContractorsManager *contractorsManager,
        StorageHandler *storageHandler,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultProtocolError();

    TransactionResult::SharedConst resultContractorIsAbsent();

    TransactionResult::SharedConst resultUnexpectedError();

protected:
    const string logHeader() const override;

private:
    RemoveChannelCommand::Shared mCommand;
    ContractorsManager *mContractorsManager;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_REMOVECHANNELTRANSACTION_H
