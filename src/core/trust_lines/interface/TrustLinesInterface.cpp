#include "TrustLinesInterface.h"

TrustLinesInterface::TrustLinesInterface(
        TrustLinesManager *manager) :

        mManager(manager){}

TrustLinesInterface::~TrustLinesInterface() {

}

void TrustLinesInterface::open(
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount) {

    mManager->open(
            contractorUUID,
            amount);
}

void TrustLinesInterface::accept(
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount) {

    mManager->accept(
            contractorUUID,
            amount);
}

void TrustLinesInterface::close(
        const NodeUUID &contractorUUID) {

    mManager->close(
            contractorUUID);
}

void TrustLinesInterface::reject(
        const NodeUUID &contractorUUID) {

    mManager->reject(
            contractorUUID);
}

const bool TrustLinesInterface::isExist(
    const NodeUUID &contractorUUID) {

    return mManager->isTrustLineExist(
        contractorUUID);
}

const bool TrustLinesInterface::isDirectionOutgoing(
    const NodeUUID &contractorUUID) {

    return mManager->checkDirection(
        contractorUUID,
        TrustLineDirection::Outgoing);
}

const bool TrustLinesInterface::isDirectionIncoming(
    const NodeUUID &contractorUUID) {

    return mManager->checkDirection(
        contractorUUID,
        TrustLineDirection::Incoming);
}

const bool TrustLinesInterface::checkOutgoingAmount(
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount) {

    return mManager->trustLineByContractorUUID(contractorUUID)->outgoingTrustAmount() == amount;
}

const bool TrustLinesInterface::checkIncomingAmount(
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount) {

    return mManager->trustLineByContractorUUID(contractorUUID)->incomingTrustAmount() == amount;
}
