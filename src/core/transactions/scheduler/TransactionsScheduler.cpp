#include "TransactionsScheduler.h"

TransactionsScheduler::TransactionsScheduler(
    as::io_service &IOService,
    storage::UUIDMapBlockStorage *storage,
    Logger *logger) :

    mIOService(IOService),
    mStorage(storage),
    mLog(logger),

    mTransactions(new map<BaseTransaction::Shared, TransactionState::SharedConst>()),
    mProcessingTimer(new as::steady_timer(mIOService)) {
}

/*!
 * Schedules awakening for the next transaction.
 * Doesn't blocks execution.
 */
void TransactionsScheduler::run() {

    if (mTransactions->empty()) {
        // If no transactions are present -
        // there is no reason to reschedule next interruption:
        // no transactions would be launched
        // (new transaction can only be launched
        // via network message or user command).
        return;
    }

    // WARN:
    // No one transaction should be launched directly from this method:
    // there is no any guarantee that the rest system components are ready.
    //
    // (initialisation order:
    // transactions manager may be initialised before other system components)
    adjustAwakeningToNextTransaction();
}

void TransactionsScheduler::scheduleTransaction(
    BaseTransaction::Shared transaction) {

    (*mTransactions)[transaction] =
        TransactionState::awakeAsFastAsPossible();

    adjustAwakeningToNextTransaction();
}

void TransactionsScheduler::postponeTransaction(
    BaseTransaction::Shared transaction,
    uint16_t millisecondsDelay) {

    (*mTransactions)[transaction] =
        TransactionState::awakeAfterMilliseconds(millisecondsDelay);

    adjustAwakeningToNextTransaction();
}

void TransactionsScheduler::killTransaction(
    const TransactionUUID &transactionUUID) {

    for (const auto &transactionAndState : *mTransactions) {
        if (transactionAndState.first->UUID() == transactionUUID) {
            forgetTransaction(transactionAndState.first);
        }
    }
}

void TransactionsScheduler::tryAttachMessageToTransaction(
    Message::Shared message) {

    if (message->isTransactionMessage()) {

        for (auto const &transactionAndState : *mTransactions) {

            if (static_pointer_cast<TransactionMessage>(message)->transactionUUID() != transactionAndState.first->UUID()) {
                    continue;
            }

            for (auto const &messageType : transactionAndState.second->acceptedMessagesTypes()) {
                if (messageType != message->typeID()) {
                    continue;
                }

                transactionAndState.first->pushContext(message);
                launchTransaction(transactionAndState.first);
                return;
            }
        }

    } else {
        throw ValueError(
            "TransactionsScheduler::handleMessage: "
                "invalid/unexpected message/response received");
    }

}

void TransactionsScheduler::launchTransaction(
    BaseTransaction::Shared transaction) {

    try {
        // Even if transaction will raise an exception -
        // it must not be thrown up,
        // to not to break transactions processing flow.

        auto result = transaction->run();
        if (result.get() == nullptr) {
            throw ValueError(
                "TransactionsScheduler::launchTransaction: "
                "transaction->run() returned nullptr result.");
        }

        handleTransactionResult(
            transaction,
            result
        );

    } catch (exception &e) {
        mLog->logException(
            "TransactionScheduler::launchTransaction:",
            e
        );
        forgetTransaction(transaction);
        // todo: add production log here. (this one is unusable)
        /*auto errors = mLog->error("TransactionsScheduler");
        errors << "Transaction interrupted with exception: "
               << "transaction type: " << transaction->transactionType()
               << e.what();*/
    }
}


void TransactionsScheduler::handleTransactionResult(
    BaseTransaction::Shared transaction,
    TransactionResult::SharedConst result) {

    switch (result->resultType()) {
        case TransactionResult::ResultType::CommandResultType: {
            processCommandResult(
                transaction,
                result->commandResult()
            );
            break;
        }

        case TransactionResult::ResultType::MessageResultType: {
            processMessageResult(
                transaction,
                result->messageResult()
            );
            break;
        }

        case TransactionResult::ResultType::TransactionStateType: {
            processTransactionState(
                transaction,
                result->state()
            );
            break;
        }
    }

}

void TransactionsScheduler::processCommandResult(
    BaseTransaction::Shared transaction,
    CommandResult::SharedConst result) {

    commandResultIsReadySignal(result);
    forgetTransaction(transaction);
}

void TransactionsScheduler::processMessageResult(
    BaseTransaction::Shared transaction,
    MessageResult::SharedConst result) {

    // todo: add writing to remote transactions results log.
    forgetTransaction(transaction);
}

