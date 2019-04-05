#ifndef GEO_NETWORK_CLIENT_GETCONTRACTORLISTTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETCONTRACTORLISTTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_line_channels/ContractorListCommand.h"
#include "../../../contractors/ContractorsManager.h"

class GetContractorListTransaction : public BaseTransaction {

public:
    typedef shared_ptr<GetContractorListTransaction> Shared;

public:
    GetContractorListTransaction(
        ContractorListCommand::Shared command,
        ContractorsManager *contractorsManager,
        Logger &logger)
        noexcept;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    ContractorListCommand::Shared mCommand;
    ContractorsManager *mContractorsManager;
};


#endif //GEO_NETWORK_CLIENT_GETCONTRACTORLISTTRANSACTION_H
