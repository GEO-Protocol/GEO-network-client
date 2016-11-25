#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/multiprecision/cpp_int.hpp>

namespace uuids = boost::uuids;
namespace multiprecision = boost::multiprecision;

class TransactionsManager {

public:
    TransactionsManager();

    ~TransactionsManager();

    void openTrustLine(const uuids::uuid &contractorUuid, multiprecision::checked_uint256_t amount);
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
