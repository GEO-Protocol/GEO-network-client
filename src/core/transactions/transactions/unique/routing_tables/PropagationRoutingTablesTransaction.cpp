#include "PropagationRoutingTablesTransaction.h"

PropagationRoutingTablesTransaction::PropagationRoutingTablesTransaction(
    NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const TrustLineUUID &trustLineUUID,
    TransactionsScheduler *scheduler,
    TrustLinesManager *trustLinesManager) :

    UniqueTransaction(
        BaseTransaction::TransactionType::PropagationRoutingTablesTransactionType,
        nodeUUID,
        scheduler
    ),
    mContractorUUID(contractorUUID),
    mTrustLineUUID(trustLineUUID),
    mTrustLinesManager(trustLinesManager) {}

PropagationRoutingTablesTransaction::PropagationRoutingTablesTransaction(
    BytesShared buffer,
    TransactionsScheduler *scheduler,
    TrustLinesManager *trustLinesManager) :

    UniqueTransaction(
        scheduler
    ),
    mTrustLinesManager(trustLinesManager) {

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

        case 1: {
            auto flagAndTransactionUUID = isTransactionToContractorUnique();
            if (!flagAndTransactionUUID.first) {
                killTransaction(flagAndTransactionUUID.second);
            }
            increaseStepsCounter();
        }

        case 2: {
            if (mContext != nullptr) {
                return checkFirstLevelRoutingTablePropagationContext();

            } else {
                if (mRequestCounter < kMaxRequestsCount) {
                    if (!isContractorsCountEnoughForRoutingTablePropagation()) {
                        return finishTransaction();
                    }
                    sendFirstLevelRoutingTableToContractor();
                    if (mRequestCounter > 0) {
                        mConnectionTimeout *= kConnectionProgression;
                    }
                    mRequestCounter ++;

                } else {
                    return breakTransaction();
                }
            }
            return waitingForFirstLevelRoutingTablePropagationResponse();
        }

        default: {
            throw ConflictError("PropagationRoutingTablesTransaction::run: "
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

TransactionResult::SharedConst PropagationRoutingTablesTransaction::checkFirstLevelRoutingTablePropagationContext() {

    if (mContext->typeID() == Message::MessageTypeID::ResponseMessageType) {
        Response::Shared response = static_pointer_cast<Response>(mContext);
        switch (response->code()) {

            case 200: {
                return finishTransaction();
            }

            default: {

                return breakTransaction();
            }
        }
    }

    return breakTransaction();
}

bool PropagationRoutingTablesTransaction::isContractorsCountEnoughForRoutingTablePropagation() {

    return mTrustLinesManager->trustLines().size() > 1;
}

void PropagationRoutingTablesTransaction::sendFirstLevelRoutingTableToContractor() {

    FirstLevelRoutingTableOutgoingMessage::Shared firstLevelMessage = make_shared<FirstLevelRoutingTableOutgoingMessage>(
        mNodeUUID,
        mTrustLineUUID
    );

    vector<pair<NodeUUID, TrustLineDirection>> neighborsAndDirections;
    for (const auto &contractorAndTrustLine : mTrustLinesManager->trustLines()) {
        if (mContractorUUID == contractorAndTrustLine.first) {
            continue;
        }

        neighborsAndDirections.push_back(
            make_pair(
                contractorAndTrustLine.first,
                mTrustLinesManager->trustLine(contractorAndTrustLine.first)->direction()
            )
        );
    }

    firstLevelMessage->pushBack(
        mNodeUUID,
        neighborsAndDirections
    );

    Message::Shared message = dynamic_pointer_cast<Message>(firstLevelMessage);
    addMessage(
        message,
        mContractorUUID
    );
}

TransactionResult::SharedConst PropagationRoutingTablesTransaction::waitingForFirstLevelRoutingTablePropagationResponse() {

    TransactionState *transactionState = new TransactionState(
        microsecondsSinceGEOEpoch(
            utc_now() + pt::microseconds(mConnectionTimeout * 1000)
        ),
        Message::MessageTypeID::ResponseMessageType,
        false
    );


    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(TransactionState::SharedConst(transactionState));
    return TransactionResult::SharedConst(transactionResult);
}

TransactionResult::SharedConst PropagationRoutingTablesTransaction::breakTransaction() {

    return make_shared<const TransactionResult>(
        TransactionState::exit()
    );
}

TransactionResult::SharedConst PropagationRoutingTablesTransaction::finishTransaction() {

    return TransactionResult::finishSuccessfulWithoutResult();
}

vector<pair<NodeUUID, TrustLineDirection>> PropagationRoutingTablesTransaction::emulateSecondLevelRoutingTable() {

    vector<pair<NodeUUID, TrustLineDirection>> secondLevelRoutingTable;
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

        secondLevelRoutingTable.push_back(
          make_pair(
              contractorUUID,
              trustLineDirection
          )
        );
    }

    return secondLevelRoutingTable;
}