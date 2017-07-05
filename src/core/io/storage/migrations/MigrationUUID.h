
#ifndef GEO_NETWORK_CLIENT_MIGRATIONUUID_H
#define GEO_NETWORK_CLIENT_MIGRATIONUUID_H


#include "../../../common/NodeUUID.h"


/*
 * Used to globally distinquish transactions in whoole the system.
 * Each one new transaction in the system will be initialised
 * with a newly generated MigrationUUID.
 *
 * todo: research and document if uuid4 is enought for storing global transactions uuids.
 *
 *
 * At this moment, MigrationUUID uses the same logic as the NodeUUID,
 * but it's possible, that NodeUUID would be change it's logic in the future.
 * But for now, it's simply enough to inherit MigrationUUID from NodeUUID.
 */

class MigrationUUID:
        public NodeUUID {

public:
    using NodeUUID::NodeUUID;
};

#endif //GEO_NETWORK_CLIENT_MIGRATIONUUID_H
