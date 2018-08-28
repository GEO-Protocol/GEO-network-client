/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_CREDITUSAGECOMMAND_H
#define GEO_NETWORK_CLIENT_CREDITUSAGECOMMAND_H

#include "../BaseUserCommand.h"

#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include "../../../../common/exceptions/ValueError.h"
#include "../../../../common/exceptions/MemoryError.h"


class CreditUsageCommand:
    public BaseUserCommand {

public:
    typedef shared_ptr<CreditUsageCommand> Shared;

public:
    CreditUsageCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    const TrustLineAmount& amount() const;

    const NodeUUID& contractorUUID() const;

public:
    // Results handlers
    CommandResult::SharedConst responseNoConsensus() const;
    CommandResult::SharedConst responseOK(
        string &transactionUUID) const;

private:
    NodeUUID mContractorUUID;
    TrustLineAmount mAmount;
};

#endif //GEO_NETWORK_CLIENT_CREDITUSAGECOMMAND_H
