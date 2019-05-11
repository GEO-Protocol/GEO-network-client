#ifndef GEO_NETWORK_CLIENT_UPDATECHANNELADDRESSESTARGETTRANSACTION_H
#define GEO_NETWORK_CLIENT_UPDATECHANNELADDRESSESTARGETTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../network/messages/trust_line_channels/UpdateChannelAddressesMessage.h"
#include "../../../contractors/ContractorsManager.h"
#include "../../../io/storage/StorageHandler.h"

class UpdateChannelAddressesTargetTransaction : public BaseTransaction {

public:
    typedef shared_ptr<UpdateChannelAddressesTargetTransaction> Shared;

public:
    UpdateChannelAddressesTargetTransaction(
        UpdateChannelAddressesMessage::Shared message,
        ContractorsManager *contractorsManager,
        StorageHandler *storageHandler,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    const string logHeader() const override;

private:
    UpdateChannelAddressesMessage::Shared mMessage;
    ContractorsManager *mContractorsManager;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_UPDATECHANNELADDRESSESTARGETTRANSACTION_H
