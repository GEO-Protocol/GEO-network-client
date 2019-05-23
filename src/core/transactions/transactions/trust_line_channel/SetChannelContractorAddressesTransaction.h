#ifndef GEO_NETWORK_CLIENT_SETCHANNELCONTRACTORADDRESSESTRANSACTION_H
#define GEO_NETWORK_CLIENT_SETCHANNELCONTRACTORADDRESSESTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_line_channels/SetChannelContractorAddressesCommand.h"

#include "../../../contractors/ContractorsManager.h"

class SetChannelContractorAddressesTransaction : public BaseTransaction {

public:
    typedef shared_ptr<SetChannelContractorAddressesTransaction> Shared;

public:
    SetChannelContractorAddressesTransaction(
        SetChannelContractorAddressesCommand::Shared command,
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
    SetChannelContractorAddressesCommand::Shared mCommand;
    Contractor::Shared mContractor;
    ContractorsManager *mContractorsManager;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_SETCHANNELCONTRACTORADDRESSESTRANSACTION_H
