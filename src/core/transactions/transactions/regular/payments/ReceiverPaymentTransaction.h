#ifndef GEO_NETWORK_CLIENT_RECEIVERPAYMENTTRANSACTION_H
#define GEO_NETWORK_CLIENT_RECEIVERPAYMENTTRANSACTION_H


#include "base/BasePaymentTransaction.h"

#include "../../../../network/messages/outgoing/payments/ReceiverInitPaymentMessage.h"
#include "../../../../network/messages/outgoing/payments/ReceiverApprovePaymentMessage.h"


class ReceiverPaymentTransaction:
    public BasePaymentTransaction {

public:
    typedef shared_ptr<ReceiverPaymentTransaction> Shared;
    typedef shared_ptr<const ReceiverPaymentTransaction> ConstShared;

public:
    ReceiverPaymentTransaction(
        ReceiverInitPaymentMessage::Shared message,
        TrustLinesManager *trustLines,
        Logger *log);

    ReceiverPaymentTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLines,
        Logger *log);

    TransactionResult::SharedConst run();

    pair<BytesShared, size_t> serializeToBytes();

protected:
    // Stages handlers
    TransactionResult::Shared initOperation();
    TransactionResult::Shared processAmountReservationStage();

private:
    void deserializeFromBytes(
        BytesShared buffer);

    const string logHeader() const;

protected:
    ReceiverInitPaymentMessage::Shared mMessage;
    TrustLinesManager *mTrustLines;
    Logger *mLog;
};


#endif //GEO_NETWORK_CLIENT_RECEIVERPAYMENTTRANSACTION_H
