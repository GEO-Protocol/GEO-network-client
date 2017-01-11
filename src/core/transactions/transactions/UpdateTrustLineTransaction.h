#ifndef GEO_NETWORK_CLIENT_UPDATETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_UPDATETRUSTLINETRANSACTION_H

#include "UniqueTransaction.h"

#include "../../interface/commands/commands/UpdateTrustLineCommand.h"

#include "../scheduler/TransactionsScheduler.h"

#include "../../trust_lines/interface/TrustLinesInterface.h"

class UpdateTrustLineTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<UpdateTrustLineTransaction> Shared;

public:
    UpdateTrustLineTransaction(
            UpdateTrustLineCommand::Shared command,
            TransactionsScheduler *scheduler,
            TrustLinesInterface *interface);

    UpdateTrustLineCommand::Shared command() const;

    void setContext(
            Message::Shared message);

    pair<byte *, size_t> serializeContext();

    pair<CommandResult::SharedConst, TransactionState::SharedConst> run();

private:
    pair<CommandResult::SharedConst, TransactionState::SharedConst> conflictErrorResult();

private:
    UpdateTrustLineCommand::Shared mCommand;

    TrustLinesInterface *mTrustLinesInterface;
};


#endif //GEO_NETWORK_CLIENT_UPDATETRUSTLINETRANSACTION_H
