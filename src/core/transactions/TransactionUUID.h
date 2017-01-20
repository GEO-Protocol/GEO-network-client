#ifndef GEO_NETWORK_CLIENT_TRANSACTIONUUID_H
#define GEO_NETWORK_CLIENT_TRANSACTIONUUID_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <string>

using namespace boost::uuids;
using namespace std;

class TransactionUUID:
        public uuid {

public:
    static const size_t kUUIDSize = 16;
    static const size_t kUUIDLength = 36;

public:
    explicit TransactionUUID();

    TransactionUUID(uuid const &u);

    TransactionUUID(TransactionUUID &u);

    TransactionUUID(const TransactionUUID &u);

    TransactionUUID(const string &hex);

    operator boost::uuids::uuid();

    operator boost::uuids::uuid() const;

    TransactionUUID& operator=(const boost::uuids::uuid &u);

    const string stringUUID() const;
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONUUID_H
