#ifndef GEO_NETWORK_CLIENT_NODEUUID_H
#define GEO_NETWORK_CLIENT_NODEUUID_H


#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/endian/arithmetic.hpp>

#include <string>


using namespace boost::uuids;
using namespace std;


class NodeUUID:
    public uuid {

public:
    static const size_t kHexSize = 36;
    static const size_t kBytesSize = 16;

    static const NodeUUID& empty();

public:
    explicit NodeUUID();
    NodeUUID(uuid const &u);
    NodeUUID(NodeUUID &u);
    NodeUUID(const NodeUUID &u);
    NodeUUID(const string &hex);
    explicit NodeUUID(const uint8_t *bytes);

    operator boost::uuids::uuid();
    operator boost::uuids::uuid() const;
    NodeUUID& operator=(
        const boost::uuids::uuid &u);

    friend bool operator== (
        const NodeUUID &u1,
        const NodeUUID &u2);

    const string stringUUID() const;
};

namespace std {
    template <>
    class hash<NodeUUID>{
    public :
        size_t operator()(const NodeUUID &nodeUUID) const {
            const auto kPtr = reinterpret_cast<const size_t* >(nodeUUID.data);
            return *kPtr;
        }
    };
}

#endif //GEO_NETWORK_CLIENT_NODEUUID_H
