#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H

#include "../UniqueTransaction.h"

#include "../../../../interface/commands_interface/commands/trust_lines/CloseTrustLineCommand.h"

#include "../../../../network/messages/Message.h"
#include "../../../../network/messages/outgoing/CloseTrustLineMessage.h"

#include "../../../manager/TransactionsManager.h"

#include "../../../../common/exceptions/ConflictError.h"

class CloseTrustLineTransaction: public UniqueTransaction {

public:
    typedef shared_ptr<CloseTrustLineTransaction> Shared;

public:
    CloseTrustLineTransaction(
      NodeUUID &nodeUUID,
      CloseTrustLineCommand::Shared command,
      TransactionsScheduler *scheduler,
      TrustLinesManager *manager
    );

    CloseTrustLineCommand::Shared command() const;

    TransactionResult::Shared run();

    bool checkSameTypeTransactions();

    bool checkTrustLineExisting();

    bool checkDebt();

    void pendingSuspendTrustLineToContractor();

    void suspendTrustLineToContractor();

    void sendMessageToRemoteNode();

    TransactionResult::Shared waitingForResponseState();

    TransactionResult::Shared resultOk();

    TransactionResult::Shared trustLineAbsentResult();

    TransactionResult::Shared conflictErrorResult();

    TransactionResult::Shared noResponseResult();

private:
    const uint64_t kConnectionTimeout = 2000;
    const uint16_t kMaxRequestsCount = 5;

    CloseTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;

};


#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
