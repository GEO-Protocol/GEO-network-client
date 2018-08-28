/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_TRANSACTIONRESULT_H
#define GEO_NETWORK_CLIENT_TRANSACTIONRESULT_H

#include "../../../interface/results_interface/result/CommandResult.h"
#include "state/TransactionState.h"

#include "../../../common/exceptions/ConflictError.h"

#include <memory>

using namespace std;

class TransactionResult {
public:
    typedef shared_ptr<TransactionResult> Shared;
    typedef shared_ptr<const TransactionResult> SharedConst;

public:
    enum ResultType {
        CommandResultType = 1,
        TransactionStateType = 2,
    };

public:
    TransactionResult();

    TransactionResult(
        TransactionState::SharedConst transactionState);

    void setCommandResult(
        CommandResult::SharedConst commandResult);

    void setTransactionState(
        TransactionState::SharedConst transactionState);

    CommandResult::SharedConst commandResult() const;

    TransactionState::SharedConst state() const;

    ResultType resultType() const;

private:
    CommandResult::SharedConst mCommandResult;
    TransactionState::SharedConst mTransactionState;
};
#endif //GEO_NETWORK_CLIENT_TRANSACTIONRESULT_H
