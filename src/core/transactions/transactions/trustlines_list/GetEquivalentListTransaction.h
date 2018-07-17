#ifndef GEO_NETWORK_CLIENT_GETEQUIVALENTLISTTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETEQUIVALENTLISTTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_lines_list/EquivalentListCommand.h"
#include "../../../equivalents/EquivalentsSubsystemsRouter.h"

class GetEquivalentListTransaction : public BaseTransaction {

public:
    typedef shared_ptr<GetEquivalentListTransaction> Shared;

public:
    GetEquivalentListTransaction(
        NodeUUID &nodeUUID,
        EquivalentListCommand::Shared command,
        EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
        Logger &logger)
        noexcept;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    EquivalentListCommand::Shared mCommand;
    EquivalentsSubsystemsRouter *mEquivalentsSubsystemsRouter;
};


#endif //GEO_NETWORK_CLIENT_GETEQUIVALENTLISTTRANSACTION_H
