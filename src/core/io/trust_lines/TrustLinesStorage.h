#ifndef GEO_NETWORK_CLIENT_TRUSTLINESSTORAGE_H
#define GEO_NETWORK_CLIENT_TRUSTLINESSTORAGE_H

#include <string>
#include <vector>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include "../../common/NodeUUID.h"
#include "../../db/Block.h"
#include "../../db/UUIDMapBlockStorage.h"

using namespace std;

namespace io = db::uuid_map_block_storage;

typedef io::byte byte;

class TrustLinesStorage : public io::UUIDMapBlockStorage {

public:
    TrustLinesStorage(const string &fileName);

    ~TrustLinesStorage();

    const vector<NodeUUID> getAllContractorsUUIDs();

    void writeNewTrustLineInStorage(const NodeUUID &uuid, const byte *data, const size_t bytesCount);

    void modifyExistingTrustLineInStorage(const NodeUUID &uuid, const byte *data, const size_t bytesCount);

    const io::Block *readTrustLineFromStorage(const NodeUUID &uuid);

    void removeTrustLineFromStorage(const NodeUUID &uuid);
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINESSTORAGE_H
