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
    typedef shared_ptr<const TrustLine> ConstShared;

public:
    enum TrustLineState {
        Init = 1,
        KeysPending = 2,
        AuditPending = 3,
        Active = 4,
    };

public:
    TrustLine(
        const NodeUUID &nodeUUID,
        const TrustLineID trustLineID,
        const TrustLineAmount &incomingAmount,
        const TrustLineAmount &outgoingAmount,
        const TrustLineBalance &nodeBalance,
        bool isContractorGateway,
        TrustLineState state = Init,
        AuditNumber auditNumber = 0);

    void setIncomingTrustAmount(
        const TrustLineAmount &amount);

    void setOutgoingTrustAmount(
        const TrustLineAmount &amount);

    void pay(
        const TrustLineAmount &amount);

    void acceptPayment(
        const TrustLineAmount &amount);

    const NodeUUID& contractorNodeUUID() const;

    const TrustLineAmount& incomingTrustAmount() const;

    const TrustLineAmount& outgoingTrustAmount() const;

    const TrustLineBalance& balance() const;

    const TrustLineID trustLineID() const;

    const TrustLineState state() const;

    const AuditNumber currentAuditNumber() const;

    void setState(
        TrustLineState newState);

    void setAuditNumber(
        AuditNumber newAuditNumber);

    ConstSharedTrustLineAmount availableOutgoingAmount() const;

    ConstSharedTrustLineAmount availableIncomingAmount() const;

    ConstSharedTrustLineAmount usedAmountByContractor() const;

    ConstSharedTrustLineAmount usedAmountBySelf() const;

    bool isContractorGateway() const;

    void setContractorAsGateway(
        bool contractorAsGateway);

    static const TrustLineBalance& kZeroBalance();

    static const TrustLineAmount& kZeroAmount();

    friend bool operator== (
        const TrustLine::Shared contractor1,
        const TrustLine::Shared contractor2);

    friend bool operator== (
        const TrustLine &contractor1,
        const TrustLine &contractor2);

private:
    NodeUUID mContractorNodeUUID;
    TrustLineID mID;
    TrustLineAmount mIncomingTrustAmount;
    TrustLineAmount mOutgoingTrustAmount;
    TrustLineBalance mBalance;
    bool mIsContractorGateway;
    TrustLineState mState;
    AuditNumber mCurrentAudit;
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINE_H
