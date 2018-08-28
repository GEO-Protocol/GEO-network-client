/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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

    CommandResult::SharedConst resultOk(
        string &neighbors) const;

protected:
    void parse(const string &command);
};

#endif //GEO_NETWORK_CLIENT_FIRSTLEVELCONTRACTORSBALANCESCOMMAND_H
