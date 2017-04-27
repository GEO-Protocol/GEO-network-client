#ifndef GEO_NETWORK_CLIENT_VOUTESSTATUSRESPONSEPAYMENTTRANSACTION_H
#define GEO_NETWORK_CLIENT_VOUTESSTATUSRESPONSEPAYMENTTRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../network/messages/payments/VotesStatusRequestMessage.hpp"
#include "../../../../network/messages/payments/ParticipantsVotesMessage.h"

class VoutesStatusResponsePaymentTransaction:
        public BaseTransaction{
public:
    VoutesStatusResponsePaymentTransaction(
            const NodeUUID &nodeUUID,
            VotesStatusRequestMessage::Shared message,
            StorageHandler *storageHandler,
            Logger *logger
    );
    TransactionResult::SharedConst run();

protected:
    VotesStatusRequestMessage::Shared mRequestMessage;
    StorageHandler *mStorageHandler;
};
#endif //GEO_NETWORK_CLIENT_VOUTESSTATUSRESPONSEPAYMENTTRANSACTION_H
