#ifndef BASEPAYMENTTRANSACTION_H
#define BASEPAYMENTTRANSACTION_H


#include "../../../base/BaseTransaction.h"

#include "../../../../../common/Types.h"
#include "../../../../../paths/lib/Path.h"

#include "../../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../../logger/Logger.h"


class BasePaymentTransaction:
    public BaseTransaction {

public:
    using BaseTransaction::BaseTransaction;

protected:
    // Specifies how long node must wait for the response from the remote node.
    // This timeout must take into account also that remote node may process other transaction,
    // and may be too busy to response.
    // (it is not only network transfer timeout).
    static const auto kMaxMessageTransferLagMSec = 1500; // msec

    static const auto kMaxNodesCount = 6;
};

#endif // BASEPAYMENTTRANSACTION_H
