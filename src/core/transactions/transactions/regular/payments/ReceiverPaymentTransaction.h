#ifndef GEO_NETWORK_CLIENT_RECEIVERPAYMENTTRANSACTION_H
#define GEO_NETWORK_CLIENT_RECEIVERPAYMENTTRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../common/time/TimeUtils.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../logger/Logger.h"

#include "../../../../interface/commands_interface/commands/payments/CreditUsageCommand.h"
#include "../../../../network/messages/outgoing/payments/ReceiverInitPaymentMessage.h"
#include "../../../../network/messages/outgoing/payments/ReceiverApproveMessage.h"


class ReceiverPaymentTransaction:
    public BaseTransaction {

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
//    TransactionResult::Shared processAmountBlockingStage();

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
