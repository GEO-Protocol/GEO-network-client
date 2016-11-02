#ifndef GEO_NETWORK_CLIENT_NODEUUID_H
#define GEO_NETWORK_CLIENT_NODEUUID_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

using boost::uuids::uuid;

class NodeUUID: public uuid {
public:
    NodeUUID();
    explicit NodeUUID(uuid const& u);

    operator boost::uuids::uuid();
    operator boost::uuids::uuid() const;
};


#endif //GEO_NETWORK_CLIENT_NODEUUID_H
