#ifndef GEO_NETWORK_CLIENT_VOUTESSTATUSRESPONSEPAYMENTTRANSACTION_H
#define GEO_NETWORK_CLIENT_VOUTESSTATUSRESPONSEPAYMENTTRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../network/messages/payments/VotesStatusRequestMessage.hpp"
#include "../../../../network/messages/payments/ParticipantsVotesMessage.h"
#include "../../../../crypto/lamportscheme.h"

using namespace crypto;

class VotesStatusResponsePaymentTransaction:
        public BaseTransaction{
public:
    VotesStatusResponsePaymentTransaction(
        const NodeUUID &nodeUUID,
        VotesStatusRequestMessage::Shared message,
        StorageHandler *storageHandler,
        bool isRequestedTransactionCurrentlyRunned,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

protected:
    VotesStatusRequestMessage::Shared mRequest;
    StorageHandler *mStorageHandler;
    bool mIsRequestedTransactionCurrentlyRunned;
};
#endif //GEO_NETWORK_CLIENT_VOUTESSTATUSRESPONSEPAYMENTTRANSACTION_H
