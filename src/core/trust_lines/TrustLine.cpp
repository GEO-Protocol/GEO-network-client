#include "TrustLine.h"

TrustLine::TrustLine(
    const NodeUUID &nodeUUID,
    const TrustLineID trustLineID,
    const TrustLineAmount &incomingAmount,
    const TrustLineAmount &outgoingAmount,
    const TrustLineBalance &nodeBalance,
    bool isContractorGateway,
    TrustLineState state,
    AuditNumber auditNumber) :

    mContractorNodeUUID(nodeUUID),
    mID(trustLineID),
    mIncomingTrustAmount(incomingAmount),
    mOutgoingTrustAmount(outgoingAmount),
    mBalance(nodeBalance),
    mIsContractorGateway(isContractorGateway),
    mCurrentAudit(auditNumber),
    mState(state),
    mTotalIncomingReceiptsAmount(kZeroAmount()),
    mTotalOutgoingReceiptsAmount(kZeroAmount())
{
    // todo zero amounts checking
}

TrustLine::TrustLine(
    const NodeUUID &nodeUUID,
    const TrustLineID trustLineID,
    bool isContractorGateway,
    TrustLineState state) :

    mContractorNodeUUID(nodeUUID),
    mID(trustLineID),
    mIncomingTrustAmount(kZeroAmount()),
    mOutgoingTrustAmount(kZeroAmount()),
    mBalance(kZeroBalance()),
    mIsContractorGateway(isContractorGateway),
    mCurrentAudit(kInitialAuditNumber),
    mState(state),
    mTotalIncomingReceiptsAmount(kZeroAmount()),
    mTotalOutgoingReceiptsAmount(kZeroAmount())
{}

/**
 * Sets incoming trust amount of the trust line.
 * This method rewrites previous incoming trust amount.
 *
 * Throws ValueError in case if "amount" is too big.
 */
void TrustLine::setIncomingTrustAmount(
    const TrustLineAmount &amount)
{
    // Incoming trust amount may be converted to the TrustLineBalance,
    // and, in case if it would be converted to the negative value, - overflow is possible.
    // To avoid this - max value of TrustLineAmount is avoided from using.
    if (numeric_limits<TrustLineAmount>::max() == amount) {
        throw ValueError("TrustLine::setIncomingTrustAmount: "
                             "Amount is too big.");
    }

    mIncomingTrustAmount = amount;
}

/*!
 * Sets outgoing trust amount of the trust line.
 * This method rewrites previous outgoing trust amount.
 *
 *
 * Throws ValueError in case if "amount" is too big.
 */
void TrustLine::setOutgoingTrustAmount(
    const TrustLineAmount &amount)
{
    // Incoming trust amount may be converted to the TrustLineBalance,
    // and in case if it would be converted to the negative value - overflow is possible.
    // To avoid this - max value of TrustLineAmount is avoided from using.
    if (numeric_limits<TrustLineAmount>::max() == amount) {
        throw ValueError("TrustLine::setOutgoingTrustAmount: "
                             "Amount is too big.");
    }

    mOutgoingTrustAmount = amount;
}

void TrustLine::setBalance(
    const TrustLineBalance &balance)
{
    if (numeric_limits<TrustLineBalance>::max() == balance) {
        throw ValueError("TrustLine::setBalance: Balance is too big");
    }
    mBalance = balance;
}

const NodeUUID &TrustLine::contractorNodeUUID() const
{
    return mContractorNodeUUID;
}

const TrustLineAmount &TrustLine::incomingTrustAmount() const
{
    return mIncomingTrustAmount;
}

const TrustLineAmount &TrustLine::outgoingTrustAmount() const
{
    return mOutgoingTrustAmount;
}

const TrustLineBalance &TrustLine::balance() const
{
    return mBalance;
}

const TrustLineID TrustLine::trustLineID() const
{
    return mID;
}

const TrustLine::TrustLineState TrustLine::state() const
{
    return mState;
}

const AuditNumber TrustLine::currentAuditNumber() const
{
    return mCurrentAudit;
}

void TrustLine::setState(
    TrustLine::TrustLineState newState)
{
    mState = newState;
}

void TrustLine::setAuditNumber(
    AuditNumber newAuditNumber)
{
    mCurrentAudit = newAuditNumber;
}

/*!
 * Returns amount that is available to use on the trust line.
 */
ConstSharedTrustLineAmount TrustLine::availableOutgoingAmount() const
{
    if (mBalance < kZeroBalance() && absoluteBalanceAmount(mBalance) > mIncomingTrustAmount) {
        return make_shared<const TrustLineAmount>(0);
    }
    return make_shared<const TrustLineAmount>(
        mIncomingTrustAmount + mBalance);
}

/*!
 * Returns amount that is available to use on the trust line from contractor node.
 */
ConstSharedTrustLineAmount TrustLine::availableIncomingAmount() const
{
    if (mBalance > kZeroBalance() && absoluteBalanceAmount(mBalance) > mOutgoingTrustAmount) {
        return make_shared<const TrustLineAmount>(0);
    }
    return make_shared<const TrustLineAmount>(
        mOutgoingTrustAmount - mBalance);
}

ConstSharedTrustLineAmount TrustLine::usedAmountByContractor() const
{
    if (mBalance >= kZeroBalance()) {
        return make_shared<const TrustLineAmount>(mBalance);
    } else {
        return make_shared<const TrustLineAmount>(0);
    }
}

ConstSharedTrustLineAmount TrustLine::usedAmountBySelf() const
{
    if (mBalance <= kZeroBalance()) {
        return make_shared<const TrustLineAmount>(-mBalance);
    } else {
        return make_shared<const TrustLineAmount>(0);
    }
}

