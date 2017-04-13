#include "TrustLine.h"

TrustLine::TrustLine(
    const NodeUUID &nodeUUID,
    const TrustLineAmount &incomingAmount,
    const TrustLineAmount &outgoingAmount,
    const TrustLineBalance &nodeBalance) :

    mContractorNodeUUID(nodeUUID),
    mIncomingTrustAmount(incomingAmount),
    mOutgoingTrustAmount(outgoingAmount),
    mBalance(nodeBalance) {

    if (mBalance > kZeroBalance()){
        if (mBalance > mIncomingTrustAmount) {
            throw ValueError("TrustLine::TrustLine: "
                                 "Balance can't be greater than incoming trust amount.");
        }

    } else {
        if (-mBalance > mOutgoingTrustAmount) {
            throw ValueError("TrustLine::TrustLine: "
                                 "Balance can't be less than outgoing trust amount.");
        }
    }

    mTrustLineState.first = TrustState::NonActivated;
    mTrustLineState.second = TrustState::NonActivated;
}

TrustLine::TrustLine(
    const NodeUUID &nodeUUID,
    const TrustLineAmount &incomingAmount,
    const TrustLineAmount &outgoingAmount):

    mContractorNodeUUID(nodeUUID),
    mIncomingTrustAmount(incomingAmount),
    mOutgoingTrustAmount(outgoingAmount),
    mBalance(0) {

    mTrustLineState.first = TrustState::NonActivated;
    mTrustLineState.second = TrustState::NonActivated;
}

TrustLine::TrustLine(
    const byte *buffer,
    const NodeUUID &contractorUUID) :

    mContractorNodeUUID(contractorUUID) {

    mTrustLineState.first = TrustState::NonActivated;
    mTrustLineState.second = TrustState::NonActivated;
    deserialize(buffer);
}

/**
 * Sets incoming trust amount of the trust line.
 * This method rewrites previous incoming trust amount.
 *
 * Throws ValueError in case if "amount" is too big.
 */
void TrustLine::setIncomingTrustAmount(
    const TrustLineAmount &amount) {

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
    const TrustLineAmount &amount) {

    // Incoming trust amount may be converted to the TrustLineBalance,
    // and in case if it would be converted to the negative value - overflow is possible.
    // To avoid this - max value of TrustLineAmount is avoided from using.
    if (numeric_limits<TrustLineAmount>::max() == amount) {
        throw ValueError("TrustLine::setOutgoingTrustAmount: "
                             "Amount is too big.");
    }

    mOutgoingTrustAmount = amount;
}

/*!
 * Sets balance of the trust line.
 * This method rewrites previous balance.
 */
void TrustLine::setBalance(
    const TrustLineBalance &balance) {

    mBalance = balance;
}

void TrustLine::activateOutgoingDirection() {

    mTrustLineState.second = TrustState::Active;
}

void TrustLine::suspendOutgoingDirection() {

    mTrustLineState.second = TrustState::Suspended;
}

void TrustLine::activateIncomingDirection() {

    mTrustLineState.first = TrustState::Active;
}

void TrustLine::suspendIncomingDirection() {

    mTrustLineState.first = TrustState::Suspended;
}

const NodeUUID &TrustLine::contractorNodeUUID() const {

    return mContractorNodeUUID;
}

const TrustLineAmount &TrustLine::incomingTrustAmount() const {

    return mIncomingTrustAmount;
}

const TrustLineAmount &TrustLine::outgoingTrustAmount() const {

    return mOutgoingTrustAmount;
}

const TrustLineBalance &TrustLine::balance() const {

    return mBalance;
}

/*!
 * Returns amount that is availale to use on the trust line.
 */
ConstSharedTrustLineAmount TrustLine::availableAmount() const
{
    if (mBalance >= kZeroBalance()){
        if (mIncomingTrustAmount > kZeroAmount()) {
            return make_shared<const TrustLineAmount>(
                mIncomingTrustAmount - TrustLineAmount(mBalance));
        }
        return make_shared<const TrustLineAmount>(mBalance);
    }

    return make_shared<const TrustLineAmount>(
        mIncomingTrustAmount + mBalance);
}

