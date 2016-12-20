#ifndef GEO_NETWORK_CLIENT_NODEUUID_H
#define GEO_NETWORK_CLIENT_NODEUUID_H


#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <string>


using namespace boost::uuids;
using namespace std;


class NodeUUID:
    public uuid {

public:
    static const size_t kUUIDLength = 16;

public:
    explicit NodeUUID();
    NodeUUID(uuid const &u);
    NodeUUID(NodeUUID &u);
    NodeUUID(const NodeUUID &u);
    NodeUUID(const string &hex);

    operator boost::uuids::uuid();
    operator boost::uuids::uuid() const;
    NodeUUID& operator=(
        const boost::uuids::uuid &u);

    const string stringUUID() const;
};

#endif //GEO_NETWORK_CLIENT_NODEUUID_H
