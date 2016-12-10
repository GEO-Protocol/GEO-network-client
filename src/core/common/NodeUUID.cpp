#include "NodeUUID.h"

NodeUUID::NodeUUID()
    :uuid(boost::uuids::random_generator()()){
}

NodeUUID::NodeUUID(uuid const &u):
    boost::uuids::uuid(u){
}

NodeUUID::NodeUUID(const uint8_t *bytes) {
    memcpy(data, bytes, kUUIDLength);
}

NodeUUID::operator boost::uuids::uuid() {
    return static_cast<boost::uuids::uuid&>(*this);
}

NodeUUID::operator boost::uuids::uuid() const {
    return static_cast<boost::uuids::uuid const&>(*this);
}

/*!
 * Compares two uuids in endiannes-aware manner.
 * Returns
 *    LESS in case if a < b,
 *    GREATER in case if a > b,
 *    and EQUAL in case if a==b;
 */
NodeUUID::ComparePredicates NodeUUID::compare(const NodeUUID &a, const NodeUUID &b) {

#ifdef BOOST_BIG_ENDIAN
    for (size_t i=0; i<kUUIDLength; ++i){
        if (a.data[i] < b.data[i])
            return LESS;

        else if (a.data[i] > b.data[i])
            return GREATER;
    }
    return EQUAL;
#endif

#ifdef BOOST_LITTLE_ENDIAN
    for (int i=15; i>-1; --i){
        if (a.data[i] < b.data[i])
            return LESS;

        else if (a.data[i] > b.data[i])
            return GREATER;
    }
    return EQUAL;
#endif
}

