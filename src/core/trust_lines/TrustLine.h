#ifndef GEO_NETWORK_CLIENT_TRUSTLINE_H
#define GEO_NETWORK_CLIENT_TRUSTLINE_H

#include <boost/function.hpp>
#include <boost/bind.hpp>
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
typedef boost::function<void()> callback;

class TrustLinesManager;

struct TrustLine {
    friend class TrustLinesManager;

private:
    uuids::uuid mContractorNodeUuid;
    trust_amount mIncomingTrustAmount;
    trust_amount mOutgoingTrustAmount;
    balance_value mBalance;
    callback mManagerCallback;

private:
    TrustLine(const uuids::uuid &nodeUUID,
              const trust_amount &incomingAmount,
              const trust_amount &outgoingAmount,
              const balance_value &nodeBalance);

    void setContractorNodeUUID(const uuids::uuid &nodeUUID);

    void setIncomingTrustAmount(callback managersCallback, const trust_amount &incomingAmount);

    void setOutgoingTrustAmount(callback managersCallback, const trust_amount &outgoingAmount);

    void setBalance(callback managersCallback, const balance_value &nodeBalance);

    uuids::uuid& getContractorNodeUUID();

    trust_amount& getIncomingTrustAmount();

    trust_amount& getOutgoingTrustAmount();

    balance_value& getBalance();
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINE_H
