#ifndef GEO_NETWORK_CLIENT_MAXIMALAMOUNTTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXIMALAMOUNTTRANSACTION_H

#include "BaseTransaction.h"
#include "../../interface/commands/commands/MaximalTransactionAmountCommand.h"

class TransactionsScheduler;

class MaximalAmountTransaction : public BaseTransaction {
    friend class TransactionsScheduler;

private:
    MaximalTransactionAmountCommand::Shared mCommand;

public:
    MaximalAmountTransaction(
            MaximalTransactionAmountCommand::Shared command);

private:
    MaximalTransactionAmountCommand::Shared command() const;

    void setContext(
            Message::Shared message);

    pair<CommandResult::SharedConst, TransactionState::SharedConst> run();
};


#endif //GEO_NETWORK_CLIENT_MAXIMALAMOUNTTRANSACTION_H
