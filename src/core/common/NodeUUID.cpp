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

const string NodeUUID::stringUUID() const{
    uuid u;
    memcpy(&u.data, data, 16);
    return boost::lexical_cast<string>(u);
}

NodeUUID& NodeUUID::operator=(const boost::uuids::uuid &u){
    memcpy(data, u.data, kBytesSize);
    return *this;
}

const NodeUUID& NodeUUID::empty ()
{
    static const NodeUUID kEmpty("00000000-0000-0000-0000-000000000000");
    return kEmpty;
}

