#include "TrustLine.h"

TrustLine::TrustLine(
    const NodeUUID &nodeUUID,
    const trust_amount &incomingAmount,
    const trust_amount &outgoingAmount,
    const balance_value &nodeBalance) {

    // todo: use initialisation lists when possible
    mContractorNodeUuid = nodeUUID;
    mIncomingTrustAmount = incomingAmount;
    mOutgoingTrustAmount = outgoingAmount;
    mBalance = nodeBalance;
}

// todo: remove
//void TrustLine::setContractorNodeUUID(
//        const NodeUUID &nodeUUID) {
//    mContractorNodeUuid = nodeUUID;
//}

void TrustLine::setIncomingTrustAmount(
    const trust_amount &amount,
    callback callback) {

    mIncomingTrustAmount = amount;
    mManagerCallback = callback; // todo: why this callback is assigned here? when it would be called again?
    mManagerCallback();
}

void TrustLine::setOutgoingTrustAmount(
    const trust_amount &amount,
    callback callback) {

    mOutgoingTrustAmount = amount;
    mManagerCallback = callback; // todo: why this callback is assigned here? when it would be called again?
    mManagerCallback();
}

void TrustLine::setBalance(
    const balance_value &balance,
    callback callback) {

    mBalance = balance;
    mManagerCallback = callback; // todo: why this callback is assigned here? when it would be called again?
    mManagerCallback();
}

const NodeUUID &TrustLine::getContractorNodeUUID() const{
    return mContractorNodeUuid;
}

const trust_amount &TrustLine::getIncomingTrustAmount() const{
    return mIncomingTrustAmount;
}

const trust_amount &TrustLine::getOutgoingTrustAmount() const{
    return mOutgoingTrustAmount;
}

const balance_value &TrustLine::getBalance() const{
    return mBalance;
}

