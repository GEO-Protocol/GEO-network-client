#ifndef GEO_NETWORK_CLIENT_NOEQUIVALENTTRANSACTION_H
#define GEO_NETWORK_CLIENT_NOEQUIVALENTTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/BaseUserCommand.h"
#include "../../../network/messages/general/NoEquivalentMessage.h"

class NoEquivalentTransaction : public BaseTransaction {

public:
    typedef shared_ptr<NoEquivalentTransaction> Shared;

public:
    NoEquivalentTransaction(
        BaseUserCommand::Shared command,
        Logger &logger);

    NoEquivalentTransaction(
        TransactionMessage::Shared message,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    const string logHeader() const override;

private:
    BaseUserCommand::Shared mCommand;
    TransactionMessage::Shared mMessage;
};


#endif //GEO_NETWORK_CLIENT_NOEQUIVALENTTRANSACTION_H
