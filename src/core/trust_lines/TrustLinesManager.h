#ifndef TRUSTLINEMANAGER_TRUSTLINESMANAGER_H
#define TRUSTLINEMANAGER_TRUSTLINESMANAGER_H

#include <map>
#include <vector>
#include "TrustLine.h"

using namespace std;

class TrustLinesManager {

private:
    const size_t BUCKET_SIZE = 97;
    const size_t TRUST_AMOUNT_PART_SIZE = 32;
    const size_t BALANCE_PART_SIZE = 32;
    const size_t SIGN_BYTE_PART_SIZE = 1;
    const trust_amount ZERO_CHECKED_INT256_VALUE = 0;
    const balance_value ZERO_INT256_VALUE = 0;
    map<uuids::uuid, TrustLine *> mTrustLines;

private:

    /**
     * Function serializeBytesFromTrustLineStruct(TrustLine *trustLine) <br>
     * Serialize TrustLine structure instace into bytes array.
     * @param trustLine - pointer to TrustLine structure instance.
     * @return pointer to bytes array with serialized TrustLine structure instance.
     */
    uint8_t *serializeBytesFromTrustLineStruct(TrustLine *trustLine);

    /**
     * Function trustAmountDataToBytes(trust_amount amount) <br>
     * Serialize TrustLine instance's incoming_trust_amount and outgoing_trust_amount field-members into bytes vector. <br>
     * Invokes in serializeBytesFromTrustLineStruct(TrustLine *trustLine).
     * @param amount - trust line value, type of checked_uint256_t @see boost/multiprecision/cpp_int.hpp. <br>
     * trust_amount type defined in header file. @see TrustLine.h
     * @return vector of bytes size of 32 bytes.
     */
    vector<uint8_t> trustAmountDataToBytes(trust_amount amount);

    /**
     * Function balanceToBytes(balance_value balance) <br>
     * Serialize TrustLine instance's balance field-member into bytes vector. <br>
     * Invokes in serializeBytesFromTrustLineStruct(TrustLine *trustLine).
     * @param balance - balance value, type of int256_t @see boost/multiprecision/cpp_int.hpp. <br>
     * balance_value type defined in header file. @see TrustLine.h
     * @return vector of bytes size of 33 bytes. Last byte in vector specified sign, others bytes are value.
     */
    vector<uint8_t> balanceToBytes(balance_value balance);

    /**
     * Function deserializeTrustLineStructFromBytes(uint8_t *buffer, uuids::uuid contractorUUID) <br>
     * Deserialize bytes array into TrustLine structure instace.
     * @param buffer - uint8_t array with set of bytes.
     * @param contractorUUID - contractor UUID absent into buffer and set in instance manually.
     * Deserialized instace insert into map.
     */
    void deserializeTrustLineStructFromBytes(uint8_t *buffer, uuids::uuid contractorUUID);

    /**
     * Function parseTrustAmountData(uint8_t *buffer) <br>
     * Deserialize bytes array into TrustLine instance's incoming_trust_amount and outgoing_trust_amount field-members. <br>
     * Invokes in deserializeTrustLineStructFromBytes(uint8_t *buffer, uuids::uuid contractorUUID).
     * @param buffer - uint8_t array with set of bytes.
     * @return trust_amount - incoming/outgoing trust line value, type of checked_uint256_t.
     */
    trust_amount parseTrustAmountData(uint8_t *buffer);

    /**
     * Function parseBalanceData(uint8_t *buffer) <br>
     * Deserialize bytes array into TrustLine instance's balance field-member. <br>
     * Invokes in deserializeTrustLineStructFromBytes(uint8_t *buffer, uuids::uuid contractorUUID).
     * @param buffer - uint8_t array with set of bytes.
     * @return balance_value - user's balance value, type of int256_t.
     */
    balance_value parseBalanceData(uint8_t *buffer);

    /**
     * Function saveTrustLine(TrustLine *trustLine) <br>
     * Save trust lines bytes in bin file, if storage manager returns success result - save trust line struct instance in ram (map). <br>
     * @param trustLine - pointer on trust line structure instance.
     * @return true if trust line is successfully saved, false if something wrong.
     */
    bool saveTrustLine(TrustLine *trustLine);

    /**
     * Function removeTrustLine(TrustLine *trustLine) <br>
     * Remove trust lines bytes from bin file, if storage manager returns success result - remove trust line struct instance from ram (map). <br>
     * @param contractorUUID - contractor UUID.
     * @return true if trust line is successfully saved, false if something wrong.
     */
    bool removeTrustLine(const uuids::uuid contractorUUID);

    /**
     * Function isTrustLineExist(const uuids::uuid contractorUUID) <br>
     * Check if trust line with such contractor exist.
     * @param contractorUUID - contractor UUID.
     * @return true if pointer to TrustLine struct instance by such contractor UUID key exist in map, else return false.
     */
    bool isTrustLineExist(const uuids::uuid contractorUUID);

public:

    /**
     * Constructor
     */
    TrustLinesManager();

    /**
     * Destructor
     */
    ~TrustLinesManager();

    /**
     * Function open(const uuids::uuid contractorUUID, const trust_amount amount) <br>
     * Open trust line from user to contractor. <br>
     * @param contractorUUID - contractor UUID.
     * @param amount - trust line value (size, volume who knows how it's write) to contractor.
     * @return true if trust line successfully opened, false if something wrong.
     */
    bool open(const uuids::uuid contractorUUID, const trust_amount amount);

    /**
     * Function close(const uuids::uuid contractorUUID) <br>
     * Close trust line from user to contractor. <br>
     * @param contractorUUID - contractor UUID.
     * @return 1 if trust line is successfully closed, -1 if trust line can not be closed, <br>
     * -2 if error ocurred while save/remove in/from storage operation performed, 0 if trust line with such contractor does not exist.
     */
    int close(const uuids::uuid contractorUUID);

    /**
     * Function accept(const uuids::uuid contractorUUID, const trust_amount amount) <br>
     * Accept trust line from contractor to user. <br>
     * @param contractorUUID - contractor UUID.
     * @param amount - trust line value(size, volume who knows how it's write) from contractor.
     * @return true if trust line successfully opened, false if something wrong.
     */
    bool accept(const uuids::uuid contractorUUID, const trust_amount amount);

    /**
     * Function reject(const uuids::uuid contractorUUID) <br>
     * Reject trust line from contractor to user. <br>
     * @param contractorUUID - contractor UUID.
     * @return 1 if trust line is successfully rejected, -1 if trust line can not be rejected, <br>
     * -2 if error ocurred while save/remove in/from storage operation performed, 0 if trust line with such contractor does not exist.
     */
    int reject(const uuids::uuid contractorUUID);
};


#endif //TRUSTLINEMANAGER_TRUSTLINESMANAGER_H
