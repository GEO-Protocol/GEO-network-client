#ifndef GEO_NETWORK_CLIENT_FIRSTLEVELCONTRACTORSBALANCESCOMMAND_H
#define GEO_NETWORK_CLIENT_FIRSTLEVELCONTRACTORSBALANCESCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"


class GetTrustLinesCommand :
    public BaseUserCommand {

public:
    typedef shared_ptr<GetTrustLinesCommand> Shared;

public:
    GetTrustLinesCommand(
        const CommandUUID &uuid,
        const string &commandBuffer)
        noexcept;

    static const string &identifier();

    const SerializedEquivalent equivalent() const;

    CommandResult::SharedConst resultOk(
        string &neighbors) const;

private:
    SerializedEquivalent mEquivalent;
};

#endif //GEO_NETWORK_CLIENT_FIRSTLEVELCONTRACTORSBALANCESCOMMAND_H
