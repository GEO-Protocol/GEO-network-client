#include "FromInitiatorToContractorRoutingTablesAcceptTransaction.h"

FromInitiatorToContractorRoutingTablesAcceptTransaction::FromInitiatorToContractorRoutingTablesAcceptTransaction(
    NodeUUID &nodeUUID,
    FirstLevelRoutingTableIncomingMessage::Shared message,
    TransactionsScheduler *scheduler) :

    RoutingTablesTransaction(
        BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType,
        nodeUUID,
        const_cast<NodeUUID&> (message->senderUUID()),
        scheduler
    ),

    mFirstLevelMessage(message) {}

FromInitiatorToContractorRoutingTablesAcceptTransaction::FromInitiatorToContractorRoutingTablesAcceptTransaction(
    BytesShared buffer,
    TransactionsScheduler *scheduler) :

    RoutingTablesTransaction(
        buffer,
        scheduler) {}

FirstLevelRoutingTableIncomingMessage::Shared FromInitiatorToContractorRoutingTablesAcceptTransaction::message() const {

    return mFirstLevelMessage;
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesAcceptTransaction::run() {

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
            throw ConflictError("FromInitiatorToContractorRoutingTablesAcceptTransaction::run: "
                                    "Illegal step execution.");
        }

    }
}

pair<bool, const TransactionUUID> FromInitiatorToContractorRoutingTablesAcceptTransaction::isTransactionToContractorUnique() {

    auto transactions = pendingTransactions();
    for (auto const &transactionsAndState : *transactions) {

        auto transaction = transactionsAndState.first;

        switch (transaction->transactionType()) {

            case BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType: {

                FromInitiatorToContractorRoutingTablePropagationTransaction::Shared propagationRoutingTableTransaction = static_pointer_cast<FromInitiatorToContractorRoutingTablePropagationTransaction>(transaction);
                if (mTransactionUUID != propagationRoutingTableTransaction->UUID()) {
                    continue;
                }

                if (mContractorUUID == propagationRoutingTableTransaction->contractorUUID()) {
                    continue;
                }

                return make_pair(
                    false,
                    transaction->UUID()
                );

            }

            case BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType: {

                FromInitiatorToContractorRoutingTablesAcceptTransaction::Shared acceptRoutingTableTransaction = static_pointer_cast<FromInitiatorToContractorRoutingTablesAcceptTransaction>(transaction);
                if (mTransactionUUID != acceptRoutingTableTransaction->UUID()) {
                    continue;
                }

                if (mContractorUUID == acceptRoutingTableTransaction->contractorUUID()) {
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

void FromInitiatorToContractorRoutingTablesAcceptTransaction::saveFirstLevelRoutingTable() {

    cout << "First level routing table message received " << endl;
    cout << "Sender UUID -> " << mFirstLevelMessage->senderUUID().stringUUID() << endl;
    cout << "Routing table " << endl;
    for (const auto &nodeAndRecords : mFirstLevelMessage->mRecords) {
        cout << "Node UUID -> " << nodeAndRecords.first.stringUUID() << endl;
        for (const auto &neighborAndDirect : nodeAndRecords.second) {
            cout << "Neighbor UUID -> " << neighborAndDirect.first.stringUUID() << endl;
            cout << "Direction -> " << neighborAndDirect.second << endl;
        }
    }
}

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesAcceptTransaction::waitingForSecondLevelRoutingTableState() {

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

TransactionResult::SharedConst FromInitiatorToContractorRoutingTablesAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable() {

    if (!mContext.empty()) {
        SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage;

        for (const auto& incomingMessage : mContext) {
            if (incomingMessage->typeID() == Message::MessageTypeID::SecondLevelRoutingTableIncomingMessageType) {
                secondLevelMessage = static_pointer_cast<SecondLevelRoutingTableIncomingMessage>(incomingMessage);
            }
        }

        if (secondLevelMessage == nullptr) {
            throw ConflictError("FromInitiatorToContractorRoutingTablesAcceptTransaction::checkIncomingMessageForSecondLevelRoutingTable: "
                                    "Can not cast to SecondLevelRoutingTableIncomingMessage.");
        }
        saveSecondLevelRoutingTable(secondLevelMessage);
        sendResponseToContractor(
            const_cast<NodeUUID&> (secondLevelMessage->senderUUID()),
            200
        );
        return finishTransaction();

    } else {
        cout << "TRY RECEIVE SECOND LEVEL ROUTING TABLE AGAIN" << endl;
        return waitingForSecondLevelRoutingTableState();
    }
}

void FromInitiatorToContractorRoutingTablesAcceptTransaction::saveSecondLevelRoutingTable(
    SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage) {

    cout << "Second level routing table message received " << endl;
    cout << "Sender UUID -> " << secondLevelMessage->senderUUID().stringUUID() << endl;
    cout << "Routing table " << endl;
    for (const auto &nodeAndRecords : secondLevelMessage->mRecords) {
        cout << "Node UUID -> " << nodeAndRecords.first.stringUUID() << endl;
        for (const auto &neighborAndDirect : nodeAndRecords.second) {
            cout << "Neighbor UUID -> " << neighborAndDirect.first.stringUUID() << endl;
            cout << "Direction -> " << neighborAndDirect.second << endl;
        }
    }
}

void FromInitiatorToContractorRoutingTablesAcceptTransaction::sendResponseToContractor(
    NodeUUID &contractorUUID,
    uint16_t code) {

    Message *message = new RoutingTablesResponse(
        mNodeUUID,
        code
    );

    addMessage(
        Message::Shared(message),
        contractorUUID
    );
}