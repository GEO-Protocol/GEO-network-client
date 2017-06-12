#include "NodeUUID.h"

NodeUUID::NodeUUID()
    :uuid(boost::uuids::random_generator()()){
}

NodeUUID::NodeUUID(uuid const &u):
    boost::uuids::uuid(u){}

NodeUUID::NodeUUID(NodeUUID &u) {
    memcpy(data, u.data, kBytesSize);
}

NodeUUID::NodeUUID(const NodeUUID &u){
    memcpy(data, u.data, kBytesSize);
}

NodeUUID::NodeUUID(const string &hex) {
    uuid u = boost::lexical_cast<uuid>(hex);
    memcpy(data, u.data, kBytesSize);
}

NodeUUID::NodeUUID(const uint8_t *bytes) {
    memcpy(data, bytes, kBytesSize);
}

NodeUUID::operator boost::uuids::uuid() {
    return static_cast<boost::uuids::uuid&>(*this);
}

NodeUUID::operator boost::uuids::uuid() const {
    return static_cast<boost::uuids::uuid const&>(*this);
}

const string NodeUUID::stringUUID() const{
    uuid u;
    memcpy(&u.data, data, 16);
    return boost::lexical_cast<string>(u);
}

NodeUUID& NodeUUID::operator=(const boost::uuids::uuid &u){
    memcpy(data, u.data, kBytesSize);
    return *this;
}

bool operator== (const NodeUUID &a, const NodeUUID &b)
{
#ifdef BOOST_BIG_ENDIAN
    for (size_t i=0; i<15; ++i){
        if (a.data[i] < b.data[i])
            return false;

        else if (a.data[i] > b.data[i])
            return false;
    }
    return false;
#endif

#ifdef BOOST_LITTLE_ENDIAN
    for (int i=15; i>-1; --i){
        if (a.data[i] < b.data[i])
            return false;

        else if (a.data[i] > b.data[i])
            return false;
    }
    return true;
#endif
}

const NodeUUID& NodeUUID::empty ()
{
    static const NodeUUID kEmpty("00000000-0000-0000-0000-000000000000");
    return kEmpty;
}

