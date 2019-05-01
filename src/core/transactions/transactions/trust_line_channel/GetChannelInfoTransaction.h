#ifndef GEO_NETWORK_CLIENT_GETCHANNELINFOTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETCHANNELINFOTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_line_channels/GetChannelInfoCommand.h"
#include "../../../contractors/ContractorsManager.h"

class GetChannelInfoTransaction : public BaseTransaction {

public:
    typedef shared_ptr<GetChannelInfoTransaction> Shared;

public:
    GetChannelInfoTransaction(
        GetChannelInfoCommand::Shared command,
        ContractorsManager *contractorsManager,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    TransactionResult::SharedConst resultContractorIsAbsent();

    const string logHeader() const override;

private:
    GetChannelInfoCommand::Shared mCommand;
    ContractorsManager *mContractorsManager;
};


#endif //GEO_NETWORK_CLIENT_GETCHANNELINFOTRANSACTION_H
