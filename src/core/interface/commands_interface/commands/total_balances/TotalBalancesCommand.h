#ifndef GEO_NETWORK_CLIENT_TOTALBALANCESCOMMAND_H
#define GEO_NETWORK_CLIENT_TOTALBALANCESCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

class TotalBalancesCommand : public BaseUserCommand {

public:
    typedef shared_ptr<TotalBalancesCommand> Shared;

public:
    TotalBalancesCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    const SerializedEquivalent equivalent() const;

    CommandResult::SharedConst resultOk(
        string &totalBalancesStr) const;

private:
    SerializedEquivalent mEquivalent;
};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCESCOMMAND_H
