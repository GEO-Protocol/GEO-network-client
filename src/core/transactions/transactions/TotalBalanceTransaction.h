#ifndef GEO_NETWORK_CLIENT_TOTALBALANCETRANSACTION_H
#define GEO_NETWORK_CLIENT_TOTALBALANCETRANSACTION_H

#include "BaseTransaction.h"
#include "../../interface/commands/commands/TotalBalanceCommand.h"

class TransactionsScheduler;

class TotalBalanceTransaction : public BaseTransaction {
    friend class TransactionsScheduler;

private:
    TotalBalanceCommand::Shared mCommand;

public:
    TotalBalanceTransaction(
            TotalBalanceCommand::Shared command);

private:
    TotalBalanceCommand::Shared command() const;

    void setContext(
            Message::Shared message);

    pair<CommandResult::SharedConst, TransactionState::SharedConst> run();
};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCETRANSACTION_H
