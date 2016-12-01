#ifndef GEO_NETWORK_CLIENT_TYPES_H
#define GEO_NETWORK_CLIENT_TYPES_H

#include <cstdint>
#include <boost/uuid/uuid.hpp>


namespace db {
namespace routing_tables {

typedef uint64_t RecordsCount;
typedef uint64_t RecordNumber;
typedef uint8_t  byte;

typedef boost::uuids::uuid NodeUUID;

}
}

#endif //GEO_NETWORK_CLIENT_TYPES_H
