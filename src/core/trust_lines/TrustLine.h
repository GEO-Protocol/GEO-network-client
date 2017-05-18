#ifndef GEO_NETWORK_CLIENT_TRUSTLINE_H
#define GEO_NETWORK_CLIENT_TRUSTLINE_H

#include "../common/Types.h"
#include "../common/NodeUUID.h"
#include "../common/memory/MemoryUtils.h"
#include "../common/multiprecision/MultiprecisionUtils.h"

#include "../common/exceptions/RuntimeError.h"
#include "../common/exceptions/MemoryError.h"
#include "../common/exceptions/ValueError.h"
#include "../common/exceptions/OverflowError.h"

#include <vector>


using namespace std;

// todo: tests?
// todo: hsc: review the tests.
class TrustLine {
public:
    typedef shared_ptr<TrustLine> Shared;

    [[deprecated]]
    typedef shared_ptr<const TrustLine> SharedConst;
    typedef shared_ptr<const TrustLine> ConstShared;

public:
    TrustLine(
        const NodeUUID &nodeUUID,
        const TrustLineAmount &incomingAmount,
        const TrustLineAmount &outgoingAmount,
        const TrustLineBalance &nodeBalance);

    TrustLine(
        const NodeUUID &nodeUUID,
        const TrustLineAmount &incomingAmount,
        const TrustLineAmount &outgoingAmount);

    TrustLine(
        const byte *buffer,
        const NodeUUID &contractorUUID);

    void setIncomingTrustAmount(
        const TrustLineAmount &amount);

    void setOutgoingTrustAmount(
        const TrustLineAmount &amount);

    void setBalance(
        const TrustLineBalance &balance);

    void pay(
        const TrustLineAmount &amount);

    void acceptPayment(
        const TrustLineAmount &amount);

    void activateOutgoingDirection();

    void suspendOutgoingDirection();

    void activateIncomingDirection();

    void suspendIncomingDirection();

    const NodeUUID& contractorNodeUUID() const;

    const TrustLineAmount& incomingTrustAmount() const;

    const TrustLineAmount& outgoingTrustAmount() const;

    const TrustLineBalance& balance() const;

    ConstSharedTrustLineAmount availableOutgoingAmount() const;

    ConstSharedTrustLineAmount availableIncomingAmount() const;

    ConstSharedTrustLineAmount usedAmountByContractor() const;

    ConstSharedTrustLineAmount usedAmountBySelf() const;

    const TrustLineDirection direction() const;

    const BalanceRange balanceRange() const;

    vector<byte> serialize();

    void deserialize(
        const byte *buffer);

    static const TrustLineBalance& kZeroBalance();

    static const TrustLineAmount& kZeroAmount();

    friend bool operator== (
        const TrustLine::Shared contractor1,
        const TrustLine::Shared contractor2
    );

    friend bool operator== (
        const TrustLine &contractor1,
        const TrustLine &contractor2
    );

private:
    void trustAmountToBytes(
        const TrustLineAmount &amount,
        vector<byte> &buffer);

    void balanceToBytes(
        const TrustLineBalance &balance,
        vector<byte> &buffer);

    void directionStateToBytes(
        TrustState state,
        vector<byte> &buffer);

    void parseTrustAmount(
        const byte *buffer,
        TrustLineAmount &variable);

    void parseBalance(
        const byte *buffer);

    TrustState parseDirectionState(
        const byte *buffer);

private:
    const size_t kTrustAmountPartSize = 32;
    const size_t kBalancePartSize = 32;
    const size_t kSignBytePartSize = 1;
    const size_t kTrustStatePartSize = 1;
    const size_t kRecordSize =
        + kTrustAmountPartSize
        + kTrustAmountPartSize
        + kBalancePartSize
        + kSignBytePartSize
        + kTrustStatePartSize
        + kTrustStatePartSize;

    NodeUUID mContractorNodeUUID;
    TrustLineAmount mIncomingTrustAmount;
    TrustLineAmount mOutgoingTrustAmount;
    TrustLineBalance mBalance;

    // <incoming direction state, outgoing direction state>
    pair<TrustState, TrustState> mTrustLineState;

};

#endif //GEO_NETWORK_CLIENT_TRUSTLINE_H
