#ifndef GEO_NETWORK_CLIENT_NODEUUID_H
#define GEO_NETWORK_CLIENT_NODEUUID_H

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

using boost::uuids::uuid;
using namespace std;

class NodeUUID: public uuid {
public:
    NodeUUID();
    explicit NodeUUID(uuid const& u);

    operator boost::uuids::uuid();
    operator boost::uuids::uuid() const;

    const string stringUUID() const;
};


#endif //GEO_NETWORK_CLIENT_NODEUUID_H
