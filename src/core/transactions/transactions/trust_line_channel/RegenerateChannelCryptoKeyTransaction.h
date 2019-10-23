#ifndef GEO_NETWORK_CLIENT_REGENERATECHANNELCRYPTOKEYTRANSACTION_H
#define GEO_NETWORK_CLIENT_REGENERATECHANNELCRYPTOKEYTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_line_channels/RegenerateChannelCryptoKeyCommand.h"

#include "../../../contractors/ContractorsManager.h"

class RegenerateChannelCryptoKeyTransaction : public BaseTransaction {

public:
    typedef shared_ptr<RegenerateChannelCryptoKeyTransaction> Shared;

public:
    RegenerateChannelCryptoKeyTransaction(
        RegenerateChannelCryptoKeyCommand::Shared command,
        ContractorsManager *contractorsManager,
        StorageHandler *storageHandler,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultForbiddenRun();

    TransactionResult::SharedConst resultContractorIsAbsent();

    TransactionResult::SharedConst resultProtocolError();

    TransactionResult::SharedConst resultUnexpectedError();

protected:
    const string logHeader() const override;

private:
    RegenerateChannelCryptoKeyCommand::Shared mCommand;
    ContractorsManager *mContractorsManager;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_REGENERATECHANNELCRYPTOKEYTRANSACTION_H
