#include "NodeUUID.h"

NodeUUID::NodeUUID()
    :uuid(boost::uuids::random_generator()()){
}

NodeUUID::NodeUUID(uuid const &u):
    boost::uuids::uuid(u){
}

NodeUUID::NodeUUID(NodeUUID const &u) {
    boost::uuids::uuid(boost::lexical_cast<boost::uuids::uuid>(u.stringUUID()));
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

