#include "AmountReservationsHandler.h"


/*!
 * Creates and returns new AmountReservation, assigned to the trust line with the "trustLineContractor".
 * Doesn't checks if amount is available on the trust line.
 *
 * @param trustLineContractor - uuid of the trust line contaractor node.
 * @param transactionUUID - uuid of the transaction, which reserves the amount.
 * @param amount - amount that should be reserved.
 *
 *
 * Throws ValueError in case if "amount" == 0;
 * Throws MemoryError;
 */
AmountReservation::ConstShared AmountReservationsHandler::reserve(
    const NodeUUID &trustLineContractor,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount) {

    if (amount == 0) {
        throw ValueError(
            "AmountReservationsHandler::reserve: 'amount' can't be 0");
    }

    try {
        auto reservation = AmountReservation::ConstShared(
            new AmountReservation(transactionUUID, amount));

        auto iterator = mReservations.find(trustLineContractor);
        if (iterator != mReservations.end()) {
            // Trying to insert "reservation" into the reservations container.
            auto reservations = (*iterator).second.get();
            reservations->push_back(reservation);
            return reservation;

        } else {
            // Reservations container is absent and should be created.
            unique_ptr<vector<AmountReservation::ConstShared>> reservationsContainer(
                new vector<AmountReservation::ConstShared>(1));

            reservationsContainer->push_back(reservation);
            mReservations.insert(
                make_pair(
                    trustLineContractor, move(
                        reservationsContainer)));
            return reservation;
        }

    } catch (bad_alloc &) {
        throw MemoryError(
            "AmountReservationsHandler::reserve: bad alloc.");
    }
}

/*!
 * Updates and returns existing "reservation", assigned to the trust line with the "trustLineContractor".
 *
 * @param trustLineContractor - uuid of the trust line contaractor node.
 * @param reservation - reservation that should be updated.
 * @param newAmount - amount that should be reserved.
 *
 *
 * Throws ValueError in case if "amount" == 0;
 * Throws NotFoundError in case if "reservation" doesn'is not present in already created reservations.
 * Throws MemoryError;
 */
AmountReservation::ConstShared AmountReservationsHandler::updateReservation(
    const NodeUUID &trustLineContractor,
    const AmountReservation::ConstShared reservation,
    const TrustLineAmount &newAmount) {

    if (newAmount == 0) {
        throw ValueError(
            "AmountReservationsHandler::updateReservation: 'newAmount' can't be 0");
    }

    try {
        auto newReservation = AmountReservation::ConstShared(
            new AmountReservation(
                reservation->transactionUUID(),
                newAmount));

        auto iterator = mReservations.find(trustLineContractor);
        if (iterator == mReservations.end()) {
            throw NotFoundError(
                "AmountReservationsHandler::updateReservation: "
                    "reservation with exact contractor UUID was not found.");
        }

        auto reservations = (*iterator).second.get();
        for (auto it=reservations->begin(); it!=reservations->end(); ++it){
            if (*it == reservation) {
                *it = newReservation;
                return newReservation;
            }
        }

        throw NotFoundError(
            "AmountReservationsHandler::updateReservation: "
                "reservation with exact amount was not found.");

    } catch (bad_alloc &) {
        throw MemoryError(
            "AmountReservationsHandler::updateReservation: bad alloc.");
    }
}

void AmountReservationsHandler::free(
    const NodeUUID &trustLineContractor,
    const AmountReservation::ConstShared reservation) {

    try {
        auto iterator = mReservations.find(trustLineContractor);
        if (iterator == mReservations.end()) {
            throw NotFoundError(
                "AmountReservationsHandler::free: "
                    "reservation with exact contractor UUID was not found.");
        }

        auto reservations = (*iterator).second.get();
        for (auto it=reservations->cbegin(); it!=reservations->cend(); ++it){
            if (*it == reservation) {
                reservations->erase(it);
                return;
            }
        }

        throw NotFoundError(
            "AmountReservationsHandler::free: "
                "reservation with exact amount was not found.");

    } catch (bad_alloc &) {
        throw MemoryError(
            "AmountReservationsHandler::free: bad alloc.");
    }
}

/*!
 * Returns total amount, that was reserved on the trust line with the contractor == "trustLineContractor".
 * Optionally, filters locks by transactionUUID (see reservations(...) for details).
 * In case if no amount was reserved - returns 0;
 */
ConstSharedTrustLineAmount AmountReservationsHandler::totalReserved(
    const NodeUUID &trustLineContractor,
    const TransactionUUID *transactionUUID) const {

    SharedTrustLineAmount amount(new TrustLineAmount(0));

    auto reservationsVector = reservations(trustLineContractor, transactionUUID);
    for (auto lock : reservationsVector){
        (*amount) += (*lock).amount();
    }

    return amount;
}

/*!
 * Returns all reservations, that are assigned to the trust line with the contractor "trustLineContractor".
 * In case if no such "trustLineContractor" was found, or no reservations are available after filtering -
 * returns empty vector.
 *
 * If "transactionUUID" is not "nullptr" - additionally filters reservations by the transaction.
 *
 *
 * Throws MemoryError;
 */
vector<AmountReservation::ConstShared> AmountReservationsHandler::reservations(
    const NodeUUID &trustLineContractor,
    const TransactionUUID *transactionUUID) const {

    try {
        auto iterator = mReservations.find(trustLineContractor);
        if (iterator == mReservations.end()){
            return vector<AmountReservation::ConstShared>();
        }

        if (transactionUUID == nullptr) {
            // No additional filtering is needed.
            return (*(*iterator).second);

        } else {
            // Additional filtering by the "transactionUUID" should be applied.
            auto reservations = (*iterator).second.get();

            vector<AmountReservation::ConstShared> filteredBlocksContainer;
            filteredBlocksContainer.reserve(reservations->size());

            for (auto it=reservations->cbegin(); it!=reservations->cend(); ++it) {
                if ((*it)->transactionUUID() == *transactionUUID){
                    filteredBlocksContainer.push_back(*it);
                }
            }

            filteredBlocksContainer.shrink_to_fit();
            return filteredBlocksContainer;
        }

    } catch (bad_alloc &) {
        throw MemoryError(
            "AmountReservationsHandler::reservations: bad alloc.");
    }
}
