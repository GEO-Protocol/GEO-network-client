#include "TrustLine.h"

TrustLine::TrustLine(
    const NodeUUID &nodeUUID,
    const TrustLineAmount &incomingAmount,
    const TrustLineAmount &outgoingAmount,
    const TrustLineBalance &nodeBalance) :

    mContractorNodeUuid(nodeUUID),
    mIncomingTrustAmount(incomingAmount),
    mOutgoingTrustAmount(outgoingAmount),
    mBalance(nodeBalance) {}

TrustLine::TrustLine(
    const byte *buffer,
    const NodeUUID &contractorUUID) :

    mContractorNodeUuid(contractorUUID) {

    deserializeTrustLine(
        buffer);
}

void TrustLine::setIncomingTrustAmount(
    const TrustLineAmount &amount,
    SaveTrustLineCallback callback) {

    mIncomingTrustAmount = amount;
    callback();
}

void TrustLine::setOutgoingTrustAmount(
    const TrustLineAmount &amount,
    SaveTrustLineCallback callback) {

    mOutgoingTrustAmount = amount;
    callback();
}

void TrustLine::setBalance(
    const TrustLineBalance &balance,
    SaveTrustLineCallback callback) {

    mBalance = balance;
    callback();
}

void TrustLine::activateOutgoingDirection() {

    mTrustLineState.second = TrustState::Active;
}

void TrustLine::suspendOutgoingDirection() {

    mTrustLineState.second = TrustState::Suspended;
}

void TrustLine::pendingSuspendOutgoingDirection() {

    mTrustLineState.second = TrustState::PendingSuspend;
}

void TrustLine::activateIncomingDirection() {

    mTrustLineState.first = TrustState::Active;
}

void TrustLine::pendingSuspendIncomingDirection() {

    mTrustLineState.first = TrustState::PendingSuspend;
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

const TrustLineDirection TrustLine::direction() const {

    if (mOutgoingTrustAmount > TrustLineAmount(0) && mIncomingTrustAmount > TrustLineAmount(0)) {
        return TrustLineDirection::Both;

    } else if (mOutgoingTrustAmount > TrustLineAmount(0) && mIncomingTrustAmount == TrustLineAmount(0)) {
        return TrustLineDirection::Outgoing;

    } else if (mOutgoingTrustAmount == TrustLineAmount(0) && mIncomingTrustAmount > TrustLineAmount(0)) {
        return TrustLineDirection::Incoming;

    } else {
        return TrustLineDirection::Nowhere;
    }
}

const BalanceRange TrustLine::balanceRange() const{

    if (mBalance > TrustLineBalance(0)) {
        return BalanceRange::Positive;

    } else if (mBalance < TrustLineBalance(0)) {
        return BalanceRange::Negative;

    } else {
        return BalanceRange::EqualsZero;
    }
}

vector<byte> TrustLine::serializeTrustLine() {
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

    } catch (...) {
        buffer.clear();
        throw RuntimeError("Can't serialize trust line instance to buffer");
    }

    return buffer;
}

void TrustLine::deserializeTrustLine(
    const byte *buffer) {

    try {
        parseTrustAmount(
            buffer,
            mIncomingTrustAmount);

        parseTrustAmount(
            buffer + kTrustAmountPartSize,
            mOutgoingTrustAmount);

        parseBalance(
            buffer + kTrustAmountPartSize * 2);

    } catch (...) {
        throw RuntimeError("Can't deserialize buffer to trust line instance");
    }
}

void TrustLine::trustAmountToBytes(
    const TrustLineAmount &amount,
    vector<byte> &buffer) {

    size_t oldSize = buffer.size();
    export_bits(amount, back_inserter(buffer), 8);

    size_t newSize = buffer.size();
    for (size_t i = 0; i < kTrustAmountPartSize - (newSize - oldSize); ++i) {
        buffer.push_back(0);
    }
}

void TrustLine::balanceToBytes(
    const TrustLineBalance &balance,
    vector<byte> &buffer) {

    size_t oldSize = buffer.size();
    export_bits(balance, back_inserter(buffer), 8);

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

void TrustLine::parseTrustAmount(
    const byte *buffer,
    TrustLineAmount &variable) {

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
        import_bits(variable, notZeroBytesVector.begin(), notZeroBytesVector.end());

    } else {
        import_bits(variable, bytesVector.begin(), bytesVector.end());
    }
}

void TrustLine::parseBalance(
    const byte *buffer) {

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
        import_bits(mBalance, notZeroBytesVector.begin(), notZeroBytesVector.end());

    } else {
        import_bits(mBalance, bytesVector.begin(), bytesVector.end());
    }

    if (sign == 1) {
        mBalance = mBalance * -1;
    }
}




