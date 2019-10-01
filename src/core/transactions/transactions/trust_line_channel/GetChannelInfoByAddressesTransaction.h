#ifndef GEO_NETWORK_CLIENT_GETCHANNELINFOBYADDRESSESTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETCHANNELINFOBYADDRESSESTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_line_channels/GetChannelInfoByAddressesCommand.h"
#include "../../../contractors/ContractorsManager.h"

class GetChannelInfoByAddressesTransaction : public BaseTransaction {

public:
    typedef shared_ptr<GetChannelInfoByAddressesTransaction> Shared;

public:
    GetChannelInfoByAddressesTransaction(
        GetChannelInfoByAddressesCommand::Shared command,
        ContractorsManager *contractorsManager,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    TransactionResult::SharedConst resultChannelIsAbsent();

    const string logHeader() const override;

private:
    GetChannelInfoByAddressesCommand::Shared mCommand;
    ContractorsManager *mContractorsManager;
};


#endif //GEO_NETWORK_CLIENT_GETCHANNELINFOBYADDRESSESTRANSACTION_H
