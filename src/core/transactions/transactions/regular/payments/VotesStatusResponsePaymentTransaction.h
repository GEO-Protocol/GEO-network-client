#ifndef GEO_NETWORK_CLIENT_VOUTESSTATUSRESPONSEPAYMENTTRANSACTION_H
#define GEO_NETWORK_CLIENT_VOUTESSTATUSRESPONSEPAYMENTTRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../network/messages/payments/VotesStatusRequestMessage.hpp"
#include "../../../../network/messages/payments/ParticipantsVotesMessage.h"

class VotesStatusResponsePaymentTransaction:
        public BaseTransaction{
public:
    VotesStatusResponsePaymentTransaction(
            const NodeUUID &nodeUUID,
            VotesStatusRequestMessage::Shared message,
            StorageHandler *storageHandler,
            Logger &logger
    );
    TransactionResult::SharedConst run();

protected:
    VotesStatusRequestMessage::Shared mRequest;
    StorageHandler *mStorageHandler;
};
#endif //GEO_NETWORK_CLIENT_VOUTESSTATUSRESPONSEPAYMENTTRANSACTION_H
