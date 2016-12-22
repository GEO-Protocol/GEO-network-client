#ifndef GEO_NETWORK_CLIENT_USECREDITTRANSACTION_H
#define GEO_NETWORK_CLIENT_USECREDITTRANSACTION_H

#include "BaseTransaction.h"
#include "../../interface/commands/commands/UseCreditCommand.h"

class TransactionsScheduler;

class UseCreditTransaction : public BaseTransaction {
    friend class TransactionsScheduler;

private:
    UseCreditCommand::Shared mCommand;

public:
    UseCreditTransaction(
            UseCreditCommand::Shared command);

private:
    UseCreditCommand::Shared command() const;

    void setContext(
            Message::Shared message);

    pair<CommandResult::SharedConst, TransactionState::SharedConst> run();
};


#endif //GEO_NETWORK_CLIENT_USECREDITTRANSACTION_H
