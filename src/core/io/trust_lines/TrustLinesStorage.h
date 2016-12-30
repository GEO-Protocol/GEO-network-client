#ifndef GEO_NETWORK_CLIENT_TRUSTLINESSTORAGE_H
#define GEO_NETWORK_CLIENT_TRUSTLINESSTORAGE_H

#include <string>
#include <vector>

#include "../../common/Types.h"
#include "../../common/NodeUUID.h"
#include "../../db/uuid_map_block_storage/Record.h"
#include "../../db/uuid_map_block_storage/UUIDMapBlockStorage.h"

using namespace std;

namespace storage = db::uuid_map_block_storage;

class TrustLinesStorage : public storage::UUIDMapBlockStorage {

public:
    TrustLinesStorage(
            const string &fileName);

    ~TrustLinesStorage();

    const vector<NodeUUID> getAllContractorsUUIDs();
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINESSTORAGE_H
