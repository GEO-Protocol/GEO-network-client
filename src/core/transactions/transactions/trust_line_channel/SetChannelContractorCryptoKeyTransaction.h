#ifndef GEO_NETWORK_CLIENT_SETCHANNELCONTRACTORCRYPTOKEYTRANSACTION_H
#define GEO_NETWORK_CLIENT_SETCHANNELCONTRACTORCRYPTOKEYTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_line_channels/SetChannelContractorCryptoKeyCommand.h"
#include "../../../network/messages/trust_line_channels/ConfirmChannelMessage.h"
#include "../../../network/messages/trust_line_channels/InitChannelMessage.h"

#include "../../../contractors/ContractorsManager.h"

class SetChannelContractorCryptoKeyTransaction : public BaseTransaction {

public:
    typedef shared_ptr<SetChannelContractorCryptoKeyTransaction> Shared;

public:
    SetChannelContractorCryptoKeyTransaction(
        SetChannelContractorCryptoKeyCommand::Shared command,
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
    TransactionResult::SharedConst resultOKAndWaitResponse();

    TransactionResult::SharedConst resultProtocolError();

    TransactionResult::SharedConst resultContractorIsAbsent();

    TransactionResult::SharedConst resultUnexpectedError();

private:
    TransactionResult::SharedConst runInitializationStage();

    TransactionResult::SharedConst runResponseProcessingStage();

protected:
    const string logHeader() const override;

private:
    static const uint32_t kWaitMillisecondsForResponse = 10000;
    static const uint16_t kMaxCountSendingAttempts = 3;

private:
    SetChannelContractorCryptoKeyCommand::Shared mCommand;
    ContractorsManager *mContractorsManager;
    StorageHandler *mStorageHandler;

    uint16_t mCountSendingAttempts;
};


#endif //GEO_NETWORK_CLIENT_SETCHANNELCONTRACTORCRYPTOKEYTRANSACTION_H
