/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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
