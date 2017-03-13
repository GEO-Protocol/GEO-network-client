#ifndef GEO_NETWORK_CLIENT_RECEIVERPAYMENTTRANSACTION_H
#define GEO_NETWORK_CLIENT_RECEIVERPAYMENTTRANSACTION_H


#include "base/BasePaymentTransaction.h"

#include "../../../../network/messages/outgoing/payments/ReceiverInitPaymentRequestMessage.h"
#include "../../../../network/messages/outgoing/payments/ReceiverInitPaymentResponseMessage.h"
#include "../../../../network/messages/outgoing/payments/IntermediateNodeReservationRequestMessage.h"
#include "../../../../network/messages/outgoing/payments/IntermediateNodeReservationResponseMessage.h"


class ReceiverPaymentTransaction:
    public BasePaymentTransaction {

public:
    typedef shared_ptr<ReceiverPaymentTransaction> Shared;
    typedef shared_ptr<const ReceiverPaymentTransaction> ConstShared;

public:
    ReceiverPaymentTransaction(
        const NodeUUID &currentNodeUUID,
        ReceiverInitPaymentRequestMessage::ConstShared message,
        TrustLinesManager *trustLines,
        Logger *log);

    ReceiverPaymentTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLines,
        Logger *log);

    TransactionResult::SharedConst run();

    pair<BytesShared, size_t> serializeToBytes();

protected:
    enum Stages {
        CoordinatorRequestApprooving = 1,
        AmountReservationsProcessing,
    };

    TransactionResult::Shared initOperation();
    TransactionResult::Shared processAmountReservationStage();

private:
    void deserializeFromBytes(
        BytesShared buffer);

    const string logHeader() const;

protected:
    ReceiverInitPaymentRequestMessage::ConstShared mMessage;
    Logger *mLog;
};


#endif //GEO_NETWORK_CLIENT_RECEIVERPAYMENTTRANSACTION_H