/*!
 * Returns amount that is availale to use on the trust line from contractor node.
 */
ConstSharedTrustLineAmount TrustLine::availableIncomingAmount() const
{
    if (mBalance >= kZeroBalance()){
        if (mOutgoingTrustAmount > kZeroAmount()) {
            return make_shared<const TrustLineAmount>(
                mOutgoingTrustAmount - TrustLineAmount(mBalance));
        }

        return make_shared<const TrustLineAmount>(mBalance);
    }

    return make_shared<const TrustLineAmount>(
        mOutgoingTrustAmount + mBalance);
}

ConstSharedTrustLineAmount TrustLine::usedAmountByContractor() const {
    if (mBalance >= kZeroBalance()) {
        return make_shared<const TrustLineAmount>(mBalance);
    } else {
        return make_shared<const TrustLineAmount>(0);
    }
}

ConstSharedTrustLineAmount TrustLine::usedAmountBySelf() const {
    if (mBalance <= kZeroBalance()) {
        return make_shared<const TrustLineAmount>(-mBalance);
    } else {
        return make_shared<const TrustLineAmount>(0);
    }
}

const TrustLineDirection TrustLine::direction() const {

    if (mOutgoingTrustAmount > kZeroAmount() && mIncomingTrustAmount > kZeroAmount()) {
        return TrustLineDirection::Both;

    } else if (mOutgoingTrustAmount > kZeroAmount() && mIncomingTrustAmount == kZeroAmount()) {
        return TrustLineDirection::Outgoing;

    } else if (mOutgoingTrustAmount == kZeroAmount() && mIncomingTrustAmount > kZeroAmount()) {
        return TrustLineDirection::Incoming;

    } else {
        return TrustLineDirection::Nowhere;
    }
}

const BalanceRange TrustLine::balanceRange() const{

    if (mBalance > kZeroBalance()) {
        return BalanceRange::Positive;

    } else if (mBalance < kZeroBalance()) {
        return BalanceRange::Negative;

    } else {
        return BalanceRange::EqualsZero;
    }
}

/*!
 * Throws RuntimeError in case of unsuccessful serialization.
 */
vector<byte> TrustLine::serialize() {
    vector<byte> buffer;
    buffer.reserve(kRecordSize);

    try {
        trustAmountToBytes(
            mIncomingTrustAmount,
            buffer
        );

        trustAmountToBytes(
            mOutgoingTrustAmount,
            buffer
        );

        balanceToBytes(
            mBalance,
            buffer
        );

        directionStateToBytes(
            mTrustLineState.first,
            buffer
        );

        directionStateToBytes(
            mTrustLineState.second,
            buffer
        );

        // todo: (hsc) check if this serializes the sign

        return buffer;

    } catch (exception &e) {
        throw RuntimeError(string("TrustLine::serialize: can't serialize the trust line. Details: ") +
                               e.what());
    }
}

/*!
 * Throws RuntimeError in case of unsucessfull deserialization.
 */
void TrustLine::deserialize(
    const byte *buffer) {

    try {
        size_t bufferOffset = 0;

        parseTrustAmount(
            buffer,
            mIncomingTrustAmount
        );
        bufferOffset += kTrustAmountPartSize;

        parseTrustAmount(
            buffer + bufferOffset,
            mOutgoingTrustAmount
        );
        bufferOffset += kTrustAmountPartSize;

        parseBalance(
            buffer + bufferOffset
        );
        bufferOffset += kBalancePartSize + kSignBytePartSize;

        mTrustLineState.first = parseDirectionState(
            buffer + bufferOffset
        );
        bufferOffset += sizeof(SerializedTrustLineState);

        mTrustLineState.second = parseDirectionState(
            buffer + bufferOffset
        );

    } catch (exception &e) {
        throw RuntimeError(string("TrustLine::deserialize: can't deserialize buffer to trust line instance") +
                               e.what());
    }
}

/*!
 * todo: (hsc) think how to refactor this a little bit
 */
