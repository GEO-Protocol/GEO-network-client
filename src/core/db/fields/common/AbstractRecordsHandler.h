#ifndef GEO_NETWORK_CLIENT_ABSTRACTRECORD_H
#define GEO_NETWORK_CLIENT_ABSTRACTRECORD_H

#include <cstdint>


namespace db {


class AbstractRecordsHandler {
public:
    typedef uint8_t byte;
    typedef uint32_t RecordNumber;
    typedef RecordNumber RecordsCount;
};


} // namespace db;


#endif //GEO_NETWORK_CLIENT_ABSTRACTRECORD_H
