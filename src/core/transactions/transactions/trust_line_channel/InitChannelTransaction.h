#ifndef GEO_NETWORK_CLIENT_INITCHANNELTRANSACTION_H
#define GEO_NETWORK_CLIENT_INITCHANNELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_line_channels/InitChannelCommand.h"
#include "../../../network/messages/trust_line_channels/InitChannelMessage.h"

#include "../../../contractors/ContractorsManager.h"

class InitChannelTransaction : public BaseTransaction {

public:
    typedef shared_ptr<InitChannelTransaction> Shared;

public:
    InitChannelTransaction(
        InitChannelCommand::Shared command,
        ContractorsManager *contractorsManager,
        StorageHandler *storageHandler,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultForbiddenRun();

    TransactionResult::SharedConst resultProtocolError();

    TransactionResult::SharedConst resultUnexpectedError();

protected: // trust lines history shortcuts
    const string logHeader() const
    noexcept;

protected:
    InitChannelCommand::Shared mCommand;
    Contractor::Shared mContractor;
    ContractorsManager *mContractorsManager;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_INITCHANNELTRANSACTION_H
