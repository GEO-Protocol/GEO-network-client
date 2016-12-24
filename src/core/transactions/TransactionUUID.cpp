#include "TransactionUUID.h"

TransactionUUID::TransactionUUID()
    : uuid(boost::uuids::random_generator()()) {
}

TransactionUUID::TransactionUUID(uuid const &u) :
    boost::uuids::uuid(u) {}

TransactionUUID::TransactionUUID(TransactionUUID &u) {
    memcpy(data, u.data, kUUIDSize);
}

TransactionUUID::TransactionUUID(const TransactionUUID &u) {
    memcpy(data, u.data, kUUIDSize);
}

TransactionUUID::TransactionUUID(const string &hex) {
    uuid u = boost::lexical_cast<uuid>(hex);
    memcpy(data, u.data, kUUIDSize);
}

TransactionUUID::operator boost::uuids::uuid() {
    return static_cast<boost::uuids::uuid &>(*this);
}

TransactionUUID::operator boost::uuids::uuid() const {
    return static_cast<boost::uuids::uuid const &>(*this);
}

const string TransactionUUID::stringUUID() const {
    uuid u;
    memcpy(&u.data, data, kUUIDSize);
    return boost::lexical_cast<string>(u);
}

TransactionUUID &TransactionUUID::operator=(const boost::uuids::uuid &u) {
    memcpy(data, u.data, kUUIDSize);
    return *this;
}
