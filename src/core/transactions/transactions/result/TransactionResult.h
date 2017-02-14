#ifndef GEO_NETWORK_CLIENT_TRANSACTIONRESULT_H
#define GEO_NETWORK_CLIENT_TRANSACTIONRESULT_H

#include "../../../interface/results_interface/result/CommandResult.h"
#include "../../../network/messages/result/MessageResult.h"
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
        MessageResultType = 2,
        TransactionStateType = 3,
    };

public:
    TransactionResult();

    TransactionResult(
        bool isFinishedSuccessful);

    TransactionResult(
        TransactionState::SharedConst transactionState);

    static TransactionResult::SharedConst finishSuccessfulWithoutResult();

    void setCommandResult(
        CommandResult::SharedConst commandResult);

    void setMessageResult(
        MessageResult::SharedConst messageResult);

    void setTransactionState(
        TransactionState::SharedConst transactionState);

    CommandResult::SharedConst commandResult() const;

    MessageResult::SharedConst messageResult() const;

    TransactionState::SharedConst state() const;

    const bool isFinishedSuccessfulWithoutResult() const;

    ResultType resultType() const;

private:
    CommandResult::SharedConst mCommandResult;
    MessageResult::SharedConst mMessageResult;
    TransactionState::SharedConst mTransactionState;
    bool mFinishedWithoutResult = false;
};
#endif //GEO_NETWORK_CLIENT_TRANSACTIONRESULT_H
