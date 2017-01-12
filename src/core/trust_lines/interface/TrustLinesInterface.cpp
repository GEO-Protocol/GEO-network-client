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