void TransactionsScheduler::processTransactionState(
    BaseTransaction::Shared transaction,
    TransactionState::SharedConst state) {

    if (state->mustBeRescheduled()){
        if (state->needSerialize())
            serializeTransaction(transaction);

        // From the C++ reference:
        //
        // std::map::insert:
        // ... Because element keys in a map are unique,
        // the insertion operation checks whether each inserted element has a key
        // equivalent to the one of an element already in the container, and if so,
        // the element is NOT INSERTED, ...
        //
        // So the [] operator must be used
        (*mTransactions)[transaction] = state;

        if (transaction->transactionType() == BaseTransaction::TransactionType::AcceptRoutingTablesTransactionType) {
            cout << "FromInitiatorToContractorRoutingTablesAcceptTransaction process state" << endl;
            cout << "Awakening timestamp in micros -> " << (*mTransactions)[transaction]->awakeningTimestamp() << endl;
            cout << "Waiting for such messages type as" << endl;
            for (const auto &i : (*mTransactions)[transaction]->acceptedMessagesTypes()) {
                cout << "-> " << i << endl;
            }
        }

    } else {
        forgetTransaction(transaction);
    }
}

void TransactionsScheduler::serializeTransaction(
    BaseTransaction::Shared transaction) {

    auto transactionBytesAndCount = transaction->serializeToBytes();
    if (!mStorage->isExist(storage::uuids::uuid(transaction->UUID()))) {
        mStorage->write(
            storage::uuids::uuid(transaction->UUID()),
            transactionBytesAndCount.first.get(),
            transactionBytesAndCount.second
        );

    } else {
        mStorage->rewrite(
            storage::uuids::uuid(transaction->UUID()),
            transactionBytesAndCount.first.get(),
            transactionBytesAndCount.second
        );
    }
}

void TransactionsScheduler::forgetTransaction(
    BaseTransaction::Shared transaction) {

    try {
        mStorage->erase(
            storage::uuids::uuid(transaction->UUID())
        );

    } catch (IndexError &) {}

    mTransactions->erase(transaction);
}

void TransactionsScheduler::adjustAwakeningToNextTransaction() {

    try {
        asyncWaitUntil(
            transactionWithMinimalAwakeningTimestamp().second);

    } catch (NotFoundError &e) {}
}

/*!
 *
 * Throws NotFoundError in case if no transactions are delayed.
 */
pair<BaseTransaction::Shared, GEOEpochTimestamp> TransactionsScheduler::transactionWithMinimalAwakeningTimestamp() const {

    if (mTransactions->empty()) {
        throw NotFoundError(
            "TransactionsScheduler::transactionWithMinimalAwakeningTimestamp: "
                "There are no any delayed transactions.");
    }

    auto nextTransactionAndState = mTransactions->cbegin();
    if (mTransactions->size() > 1) {
        for (auto it=(mTransactions->cbegin()++); it != mTransactions->cend(); ++it){
            if (it->second == nullptr) {
                // Transaction has no state, and, as a result, doesn't have timeout set.
                // Therefore, it can't be considered for awakening by the timeout.
                continue;
            }

            if (it->second->awakeningTimestamp() < nextTransactionAndState->second->awakeningTimestamp()){
                nextTransactionAndState = it;
            }
        }
    }

    if (nextTransactionAndState->second->mustBeRescheduled()){
        return make_pair(
            nextTransactionAndState->first,
            nextTransactionAndState->second->awakeningTimestamp());
    }

    throw NotFoundError(
        "TransactionsScheduler::transactionWithMinimalAwakeningTimestamp: "
            "there are no any delayed transactions.");

//    if (mTransactions->size() == 1) {
//        return make_pair(
//            mTransactions->cbegin()->first,
//            mTransactions->cbegin()->second->awakeningTimestamp()
//        );

//    } else {
//        GEOEpochTimestamp awakeningTimestamp = 0;
//        for (auto transactionAndState = (mTransactions->cbegin()++); transactionAndState != mTransactions->cend(); ++transactionAndState) {
//            if (transactionAndState->second == nullptr) {
//                continue;

//            }
//            awakeningTimestamp = transactionAndState->second->awakeningTimestamp();
//            break;
//        }

//        if (awakeningTimestamp == 0) {
//            throw NotFoundError(
//                "TransactionsScheduler::transactionWithMinimalAwakeningTimestamp: "
//                    "There is no acceptable awakening timestamp.");
//        }

//        for (auto transactionAndState = (mTransactions->cbegin()++); transactionAndState != mTransactions->cend(); ++transactionAndState) {
//            if (transactionAndState->second == nullptr) {
//                continue;
//            }

//            if (transactionAndState->second->awakeningTimestamp() < awakeningTimestamp) {
//                awakeningTimestamp = transactionAndState->second->awakeningTimestamp();
//            }

//            return make_pair(
//              transactionAndState->first,
//              awakeningTimestamp
//            );
//        }
//    }
//
//    throw NotFoundError(
//        "TransactionsScheduler::transactionWithMinimalAwakeningTimestamp: "
//            "There is no acceptable awakening timestamp.");
}