void TrustLine::setTotalOutgoingReceiptsAmount(
    const TrustLineAmount &amount)
{
    if (numeric_limits<TrustLineAmount>::max() == amount) {
        throw ValueError("TrustLine::setTotalOutgoingReceiptsAmount: Amount is too big.");
    }

    mTotalOutgoingReceiptsAmount = amount;
}

void TrustLine::setTotalIncomingReceiptsAmount(
    const TrustLineAmount &amount)
{
    if (numeric_limits<TrustLineAmount>::max() == amount) {
        throw ValueError("TrustLine::setTotalIncomingReceiptsAmount: Amount is too big.");
    }

    mTotalIncomingReceiptsAmount = amount;
}

// todo : need improve this logic or launch automatic audit by user configuration
bool TrustLine::isTrustLineOverflowed() const
{
    if (mTotalIncomingReceiptsAmount > mOutgoingTrustAmount
        or mTotalOutgoingReceiptsAmount > mIncomingTrustAmount) {
        return true;
    }
    return false;
}

void TrustLine::resetTotalReceiptsAmounts()
{
    mTotalOutgoingReceiptsAmount = TrustLine::kZeroAmount();
    mTotalIncomingReceiptsAmount = TrustLine::kZeroAmount();
}

bool TrustLine::isContractorGateway() const
{
    return mIsContractorGateway;
}

void TrustLine::setContractorAsGateway(
    bool contractorAsGateway)
{
    mIsContractorGateway = contractorAsGateway;
}

void TrustLine::setIsOwnKeysPresent(
    bool isOwnKeysPresent)
{
    mIsOwnKeysPresent = isOwnKeysPresent;
}

void TrustLine::setIsContractorKeysPresent(
    bool isContractorKeysPresent)
{
    mIsContractorKeysPresent = isContractorKeysPresent;
}

/*!
 * @returns static constant zero balance,
 * that is useful in comparison operations.
 */
const TrustLineBalance &TrustLine::kZeroBalance()
{
    static TrustLineBalance zero(0);
    return zero;
}

/*!
 * @returns static constant zero amount,
 * that is useful in comparison operations.
 */
const TrustLineAmount &TrustLine::kZeroAmount()
{
    static TrustLineAmount zero(0);
    return zero;
}

bool operator==(
    const TrustLine::Shared contractor1,
    const TrustLine::Shared contractor2)
{
    return contractor1->contractorNodeUUID() == contractor2->contractorNodeUUID();
}

bool operator==(
    const TrustLine &contractor1,
    const TrustLine &contractor2)
{
    return contractor1.contractorNodeUUID() == contractor2.contractorNodeUUID();
}

/**
 * Decreases debt of the contractor (if exists),
 * and uses credit to the current node (if needed).
 *
 * @param amount - specifies how much should be cleared/used.
 */
// TODO : check again this conditions
void TrustLine::pay(
    const TrustLineAmount &amount)
{
    const auto kNewBalance = mBalance - amount;
    if (mBalance * kNewBalance > kZeroBalance()) {
        if (mBalance > kZeroBalance() && mBalance > kNewBalance) {
            mBalance = kNewBalance;
            mTotalOutgoingReceiptsAmount = mTotalOutgoingReceiptsAmount + amount;
            return;
        }
        if (mBalance < kZeroBalance() && mBalance < kNewBalance) {
            mBalance = kNewBalance;
            mTotalOutgoingReceiptsAmount = mTotalOutgoingReceiptsAmount + amount;
            return;
        }
    }
    if (kNewBalance < kZeroBalance() && abs(kNewBalance) > mIncomingTrustAmount) {
        throw OverflowError(
            "TrustLine::useCredit: attempt of using more than incoming credit amount.");
    }
    if (kNewBalance > kZeroBalance() && kNewBalance > mOutgoingTrustAmount) {
        throw OverflowError(
            "TrustLine::useCredit: attempt of using more than incoming credit amount.");
    }
    mBalance = kNewBalance;
    mTotalOutgoingReceiptsAmount = mTotalOutgoingReceiptsAmount + amount;
}

/**
 * Clears debt of the current node (if exists)
 * and increases debt of the contractor (if exists).
 *
 * @param amount - specifies how much should be cleared/used.
 */
// TODO : check again this conditions
void TrustLine::acceptPayment(
    const TrustLineAmount &amount)
{
    const auto kNewBalance = mBalance + amount;
    if (mBalance * kNewBalance > kZeroBalance()) {
        if (mBalance > kZeroBalance() && mBalance > kNewBalance) {
            mBalance = kNewBalance;
            mTotalIncomingReceiptsAmount = mTotalIncomingReceiptsAmount + amount;
            return;
        }
        if (mBalance < kZeroBalance() && mBalance < kNewBalance) {
            mBalance = kNewBalance;
            mTotalIncomingReceiptsAmount = mTotalIncomingReceiptsAmount + amount;
            return;
        }
    }
    if (kNewBalance < kZeroBalance() && abs(kNewBalance) > mIncomingTrustAmount) {
        throw OverflowError(
            "TrustLine::useCredit: attempt of using more than incoming credit amount.");
    }
    if (kNewBalance > kZeroBalance() && kNewBalance > mOutgoingTrustAmount) {
        throw OverflowError(
            "TrustLine::useCredit: attempt of using more than incoming credit amount.");
    }
    mBalance = kNewBalance;
    mTotalIncomingReceiptsAmount = mTotalIncomingReceiptsAmount + amount;
}