void TrustLine::trustAmountToBytes(
    const TrustLineAmount &amount,
    vector<byte> &buffer) {

    vector<byte> bytes = trustLineAmountToBytes(const_cast<TrustLineAmount&>(amount));
    buffer.insert(
      buffer.end(),
      bytes.begin(),
      bytes.end()
    );
}

/*!
 * todo: (hsc) think how to refactor this a little bit
 */
void TrustLine::balanceToBytes(
    const TrustLineBalance &balance,
    vector<byte> &buffer) {

    vector<byte> bytes = trustLineBalanceToBytes(const_cast<TrustLineBalance&>(balance));
    buffer.insert(
        buffer.end(),
        bytes.begin(),
        bytes.end()
    );
}

void TrustLine::directionStateToBytes(
    TrustState state,
    vector<byte> &buffer) {

    SerializedTrustLineState directionState = (SerializedTrustLineState) state;

    BytesShared dataBytesBuffer = tryMalloc(sizeof(SerializedTrustLineState));
    memcpy(
      dataBytesBuffer.get(),
      &directionState,
      sizeof(SerializedTrustLineState)
    );

    for (size_t it = 0; it < sizeof(SerializedTrustLineState); ++it) {
        buffer.push_back(dataBytesBuffer.get()[it]);
    }
}

/*!
 * todo: (hsc) think how to refactor this a little bit
 *
 * Parses trust line amount from "buffer" into "variable".
 *
 */
void TrustLine::parseTrustAmount(
    const byte *buffer,
    TrustLineAmount &variable) {

    vector<byte> bytesVector(
        buffer,
        buffer + kTrustAmountPartSize
    );

    variable = bytesToTrustLineAmount(bytesVector);
}

/*!
 * todo: (hsc) think how to refactor this a little bit
 *
 * Parses trust line balance from "buffer".
 *
 */
void TrustLine::parseBalance(
    const byte *buffer) {

    vector<byte> bytesVector(
        buffer,
        buffer + kBalancePartSize + kSignBytePartSize
    );

    mBalance = bytesToTrustLineBalance(bytesVector);
}

TrustState TrustLine::parseDirectionState(
    const byte *buffer) {

    SerializedTrustLineState *directionState = new (const_cast<byte *> (buffer)) SerializedTrustLineState;
    return static_cast<TrustState>(*directionState);
}

/*!
 * @returns static constant zero balance,
 * that is useful in comparison operations.
 */
const TrustLineBalance &TrustLine::kZeroBalance() {

    static TrustLineBalance zero(0);
    return zero;
}

/*!
 * @returns static constant zero amount,
 * that is useful in comparison operations.
 */
const TrustLineAmount &TrustLine::kZeroAmount() {

    static TrustLineAmount zero(0);
    return zero;
}

bool operator==(
    const TrustLine::Shared contractor1,
    const TrustLine::Shared contractor2) {

    return contractor1->contractorNodeUUID() == contractor2->contractorNodeUUID();
}

bool operator==(
    const TrustLine &contractor1,
    const TrustLine &contractor2) {

    return contractor1.contractorNodeUUID() == contractor2.contractorNodeUUID();
}

/**
 * Decreases debt of the contractor (if exists),
 * and uses credit to the current node (if needed).
 *
 * @param amount - specifies how much should be cleared/used.
 */
void TrustLine::pay(
    const TrustLineAmount &amount)
{
    const auto kNewBalance = mBalance - amount;
    if (abs(kNewBalance) > mIncomingTrustAmount)
        throw OverflowError(
            "TrustLine::useCredit: attempt of using more than incoming credit amount.");

    mBalance = kNewBalance;
}

/**
 * Clears debt of the current node (if exists)
 * and increases debt of the contractor (if exists).
 *
 * @param amount - specifies how much should be cleared/used.
 */
void TrustLine::acceptPayment(
    const TrustLineAmount &amount)
{
    const auto kNewBalance = mBalance + amount;
    if (abs(kNewBalance) > mOutgoingTrustAmount)
        throw OverflowError(
            "TrustLine::useCredit: attempt of using more than outgoing credit amount.");

    mBalance = kNewBalance;
}
