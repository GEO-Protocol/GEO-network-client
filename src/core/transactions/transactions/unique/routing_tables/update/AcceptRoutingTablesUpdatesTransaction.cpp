#include "AcceptRoutingTablesUpdatesTransaction.h"

AcceptRoutingTablesUpdatesTransaction::AcceptRoutingTablesUpdatesTransaction(
    const NodeUUID &nodeUUID,
    RoutingTableUpdateIncomingMessage::Shared routingTableUpdateMessage,
    TrustLinesManager *trustLinesManager):

    BaseTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesUpdatesTransactionType,
        nodeUUID),
    mMessage(routingTableUpdateMessage),
    mTrustLinesManager(trustLinesManager) {}

RoutingTableUpdateIncomingMessage::Shared AcceptRoutingTablesUpdatesTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst AcceptRoutingTablesUpdatesTransaction::run() {

    updateRoutingTable();
    sendResponseToContractor(kResponseCodeSuccess);
    tryCreateNextUpdatingTransactionsPool();

    return finishTransaction();
}

void AcceptRoutingTablesUpdatesTransaction::updateRoutingTable() {

    cout << "Update routing table message received" << endl;
    cout << "Sender UUID -> " << mMessage->senderUUID() << endl;
    cout << "Initiator UUID -> " << mMessage->initiatorUUID() << endl;
    cout << "Contractor UUID -> " << mMessage->contractorUUID() << endl;
    cout << "Direction -> " << mMessage->direction() << endl;
}

void AcceptRoutingTablesUpdatesTransaction::tryCreateNextUpdatingTransactionsPool() {

    if (mMessage->updatingStep() == RoutingTableUpdateOutgoingMessage::UpdatingStep::FirstLevelNodes) {

        for (const auto &contractorAndTrustLine : mTrustLinesManager->trustLines()) {

            if (mMessage->initiatorUUID() == contractorAndTrustLine.first || mMessage->contractorUUID() == contractorAndTrustLine.first) {
                continue;
            }

            BaseTransaction::Shared updatesPropagationTransaction = make_shared<PropagateRoutingTablesUpdatesTransaction>(
                mNodeUUID,
                mMessage->initiatorUUID(),
                mMessage->contractorUUID(),
                mMessage->direction(),
                contractorAndTrustLine.first,
                RoutingTableUpdateOutgoingMessage::UpdatingStep::SecondLevelNodes);

            launchSubsidiaryTransaction(
                updatesPropagationTransaction);

        }

    }
}

void AcceptRoutingTablesUpdatesTransaction::sendResponseToContractor(
    const uint16_t code) {

    sendMessage<Response>(
        mMessage->senderUUID(),
        mNodeUUID,
        mMessage->transactionUUID(),
        code);
}