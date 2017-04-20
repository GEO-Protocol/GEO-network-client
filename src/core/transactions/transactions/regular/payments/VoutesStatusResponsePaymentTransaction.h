#ifndef GEO_NETWORK_CLIENT_VOUTESSTATUSRESPONSEPAYMENTTRANSACTION_H
#define GEO_NETWORK_CLIENT_VOUTESSTATUSRESPONSEPAYMENTTRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../io/storage/PaymentOperationStateHandler.h"
#include "../../../../network/messages/payments/VotesStatusRequestMessage.hpp"
#include "../../../../network/messages/payments/ParticipantsVotesMessage.h"

class VoutesStatusResponsePaymentTransaction:
        public BaseTransaction{
public:
    VoutesStatusResponsePaymentTransaction(
            const NodeUUID &nodeUUID,
            VotesStatusRequestMessage::Shared message,
            PaymentOperationStateHandler *paymentOperationStateHandler,
            Logger *logger
    );
    TransactionResult::SharedConst run();

protected:
    VotesStatusRequestMessage::Shared mRequestMessage;
    PaymentOperationStateHandler *mPaymentOperationStateHandler;
    Logger *mLogger;
};
#endif //GEO_NETWORK_CLIENT_VOUTESSTATUSRESPONSEPAYMENTTRANSACTION_H
