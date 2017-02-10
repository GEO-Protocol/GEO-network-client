#ifndef GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
#define GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H

#include "../../BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../interface/commands_interface/commands/payments/CreditUsageCommand.h"


class CoordinatorPaymentTransaction: public BaseTransaction {

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

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

protected:
    void deserializeFromBytes(
        BytesShared buffer);

    TransactionResult::SharedConst resultOK() const;

protected:
    CreditUsageCommand::Shared mCommand;
    TrustLinesManager *mTrustLines;
};


#endif //GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
