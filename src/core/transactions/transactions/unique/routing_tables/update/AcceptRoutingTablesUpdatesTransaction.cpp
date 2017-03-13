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

    string logLine;
    cout << "Update routing table message received" << endl;
    logLine = "SenderUUID:" + mMessage->senderUUID().stringUUID() + "::";
    cout << "Sender UUID -> " << mMessage->senderUUID() << endl;
    logLine += "InitiatorUUID:" + mMessage->initiatorUUID().stringUUID() + "::";
    cout << "Initiator UUID -> " << mMessage->initiatorUUID() << endl;
    logLine += "ContractorUUID:" + mMessage->contractorUUID().stringUUID() + "::";
    cout << "Contractor UUID -> " << mMessage->contractorUUID() << endl;
    logLine += "Direction:" + to_string(mMessage->direction());
    cout << "Direction -> " << mMessage->direction() << endl;

    mFileLogger->addLine(logLine.c_str());
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

            launchSubsidiaryTransaction(updatesPropagationTransaction);

        }

    }
}

void AcceptRoutingTablesUpdatesTransaction::sendResponseToContractor(
    const uint16_t code) {

    Message *message = new Response(
        mNodeUUID,
        mMessage->transactionUUID(),
        code);

    addMessage(
        Message::Shared(message),
        mMessage->senderUUID());
}