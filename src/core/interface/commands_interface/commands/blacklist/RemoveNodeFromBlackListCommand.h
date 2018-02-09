#ifndef GEO_NETWORK_CLIENT_REMOVENODEFROMBLACKLISTCOMMAND_H
#define GEO_NETWORK_CLIENT_REMOVENODEFROMBLACKLISTCOMMAND_H



#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"


class RemoveNodeFromBlackListCommand:
    public BaseUserCommand {

public:
    typedef shared_ptr<RemoveNodeFromBlackListCommand> Shared;

public:
    RemoveNodeFromBlackListCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier()
    noexcept;

    const NodeUUID& contractorUUID()
    noexcept;

protected:
    NodeUUID mContractorUUID;
};

#endif //GEO_NETWORK_CLIENT_REMOVENODEFROMBLACKLISTCOMMAND_H
