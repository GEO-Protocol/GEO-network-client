#ifndef GEO_NETWORK_CLIENT_SETCHANNELCONTRACTORCRYPTOKEYTRANSACTION_H
#define GEO_NETWORK_CLIENT_SETCHANNELCONTRACTORCRYPTOKEYTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_line_channels/SetChannelContractorCryptoKeyCommand.h"

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
    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultProtocolError();

    TransactionResult::SharedConst resultContractorIsAbsent();

    TransactionResult::SharedConst resultUnexpectedError();

protected:
    const string logHeader() const override;

private:
    SetChannelContractorCryptoKeyCommand::Shared mCommand;
    Contractor::Shared mContractor;
    ContractorsManager *mContractorsManager;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_SETCHANNELCONTRACTORCRYPTOKEYTRANSACTION_H
