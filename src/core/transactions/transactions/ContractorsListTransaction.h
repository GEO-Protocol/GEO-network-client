#ifndef GEO_NETWORK_CLIENT_CONTRACTORSLISTTRANSACTION_H
#define GEO_NETWORK_CLIENT_CONTRACTORSLISTTRANSACTION_H

#include "BaseTransaction.h"

#include "../../interface/commands/commands/ContractorsListCommand.h"

class ContractorsListTransaction : public BaseTransaction {
    friend class TransactionsScheduler;

private:
    ContractorsListCommand::Shared mCommand;

public:
    ContractorsListTransaction(
            ContractorsListCommand::Shared command);

private:
    ContractorsListCommand::Shared command() const;

    void setContext(
            Message::Shared message);

    pair<byte *, size_t> serializeContext();

    TransactionResult::Shared run();
};


#endif //GEO_NETWORK_CLIENT_CONTRACTORSLISTTRANSACTION_H
