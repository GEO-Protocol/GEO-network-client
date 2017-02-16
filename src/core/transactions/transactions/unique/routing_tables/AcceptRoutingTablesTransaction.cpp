#include "AcceptRoutingTablesTransaction.h"

AcceptRoutingTablesTransaction::AcceptRoutingTablesTransaction(
    NodeUUID &nodeUUID,
    FirstLevelRoutingTableIncomingMessage::Shared message,
    TransactionsScheduler *scheduler) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        nodeUUID,
        message->senderUUID(),
        message->trustLineUUID(),
        scheduler
    ),

    mFirstLevelMessage(message) {}

AcceptRoutingTablesTransaction::AcceptRoutingTablesTransaction(
    BytesShared buffer,
    TransactionsScheduler *scheduler) :

    RoutingTablesTransaction(
        buffer,
        scheduler) {}

FirstLevelRoutingTableIncomingMessage::Shared AcceptRoutingTablesTransaction::message() const {

    return mFirstLevelMessage;
}

pair<BytesShared, size_t> AcceptRoutingTablesTransaction::serializeToBytes() const {}

void AcceptRoutingTablesTransaction::deserializeFromBytes(BytesShared buffer) {}

TransactionResult::SharedConst AcceptRoutingTablesTransaction::run() {

    if (!isUniqueWasChecked) {
        auto flagAndTransactionUUID = isTransactionToContractorUnique();
        if (!flagAndTransactionUUID.first) {
            killTransaction(flagAndTransactionUUID.second);
        }
        isUniqueWasChecked = true;
    }

    switch (mStep) {

        case RoutingTableLevelStepIdentifier::FirstLevelRoutingTableStep: {
            saveFirstLevelRoutingTable();
            sendResponseToContractor(
                const_cast<TransactionUUID&> (mFirstLevelMessage->transactionUUID()),
                const_cast<NodeUUID&> (mFirstLevelMessage->senderUUID()),
                200
            );
            increaseStepsCounter();
            return waitingForSecondLevelRoutingTableState();
        }

        case RoutingTableLevelStepIdentifier::SecondLevelRoutingTableStep: {
            return checkIncomingMessageForSecondLevelRoutingTable();
        }

        default: {
            break;
        }

    }
}

pair<bool, const TransactionUUID> AcceptRoutingTablesTransaction::isTransactionToContractorUnique() {

    auto transactions = pendingTransactions();
    for (auto const &transactionsAndState : *transactions) {

        auto transaction = transactionsAndState.first;

        switch (transaction->transactionType()) {

            case BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType: {

                PropagationRoutingTablesTransaction::Shared propagationRoutingTableTransaction = static_pointer_cast<PropagationRoutingTablesTransaction>(transaction);
                if (mTransactionUUID != propagationRoutingTableTransaction->UUID()) {
                    continue;
                }

                if (mContractorUUID != propagationRoutingTableTransaction->contractorUUID()) {
                    continue;
                }

                if (mTrustLineUUID == propagationRoutingTableTransaction->trustLineUUID()) {
                    continue;
                }

                return make_pair(
                    false,
                    transaction->UUID()
                );

            }

            case BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType: {

                AcceptRoutingTablesTransaction::Shared acceptRoutingTableTransaction = static_pointer_cast<AcceptRoutingTablesTransaction>(transaction);
                if (mTransactionUUID != acceptRoutingTableTransaction->UUID()) {
                    continue;
                }

                if (mContractorUUID != acceptRoutingTableTransaction->contractorUUID()) {
                    continue;
                }

                if (mTrustLineUUID == acceptRoutingTableTransaction->trustLineUUID()) {
                    continue;
                }

                return make_pair(
                    false,
                    transaction->UUID()
                );

            }

            default: {
                break;
            }

        }

    }

    return make_pair(
        true,
        TransactionUUID()
    );
}

void AcceptRoutingTablesTransaction::saveFirstLevelRoutingTable() {

    cout << "First level routing table message received " << endl;
    cout << "Sender UUID -> " << mFirstLevelMessage->senderUUID().stringUUID() << endl;
    cout << "Trust line UUID -> " << mFirstLevelMessage->trustLineUUID().stringUUID() << endl;
    cout << "Routing table " << endl;
    for (const auto &nodeAndRecords : mFirstLevelMessage->mRecords) {
        cout << "Node UUID -> " << nodeAndRecords.first.stringUUID() << endl;
        for (const auto &neighborAndDirect : nodeAndRecords.second) {
            cout << "Neighbor UUID -> " << neighborAndDirect.first.stringUUID() << endl;
            cout << "Direction -> " << neighborAndDirect.second << endl;
        }
    }
}

TransactionResult::SharedConst AcceptRoutingTablesTransaction::waitingForSecondLevelRoutingTableState() {

    TransactionState *transactionState = new TransactionState(
        microsecondsSinceGEOEpoch(
            utc_now() + pt::microseconds(mConnectionTimeout * 1000)
        ),
        Message::MessageTypeID::SecondLevelRoutingTableIncomingMessageType,
        false
    );


    return transactionResultFromState(
        TransactionState::SharedConst(transactionState)
    );
}

TransactionResult::SharedConst AcceptRoutingTablesTransaction::checkIncomingMessageForSecondLevelRoutingTable() {

    if (!mContext.empty()) {
        SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage;

        for (const auto& incomingMessage : mContext) {
            if (incomingMessage->typeID() == Message::MessageTypeID::SecondLevelRoutingTableIncomingMessageType) {
                secondLevelMessage = static_pointer_cast<SecondLevelRoutingTableIncomingMessage>(incomingMessage);
            }
        }

        if (secondLevelMessage == nullptr) {
            throw ConflictError("AcceptRoutingTablesTransaction::checkIncomingMessageForSecondLevelRoutingTable: "
                                    "Can not cast to SecondLevelRoutingTableIncomingMessage.");
        }
        saveSecondLevelRoutingTable(secondLevelMessage);
        sendResponseToContractor(
            const_cast<TransactionUUID&> (secondLevelMessage->transactionUUID()),
            const_cast<NodeUUID&> (secondLevelMessage->senderUUID()),
            200
        );
        return finishTransaction();

    } else {
        throw ConflictError("AcceptRoutingTablesTransaction::checkIncomingMessageForSecondLevelRoutingTable: "
                                "There are no incoming messages.");
    }
}

void AcceptRoutingTablesTransaction::saveSecondLevelRoutingTable(
    SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage) {

    cout << "Second level routing table message received " << endl;
    cout << "Sender UUID -> " << secondLevelMessage->senderUUID().stringUUID() << endl;
    cout << "Trust line UUID -> " << secondLevelMessage->trustLineUUID().stringUUID() << endl;
    cout << "Routing table " << endl;
    for (const auto &nodeAndRecords : secondLevelMessage->mRecords) {
        cout << "Node UUID -> " << nodeAndRecords.first.stringUUID() << endl;
        for (const auto &neighborAndDirect : nodeAndRecords.second) {
            cout << "Neighbor UUID -> " << neighborAndDirect.first.stringUUID() << endl;
            cout << "Direction -> " << neighborAndDirect.second << endl;
        }
    }
}

void AcceptRoutingTablesTransaction::sendResponseToContractor(
    TransactionUUID &transactionUUID,
    NodeUUID &contractorUUID,
    uint16_t code) {

    Message *message = new Response(
        mNodeUUID,
        transactionUUID,
        code
    );

    addMessage(
        Message::Shared(message),
        contractorUUID
    );
}