#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H

#include "BaseTransaction.h"
#include "../../interface/commands/commands/CloseTrustLineCommand.h"

class CloseTrustLineTransaction : public BaseTransaction {
    friend class TransactionsScheduler;

private:
    CloseTrustLineCommand::Shared mCommand;

public:
    CloseTrustLineTransaction(
            CloseTrustLineCommand::Shared command);

private:
    CloseTrustLineCommand::Shared command() const;

    void setContext(
            Message::Shared message);

    pair<CommandResult::SharedConst, TransactionState::SharedConst> run();

    pair<byte *, size_t> serializeContext();
};



#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
