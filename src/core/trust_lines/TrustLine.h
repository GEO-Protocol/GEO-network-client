#ifndef GEO_NETWORK_CLIENT_TRUSTLINE_H
#define GEO_NETWORK_CLIENT_TRUSTLINE_H

#include "../common/NodeUUID.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/multiprecision/cpp_int.hpp>


using namespace std;
using boost::multiprecision::cpp_int;

namespace multiprecision = boost::multiprecision;

typedef multiprecision::checked_uint256_t trust_amount;
typedef multiprecision::int256_t balance_value;
typedef boost::function<void()> callback;

struct TrustLine {
    friend class TrustLinesManager;

public:
    typedef shared_ptr<TrustLine> Shared;
    typedef shared_ptr<const TrustLine> SharedConst;

private:
    NodeUUID mContractorNodeUuid;
    trust_amount mIncomingTrustAmount;
    trust_amount mOutgoingTrustAmount;
    balance_value mBalance;
    callback mManagerCallback;

private:
    TrustLine(const NodeUUID &nodeUUID,
              const trust_amount &incomingAmount,
              const trust_amount &outgoingAmount,
              const balance_value &nodeBalance);

    void setContractorNodeUUID(
            const NodeUUID &nodeUUID);

    void setIncomingTrustAmount(
            callback managersCallback,
            const trust_amount &incomingAmount);

    void setOutgoingTrustAmount(
            callback managersCallback,
            const trust_amount &outgoingAmount);

    void setBalance(
            callback managersCallback,
            const balance_value &nodeBalance);

public:
    const NodeUUID &getContractorNodeUUID() const;

    const trust_amount &getIncomingTrustAmount() const;

    const trust_amount &getOutgoingTrustAmount() const;

    const balance_value &getBalance() const;
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINE_H
