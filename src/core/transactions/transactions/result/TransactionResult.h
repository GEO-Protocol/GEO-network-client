#ifndef GEO_NETWORK_CLIENT_TRANSACTIONRESULT_H
#define GEO_NETWORK_CLIENT_TRANSACTIONRESULT_H

#include "../../../interface/results/result/CommandResult.h"
#include "../../../network/messages/result/MessageResult.h"
#include "state/TransactionState.h"

#include <memory>

using namespace std;

class TransactionResult {
public:
    typedef shared_ptr<TransactionResult> Shared;
    typedef shared_ptr<const TransactionResult> SharedConst;

public:
    enum ResultType {
        CommandResultType = 1,
        MessageResultType = 2
    };

public:
    TransactionResult();

    void setCommandResult(
        CommandResult::Shared commandResult);

    void setMessageResult(
        MessageResult::Shared messageResult);

    void setTransactionState(
        TransactionState::Shared transactionState);

    CommandResult::SharedConst commandResult() const;

    MessageResult::SharedConst messageResult() const;

    TransactionState::SharedConst transactionState() const;

    ResultType resultType() const;


private:
    CommandResult::Shared mCommandResult;
    MessageResult::Shared mMessageResult;
    TransactionState::Shared mTransactionState;
};


#endif //GEO_NETWORK_CLIENT_TRANSACTIONRESULT_H
