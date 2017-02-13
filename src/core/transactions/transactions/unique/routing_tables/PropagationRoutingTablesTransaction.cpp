#include "PropagationRoutingTablesTransaction.h"

PropagationRoutingTablesTransaction::PropagationRoutingTablesTransaction(
    NodeUUID &nodeUUID,
    NodeUUID &contractorUUID,
    TrustLineUUID &trustLineUUID,
    TransactionsScheduler *scheduler) :

    UniqueTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        nodeUUID,
        scheduler
    ),
    mContractorUUID(contractorUUID),
    mTrustLineUUID(trustLineUUID) {}

PropagationRoutingTablesTransaction::PropagationRoutingTablesTransaction(
    BytesShared buffer,
    TransactionsScheduler *scheduler) :

    UniqueTransaction(
        scheduler
    ){

    deserializeFromBytes(buffer);
}

const NodeUUID &PropagationRoutingTablesTransaction::contractorUUID() const {

    return mContractorUUID;
}

const TrustLineUUID &PropagationRoutingTablesTransaction::trustLineUUID() const {

    return mTrustLineUUID;
}

pair<BytesShared, size_t> PropagationRoutingTablesTransaction::serializeToBytes() const {}

void PropagationRoutingTablesTransaction::deserializeFromBytes(
    BytesShared buffer) {}

TransactionResult::SharedConst PropagationRoutingTablesTransaction::run() {

    switch(mStep) {

        case 1:{
            auto flagAndTransactionUUID = isTransactionToContractorUnique();
            if (!flagAndTransactionUUID.first) {
                killTransaction(flagAndTransactionUUID.second);
            }
            increaseStepsCounter();
        }

        default: {
            throw ConflictError("OpenTrustLineTransaction::run: "
                                    "Illegal step execution.");
        }
    }
}

pair<bool, const TransactionUUID> PropagationRoutingTablesTransaction::isTransactionToContractorUnique() {

    auto transactions = pendingTransactions();
    for (auto const &transactionsAndState : *transactions) {

        auto transaction = transactionsAndState.first;

        switch (transaction->transactionType()) {

            case BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType: {
                PropagationRoutingTablesTransaction::Shared propagationRoutingTableTransaction = static_pointer_cast<PropagationRoutingTablesTransaction>(transaction);
                if (mTransactionUUID != transaction->UUID()) {
                    if (mContractorUUID == propagationRoutingTableTransaction->contractorUUID()) {
                        if (mTrustLineUUID == propagationRoutingTableTransaction->trustLineUUID()) {
                            return make_pair(
                                false,
                                transaction->UUID()
                            );
                        }
                    }
                }
                break;
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

void PropagationRoutingTablesTransaction::sendFirstLevelRoutingTableToContractor() {

    FirstLevelRoutingTableOutgoingMessage::Shared firstLevelMessage = make_shared<FirstLevelRoutingTableOutgoingMessage>(
        mNodeUUID,
        mNodeUUID,
        mTrustLineUUID
    );

    auto firstLevelRoutingTable = emulateFirstLevelRoutingTable();

    for (const auto &neighborAndDirection : firstLevelRoutingTable) {
        firstLevelMessage->pushBack(
          neighborAndDirection.first,
          neighborAndDirection.second
        );
    }

    Message::Shared message = dynamic_pointer_cast<Message>(firstLevelMessage);
}

vector<pair<NodeUUID, TrustLineDirection>> PropagationRoutingTablesTransaction::emulateFirstLevelRoutingTable() {

    vector<pair<NodeUUID, TrustLineDirection>> firstLevelRoutingTable;
    const int kMaxRandomValue = 2;

    for (size_t iterator = 0; iterator < 10000; ++iterator) {
        NodeUUID contractorUUID;
        TrustLineDirection trustLineDirection;

        int randomValue = rand() % kMaxRandomValue;
        switch (randomValue) {

            case 0: {
                trustLineDirection = TrustLineDirection::Incoming;
            }

            case 1: {
                trustLineDirection = TrustLineDirection::Outgoing;
            }

            case 2: {
                trustLineDirection = TrustLineDirection::Both;
            }

            default: {
                trustLineDirection = TrustLineDirection::Nowhere;
            }
        }

        firstLevelRoutingTable.push_back(
          make_pair(
              contractorUUID,
              trustLineDirection
          )
        );
    }

    return firstLevelRoutingTable;
}