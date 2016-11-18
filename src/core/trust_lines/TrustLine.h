#ifndef TRUSTLINEMANAGER_TRUSTLINE_H
#define TRUSTLINEMANAGER_TRUSTLINE_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/multiprecision/cpp_int.hpp>

using namespace std;
using boost::multiprecision::cpp_int;

namespace uuids = boost::uuids;
namespace multiprecision = boost::multiprecision;

typedef multiprecision::checked_uint256_t trust_amount;
typedef multiprecision::int256_t balance_value;

class TrustLinesManager;

struct TrustLine {

private:
    friend class TrustLinesManager;
    uuids::uuid contractor_node_uuid;
    trust_amount incoming_trust_amount;
    trust_amount outgoing_trust_amount;
    balance_value balance;

private:
    TrustLine(uuids::uuid nodeUUID,
              trust_amount incomingAmount,
              trust_amount outgoingAmount,
              balance_value nodeBalance);

    void setContractorNodeUUID(uuids::uuid nodeUUID);

    void setIncomingTrustAmount(trust_amount incomingAmount);

    void setOutgoingTrustAmount(trust_amount outgoingAmount);

    void setBalance(balance_value nodeBalance);

    uuids::uuid getContractorNodeUUID();

    trust_amount getIncomingTrustAmount();

    trust_amount getOutgoingTrustAmount();

    balance_value getBalance();
};

#endif //TRUSTLINEMANAGER_TRUSTLINE_H