void TransactionsScheduler::asyncWaitUntil(
    GEOEpochTimestamp nextAwakeningTimestamp) {


    GEOEpochTimestamp microsecondsDelay = 0;
    GEOEpochTimestamp now = microsecondsSinceGEOEpoch(utc_now());
    if (nextAwakeningTimestamp > now) {
        // NOTE: "now" is used twice:
        //in comparison and in delay calculation.
        //
        // It is important to use THE SAME timestamp in comparison and subtraction,
        // so it must not be replaced with 2 calls to microsecondsSinceGEOEpoch(utc_now())
        microsecondsDelay = nextAwakeningTimestamp - now;
    }

    cout << microsecondsSinceGEOEpoch(utc_now()) << ": ASYNC WAIT \t\t" << nextAwakeningTimestamp << endl;



//    mProcessingTimer->cancel();


    mProcessingTimer->expires_from_now(
        chrono::microseconds(microsecondsDelay));

//    if (microsecondsDelay > 18000000) {
//        cout << "SLEEP FOR HARDCODED 10 SECONDS" << endl;
//        mProcessingTimer->expires_from_now(
//            pt::seconds(10)
//        );
//    } else {
//        mProcessingTimer->expires_from_now(
//            pt::microsec(microsecondsDelay)
//        );
//    }

//    mProcessingTimer->wait();
//    handleAwakening(0);

    mProcessingTimer->async_wait(
        boost::bind(
            &TransactionsScheduler::handleAwakening,
            this,
            as::placeholders::error));
}

void TransactionsScheduler::handleAwakening(
    const boost::system::error_code &error) {

    static auto errorsCount = 0;

    if (error && error == as::error::operation_aborted) {
        cout << microsecondsSinceGEOEpoch(utc_now()) << ": AWAKENED AT \t\t" << microsecondsSinceGEOEpoch(utc_now()) << " BUT OPERATION WAS CANCELLED" << endl;
        return;
    }

    if (error && error != as::error::operation_aborted) {

        auto errors = mLog->error("TransactionsScheduler::handleAwakening");
        if (errorsCount < 10) {
            errors << "Error occurred on planned awakening. Details: " << error.message().c_str()
                   << ". Next awakening would be scheduled for" << errorsCount << " seconds from now.";

            // Transactions processing must not be cancelled.
            // Next awakening would be delayed for 10 seconds
            // (for cases when OS runs out of memory, or similar).
            asyncWaitUntil(
                microsecondsSinceGEOEpoch(
                    utc_now() + pt::seconds(errorsCount)
                )
            );

        } else {
            errors << "Some error repeatedly occurs on awakening. "
                   << "It seems that it can't be recovered automatically."
                   << "Next awakening would not be planned.";

            throw RuntimeError(
                "TransactionsScheduler::handleAwakening: "
                    "Some error repeated too much times");
        }
    }

    try {
        cout << microsecondsSinceGEOEpoch(utc_now()) << ": AWAKENED AT \t\t" << microsecondsSinceGEOEpoch(utc_now()) << endl;

        auto transactionAndDelay = transactionWithMinimalAwakeningTimestamp();
        if (microsecondsSinceGEOEpoch(utc_now()) >= transactionAndDelay.second) {
            launchTransaction(transactionAndDelay.first);
            errorsCount = 0;
        }

        adjustAwakeningToNextTransaction();

    } catch (NotFoundError &) {
        // There are no transactions left.
        // Awakenings cycle reached the end.
        return;
    }
}

bool TransactionsScheduler::isTransactionScheduled(
    BaseTransaction::Shared transaction) {

    return mTransactions->count(transaction) != 0;
}

const map<BaseTransaction::Shared, TransactionState::SharedConst>* transactions(
    TransactionsScheduler *scheduler) {

    return scheduler->mTransactions.get();
}
