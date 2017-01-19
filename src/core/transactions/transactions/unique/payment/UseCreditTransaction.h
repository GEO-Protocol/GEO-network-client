#ifndef GEO_NETWORK_CLIENT_USECREDITTRANSACTION_H
#define GEO_NETWORK_CLIENT_USECREDITTRANSACTION_H

#include "../../BaseTransaction.h"

#include "../../../../interface/commands/commands/UseCreditCommand.h"

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

    pair<byte *, size_t> serializeContext();

    TransactionResult::Shared run();
};


#endif //GEO_NETWORK_CLIENT_USECREDITTRANSACTION_H
