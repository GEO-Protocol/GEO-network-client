#ifndef GEO_NETWORK_CLIENT_CONFIRMCHANNELTRANSACTION_H
#define GEO_NETWORK_CLIENT_CONFIRMCHANNELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../network/messages/trust_line_channels/InitChannelMessage.h"
#include "../../../network/messages/base/transaction/ConfirmationMessage.h"

#include "../../../contractors/ContractorsManager.h"

class ConfirmChannelTransaction : public BaseTransaction {

public:
    typedef shared_ptr<ConfirmChannelTransaction> Shared;

public:
    ConfirmChannelTransaction(
        InitChannelMessage::Shared message,
        ContractorsManager *contractorsManager,
        StorageHandler *storageHandler,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    const string logHeader() const override;

protected:
    InitChannelMessage::Shared mMessage;
    ContractorsManager *mContractorsManager;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_CONFIRMCHANNELTRANSACTION_H
