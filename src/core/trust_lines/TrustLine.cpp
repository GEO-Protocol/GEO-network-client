#include "TrustLine.h"

TrustLine::TrustLine(
    const NodeUUID &nodeUUID,
    const TrustLineAmount &incomingAmount,
    const TrustLineAmount &outgoingAmount,
    const TrustLineBalance &nodeBalance) :

    mContractorNodeUuid(nodeUUID),
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
}

TrustLine::TrustLine(
    const NodeUUID &nodeUUID,
    const TrustLineAmount &incomingAmount,
    const TrustLineAmount &outgoingAmount):

    mContractorNodeUuid(nodeUUID),
    mIncomingTrustAmount(incomingAmount),
    mOutgoingTrustAmount(outgoingAmount),
    mBalance(0) {}

TrustLine::TrustLine(
    const byte *buffer,
    const NodeUUID &contractorUUID) :

    mContractorNodeUuid(contractorUUID) {
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

    return mContractorNodeUuid;
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
ConstSharedTrustLineAmount TrustLine::availableAmount() const {

    if (mBalance >= kZeroBalance()){
        if (mIncomingTrustAmount > kZeroAmount()) {
            return ConstSharedTrustLineAmount(
                new TrustLineAmount(
                    mIncomingTrustAmount - TrustLineAmount(mBalance)
                )
            );
        }

        return ConstSharedTrustLineAmount(
            new TrustLineAmount(
                mBalance
            )
        );
    }

    return ConstSharedTrustLineAmount(
        new TrustLineAmount(
            mIncomingTrustAmount + mBalance
        )
    );
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
 * Throws RuntimeError in case of unsucessfull serialization.
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
        // todo: (hsc) check if this serializes the sign

        return buffer;

    } catch (exception &e) {
        throw RuntimeError(
            string("TrustLine::serialize: can't serialize the trust line. Details: ") +
            e.what());
    }
}

/*!
 * Throws RuntimeError in case of unsucessfull deserialization.
 */
void TrustLine::deserialize(
    const byte *buffer) {

    try {
        parseTrustAmount(
            buffer,
            mIncomingTrustAmount
        );

        parseTrustAmount(
            buffer + kTrustAmountPartSize,
            mOutgoingTrustAmount
        );

        parseBalance(
            buffer + kTrustAmountPartSize * 2
        );

    } catch (exception &e) {
        throw RuntimeError(
            string("TrustLine::deserialize: can't deserialize buffer to trust line instance") +
            e.what());
    }
}

/*!
 * todo: (hsc) think how to refactor this a little bit
 */
void TrustLine::trustAmountToBytes(
    const TrustLineAmount &amount,
    vector<byte> &buffer) {

    size_t oldSize = buffer.size();
    export_bits(
        amount,
        back_inserter(buffer),
        8
    );

    size_t newSize = buffer.size();
    for (size_t i = 0; i < kTrustAmountPartSize - (newSize - oldSize); ++i) {
        buffer.push_back(0);
    }
}

/*!
 * todo: (hsc) think how to refactor this a little bit
 */
void TrustLine::balanceToBytes(
    const TrustLineBalance &balance,
    vector<byte> &buffer) {

    size_t oldSize = buffer.size();
    export_bits(
        balance,
        back_inserter(buffer),
        8
    );

    size_t newSize = buffer.size();
    for (size_t i = 0; i < kBalancePartSize - (newSize - oldSize); ++i) {
        buffer.push_back(0);
    }

    if (balance.sign() == -1) {
        buffer.push_back(1);
    } else {
        buffer.push_back(0);
    }
}

/*!
 * todo: (hsc) think how to refactor this a little bit
 *
 * Parses trust line amount from "buffer" into "variable".
 *
 * Throws MemoryError;
 */
void TrustLine::parseTrustAmount(
    const byte *buffer,
    TrustLineAmount &variable) {

    try {
        vector<byte> bytesVector(
            buffer,
            buffer + kTrustAmountPartSize
        );

        vector<byte> notZeroBytesVector;
        notZeroBytesVector.reserve(kTrustAmountPartSize);
        for (auto &item : bytesVector) {
            if (item != 0) {
                notZeroBytesVector.push_back(item);
            }
        }

        if (notZeroBytesVector.size() > 0) {
            import_bits(
                variable,
                notZeroBytesVector.begin(),
                notZeroBytesVector.end()
            );

        } else {
            import_bits(
                variable,
                bytesVector.begin(),
                bytesVector.end()
            );
        }

    } catch (bad_alloc &){
        throw MemoryError(
            "TrustLine::parseTrustAmount: bad alloc.");
    }
}

/*!
 * todo: (hsc) think how to refactor this a little bit
 *
 * Parses trust line balance from "buffer".
 *
 *
 * Throws MemoryError;
 */
void TrustLine::parseBalance(
    const byte *buffer) {

    try {
        vector<byte> bytesVector(
            buffer,
            buffer + kTrustAmountPartSize
        );

        vector<byte> notZeroBytesVector;
        notZeroBytesVector.reserve(kTrustAmountPartSize);

        byte sign = buffer[kBalancePartSize + kSignBytePartSize - 1];

        for (auto &item : bytesVector) {
            if (item != 0) {
                notZeroBytesVector.push_back(item);
            }
        }

        if (notZeroBytesVector.size() > 0) {
            import_bits(
                mBalance,
                notZeroBytesVector.begin(),
                notZeroBytesVector.end()
            );

        } else {
            import_bits(
                mBalance,
                bytesVector.begin(),
                bytesVector.end()
            );
        }

        if (sign == 1) {
            mBalance = mBalance * -1;
        }

    } catch (bad_alloc &) {
        throw MemoryError(
            "TrustLine::parseTrustAmount: bad alloc.");
    }
}

/*!
 * @returns static constant zero balance,
 * that is useful in comparison operations.
 */
const TrustLineBalance &TrustLine::kZeroBalance() {
    // NOTE: "zero" would be common for ALL instances of this class.
    static TrustLineBalance zero(0);
    return zero;
}

/*!
 * @returns static constant zero amount,
 * that is useful in comparison operations.
 */
const TrustLineAmount &TrustLine::kZeroAmount() {
    // NOTE: "zero" would be common for ALL instances of this class.
    static TrustLineAmount zero(0);
    return zero;
}
