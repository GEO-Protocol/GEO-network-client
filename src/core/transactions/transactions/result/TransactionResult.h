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
        HybridType = 3,
    };

public:
    TransactionResult();

    TransactionResult(
        TransactionState::SharedConst transactionState);

    TransactionResult(
        CommandResult::SharedConst commandResult);

    TransactionResult(
        TransactionState::SharedConst transactionState,
        CommandResult::SharedConst commandResult);

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
