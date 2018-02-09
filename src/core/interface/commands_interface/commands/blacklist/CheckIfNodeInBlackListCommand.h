#ifndef GEO_NETWORK_CLIENT_CHECKIFNODEINBLACKLIST_H
#define GEO_NETWORK_CLIENT_CHECKIFNODEINBLACKLIST_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"


class CheckIfNodeInBlackListCommand:
    public BaseUserCommand {

public:
    typedef shared_ptr<CheckIfNodeInBlackListCommand> Shared;

public:
    CheckIfNodeInBlackListCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier()
    noexcept;

    const NodeUUID& contractorUUID()
    noexcept;

protected:
    NodeUUID mContractorUUID;
};

#endif //GEO_NETWORK_CLIENT_CHECKIFNODEINBLACKLIST_H
