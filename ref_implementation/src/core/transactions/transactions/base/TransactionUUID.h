/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_TRANSACTIONUUID_H
#define GEO_NETWORK_CLIENT_TRANSACTIONUUID_H

#include "../../../common/NodeUUID.h"


/*
 * Used to globally distinquish transactions in whoole the system.
 * Each one new transaction in the system will be initialised
 * with a newly generated TransactionUUID.
 *
 * todo: research and document if uuid4 is enought for storing global transactions uuids.
 *
 *
 * At this moment, TransactionUUID uses the same logic as the NodeUUID,
 * but it's possible, that NodeUUID would be change it's logic in the future.
 * But for now, it's simply enough to inherit TransactionUUID from NodeUUID.
 */
class TransactionUUID:
    public NodeUUID {

public:
    using NodeUUID::NodeUUID;
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONUUID_H
