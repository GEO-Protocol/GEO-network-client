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
 * Throws bad_alloc;
 */
AmountReservation::ConstShared AmountReservationsHandler::reserve(
    const NodeUUID &trustLineContractor,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount,
    const AmountReservation::ReservationDirection direction)
{
    if (0 == amount)
        throw ValueError(
            "AmountReservationsHandler::reserve: amount can't be 0.");


    const auto kReservation = make_shared<AmountReservation>(
        transactionUUID,
        amount,
        direction);

    auto iterator = mReservations.find(trustLineContractor);
    if (iterator != mReservations.end()) {
        // Reservations container is present.
        // Newly created reservation must be pushed into it.
        auto reservations = (*iterator).second.get();
        reservations->push_back(kReservation);

    } else {
        // Reservations container is absent and should be created.
        auto reservationsContainer = make_unique<vector<AmountReservation::ConstShared>>();
        reservationsContainer->push_back(kReservation);
        mReservations.insert(
            make_pair(
                trustLineContractor,
                move(reservationsContainer)));
    }

    return kReservation;
}

/*!
 * Updates and returns existing "reservation", assigned to the trust line with the "trustLineContractor".
 *
 * @param trustLineContractor - uuid of the trust line contractor node.
 * @param reservation - reservation that should be updated.
 * @param newAmount - amount that should be reserved.
 *
 *
 * Throws ValueError in case if "amount" == 0;
 * Throws NotFoundError in case if "reservation" is not present in already created reservations.
 * Throws MemoryError;
 */
AmountReservation::ConstShared AmountReservationsHandler::updateReservation(
    const NodeUUID &trustLineContractor,
    const AmountReservation::ConstShared reservation,
    const TrustLineAmount &newAmount)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(reservation != nullptr);
#endif

    if (newAmount == TrustLineAmount(0))
        throw ValueError("AmountReservationsHandler::updateReservation: 'newAmount' == 0.");

    if (mReservations.count(trustLineContractor) == 0) {
        throw NotFoundError(
            "AmountReservationsHandler::updateReservation: "
                "reservation with exact contractor UUID was not found.");
    }


    const auto kNewReservation = make_shared<const AmountReservation>(
        reservation->transactionUUID(),
        newAmount,
        reservation->direction());


    auto reservations = mReservations.find(trustLineContractor)->second.get();
    for (auto it=reservations->begin(); it!=reservations->end(); ++it){
        if (*it == reservation) {
            *it = kNewReservation;
            return kNewReservation;
        }
    }

    throw NotFoundError(
        "AmountReservationsHandler::updateReservation: "
            "reservation with exact amount was not found.");
}

void AmountReservationsHandler::free(
    const NodeUUID &trustLineContractor,
    const AmountReservation::ConstShared reservation)
{
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
                if (reservations->empty()) {
                    mReservations.erase(iterator);
                }
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
    const AmountReservation::ReservationDirection direction,
    const TransactionUUID *transactionUUID) const
{
    SharedTrustLineAmount amount(new TrustLineAmount(0));

    auto reservationsVector = reservations(trustLineContractor, transactionUUID);
    for (auto lock : reservationsVector){
        if (lock->direction() == direction)
            (*amount) += (*lock).amount();
    }

    return amount;
}

/*!
 * Returns total amount, that was reserved on the trust line with the contractor == "trustLineContractor".
 * In case if no amount was reserved - returns 0;
 */
ConstSharedTrustLineAmount AmountReservationsHandler::totalReservedOnTrustLine(
    const NodeUUID &trustLineContractor) const
{
    SharedTrustLineAmount amount(new TrustLineAmount(0));

    auto reservationsVector = reservations(trustLineContractor);
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
    const TransactionUUID *transactionUUID) const
{
    try {
        auto iterator = mReservations.find(trustLineContractor);
        if (iterator == mReservations.end()){
            return vector<AmountReservation::ConstShared>();
        }

        if (transactionUUID == nullptr) {
            // No additional filtering is needed.
            return *(iterator->second);

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

bool AmountReservationsHandler::isReservationsPresent(
    const NodeUUID &trustLineContractor) const
{
    return mReservations.find(trustLineContractor) != mReservations.end();
}

const vector<AmountReservation::ConstShared> AmountReservationsHandler::contractorReservations(
    const NodeUUID &trustLineContractor,
    const AmountReservation::ReservationDirection direction) const
{
    vector<AmountReservation::ConstShared> result;
    auto reservationsVector = reservations(
        trustLineContractor,
        nullptr);
    for (auto lock : reservationsVector){
        if (lock->direction() == direction)
            result.push_back(lock);
    }
    return result;
}
