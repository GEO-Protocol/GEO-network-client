/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_SETTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_SETTRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"

#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../../../common/exceptions/ValueError.h"


using namespace std;

/**
 * This command is used to open/close/update trust line to the remote contractor node.
 *
 * To create trust line — this command must be launched against new contractor.
 * To update trust line — this command must be issued against already present contractor.
 * To remove trust line — this command must be issued with 0 amount.
 */
class SetOutgoingTrustLineCommand:
    public BaseUserCommand {

public:
    typedef shared_ptr<SetOutgoingTrustLineCommand> Shared;

public:
    SetOutgoingTrustLineCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier()
        noexcept;

    const NodeUUID &contractorUUID() const
        noexcept;

    const TrustLineAmount &amount() const
        noexcept;

private:
    NodeUUID mContractorUUID;
    TrustLineAmount mAmount;
};


#endif //GEO_NETWORK_CLIENT_SETTRUSTLINECOMMAND_H
