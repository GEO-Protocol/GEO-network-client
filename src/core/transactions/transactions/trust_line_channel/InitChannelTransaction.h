#ifndef GEO_NETWORK_CLIENT_INITCHANNELTRANSACTION_H
#define GEO_NETWORK_CLIENT_INITCHANNELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_line_channels/InitChannelCommand.h"
#include "../../../network/messages/trust_line_channels/InitChannelMessage.h"
#include "../../../network/messages/trust_line_channels/ConfirmChannelMessage.h"

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

    TransactionResult::SharedConst run() override;

protected:
    enum Stages {
        Initialization = 1,
        ResponseProcessing = 2,
    };

protected:
    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultOKAndWaitMessage();

    TransactionResult::SharedConst resultProtocolError();

    TransactionResult::SharedConst resultUnexpectedError();

private:
    TransactionResult::SharedConst runInitializationStage();

    TransactionResult::SharedConst runResponseProcessingStage();

protected:
    const string logHeader() const override;

protected:
    static const uint32_t kWaitMillisecondsForResponse = 10000;
    static const uint16_t kMaxCountSendingAttempts = 3;

private:
    InitChannelCommand::Shared mCommand;
    Contractor::Shared mContractor;
    ContractorsManager *mContractorsManager;
    StorageHandler *mStorageHandler;

    uint16_t mCountSendingAttempts;
};


#endif //GEO_NETWORK_CLIENT_INITCHANNELTRANSACTION_H
