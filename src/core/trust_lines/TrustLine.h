#ifndef GEO_NETWORK_CLIENT_TRUSTLINE_H
#define GEO_NETWORK_CLIENT_TRUSTLINE_H

#include "../common/Types.h"
#include "../common/NodeUUID.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include "../common/exceptions/RuntimeError.h"

#include <vector>

using namespace std;

namespace multiprecision = boost::multiprecision;

typedef multiprecision::checked_uint256_t TrustLineAmount;
typedef multiprecision::int256_t TrustLineBalance;

typedef boost::function<void()> SaveTrustLineCallback;

struct TrustLine {
    // todo: tests?
    // todo: hsc: review the tests.

public:
    typedef shared_ptr<TrustLine> Shared;
    typedef shared_ptr<const TrustLine> SharedConst;

public:
    TrustLine(
        const NodeUUID &nodeUUID,
        const TrustLineAmount &incomingAmount,
        const TrustLineAmount &outgoingAmount,
        const TrustLineBalance &nodeBalance);

    TrustLine(
        const byte *buffer,
        const NodeUUID &contractorUUID
    );

    void setIncomingTrustAmount(
        const TrustLineAmount &amount,
        SaveTrustLineCallback callback);

    void setOutgoingTrustAmount(
        const TrustLineAmount &amount,
        SaveTrustLineCallback callback);

    void setBalance(
        const TrustLineBalance &balance,
        SaveTrustLineCallback callback);

    const NodeUUID &contractorNodeUUID() const;

    const TrustLineAmount &incomingTrustAmount() const;

    const TrustLineAmount &outgoingTrustAmount() const;

    const TrustLineBalance &balance() const;

    const TrustLineDirection direction() const;

    vector<byte> serializeTrustLine();

    void deserializeTrustLine(
        const byte *buffer);

private:
    void trustAmountToBytes(
        const TrustLineAmount &amount,
        vector<byte> &buffer);

    void balanceToBytes(
        const TrustLineBalance &balance,
        vector<byte> &buffer);

    void parseTrustAmount(
        const byte *buffer,
        TrustLineAmount &variable);

    void parseBalance(
        const byte *buffer);

private:
    const size_t kTrustAmountPartSize = 32;
    const size_t kBalancePartSize = 32;
    const size_t kSignBytePartSize = 1;
    const size_t kRecordSize =
        +kTrustAmountPartSize
        + kTrustAmountPartSize
        + kBalancePartSize
        + kSignBytePartSize;

    NodeUUID mContractorNodeUuid;
    TrustLineAmount mIncomingTrustAmount;
    TrustLineAmount mOutgoingTrustAmount;
    TrustLineBalance mBalance;

};

#endif //GEO_NETWORK_CLIENT_TRUSTLINE_H
