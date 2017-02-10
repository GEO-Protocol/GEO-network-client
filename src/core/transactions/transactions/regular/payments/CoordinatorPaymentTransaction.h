#ifndef GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
#define GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../logger/Logger.h"

#include "../../../../interface/commands_interface/commands/payments/CreditUsageCommand.h"
#include "../../../../network/messages/outgoing/payments/ReceiverInitPaymentMessage.h"


class CoordinatorPaymentTransaction:
    public BaseTransaction {

public:
    typedef shared_ptr<CoordinatorPaymentTransaction> Shared;
    typedef shared_ptr<const CoordinatorPaymentTransaction> ConstShared;

public:
    CoordinatorPaymentTransaction(
        NodeUUID &currentNodeUUID,
        CreditUsageCommand::Shared command,
        TrustLinesManager *trustLines,
        Logger *log);

    CoordinatorPaymentTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLines,
        Logger *log);

    TransactionResult::SharedConst run();

    pair<BytesShared, size_t> serializeToBytes();

private:
    TransactionResult::Shared initOperation();
    TransactionResult::Shared processReceiverResponse();


    TransactionResult::Shared resultOK() const;

    void deserializeFromBytes(
        BytesShared buffer);

protected:
    CreditUsageCommand::Shared mCommand;
    TrustLinesManager *mTrustLines;
    Logger *mLog;
};


#endif //GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
