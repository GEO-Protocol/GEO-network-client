/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_ROUTINGTALERESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_ROUTINGTALERESPONSETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../network/messages/routing_table/RoutingTableRequestMessage.h"
#include "../../../network/messages/routing_table/RoutingTableResponseMessage.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"

#include <set>


class RoutingTableResponseTransaction :
    public BaseTransaction {

public:
    RoutingTableResponseTransaction(
            const NodeUUID &nodeUUID,
            RoutingTableRequestMessage::Shared message,
            TrustLinesManager *manager,
            Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

protected:
    RoutingTableRequestMessage::Shared mRequestMessage;
    TrustLinesManager *mTrustLinesManager;
};
#endif //GEO_NETWORK_CLIENT_ROUTINGTALERESPONSETRANSACTION_H
