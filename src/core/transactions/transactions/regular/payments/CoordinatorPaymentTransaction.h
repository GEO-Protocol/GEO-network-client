#ifndef GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
#define GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H

#include "../../BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../interface/commands_interface/commands/payments/CreditUsageCommand.h"


class CoordinatorPaymentTransaction:
    public BaseTransaction {

public:
    typedef shared_ptr<CoordinatorPaymentTransaction> Shared;
    typedef shared_ptr<const CoordinatorPaymentTransaction> ConstShared;

public:
    CoordinatorPaymentTransaction(
        NodeUUID &currentNodeUUID,
        CreditUsageCommand::Shared command,
        TrustLinesManager *trustLines);

    CoordinatorPaymentTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLines);

    TransactionResult::Shared run();

    pair<BytesShared, size_t> serializeToBytes();

protected:
    CreditUsageCommand::Shared mCommand;
    TrustLinesManager *mTrustLines;

protected:
    TransactionResult::Shared resultOK() const;

    void deserializeFromBytes(
        BytesShared buffer);
};


#endif //GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
