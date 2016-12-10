#ifndef GEO_NETWORK_CLIENT_NODEUUID_H
#define GEO_NETWORK_CLIENT_NODEUUID_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/endian/arithmetic.hpp>

using boost::uuids::uuid;

class NodeUUID: public uuid {
    friend class BucketBlockTests;

public:
    static const unsigned char kUUIDLength = 16;

    enum ComparePredicates {
        LESS = 0,
        EQUAL = 1,
        GREATER = 2,
    };
    static ComparePredicates compare(const NodeUUID &a, const NodeUUID &b);

public:
    explicit NodeUUID();
    explicit NodeUUID(const uuid &u);
    explicit NodeUUID(const uint8_t *bytes);

    operator boost::uuids::uuid();
    operator boost::uuids::uuid() const;
};



#endif //GEO_NETWORK_CLIENT_NODEUUID_H
