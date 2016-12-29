#ifndef GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H
#define GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H

#include <map>
#include <vector>
#include <malloc.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "../../common/NodeUUID.h"
#include "../../io/trust_lines/TrustLinesStorage.h"
#include "../../db/Block.h"
#include "../../db/UUIDMapBlockStorage.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/ConflictError.h"
#include "../../common/exceptions/PreconditionFaultError.h"
#include "../TrustLine.h"

using namespace std;

namespace storage = db::uuid_map_block_storage;

typedef storage::byte byte;

class TrustLinesManagerTest;

class TrustLinesManager {
    friend class TrustLinesManagerTest;

public:
    static const size_t kTrustAmountPartSize = 32;
    static const size_t kBalancePartSize = 32;
    static const size_t kSignBytePartSize = 1;
    const size_t kBucketSize = kTrustAmountPartSize + kTrustAmountPartSize + kBalancePartSize + kSignBytePartSize;
    const trust_amount ZERO_CHECKED_INT256_VALUE = 0;
    const balance_value ZERO_INT256_VALUE = 0;
    map<NodeUUID, TrustLine *> mTrustLines;
    TrustLinesStorage *mTrustLinesStorage;

private:

    /**
     * Function serializeBytesFromTrustLineStruct(TrustLine *trustLine) <br>
     * Serialize TrustLine structure instace into bytes array.
     * @param trustLine - pointer to TrustLine structure instance.
     * @return pointer to bytes array with serialized TrustLine structure instance.
     */
    byte *serializeBytesFromTrustLineStruct(TrustLine *trustLine);

    /**
     * Function trustAmountDataToBytes(trust_amount amount) <br>
     * Serialize TrustLine instance's incoming_trust_amount and outgoing_trust_amount field-members into bytes vector. <br>
     * Invokes in serializeBytesFromTrustLineStruct(TrustLine *trustLine).
     * @param amount - trust line value, type of checked_uint256_t @see boost/multiprecision/cpp_int.hpp. <br>
     * trust_amount type defined in header file. @see TrustLine.h
     * @return vector of bytes size of 32 bytes.
     */
    vector<byte> trustAmountDataToBytes(const trust_amount &amount);

    /**
     * Function balanceToBytes(balance_value balance) <br>
     * Serialize TrustLine instance's balance field-member into bytes vector. <br>
     * Invokes in serializeBytesFromTrustLineStruct(TrustLine *trustLine).
     * @param balance - balance value, type of int256_t @see boost/multiprecision/cpp_int.hpp. <br>
     * balance_value type defined in header file. @see TrustLine.h
     * @return vector of bytes size of 33 bytes. Last byte in vector specified sign, others bytes are value.
     */
    vector<byte> balanceToBytes(const balance_value &balance);

    /**
     * Function deserializeTrustLineStructFromBytes(uint8_t *buffer, uuids::uuid contractorUUID) <br>
     * Deserialize bytes array into TrustLine structure instace.
     * @param buffer - uint8_t array with set of bytes.
     * @param contractorUUID - contractor UUID absent into buffer and set in instance manually.
     * Deserialized instace insert into map.
     */
    void deserializeTrustLineStructFromBytes(const byte *buffer, const NodeUUID &contractorUUID);

    /**
     * Function parseTrustAmountData(uint8_t *buffer) <br>
     * Deserialize bytes array into TrustLine instance's incoming_trust_amount and outgoing_trust_amount field-members. <br>
     * Invokes in deserializeTrustLineStructFromBytes(uint8_t *buffer, uuids::uuid contractorUUID).
     * @param buffer - uint8_t array with set of bytes.
     * @return trust_amount - incoming/outgoing trust line value, type of checked_uint256_t.
     */
    trust_amount parseTrustAmountData(const byte *buffer);

    /**
     * Function parseBalanceData(uint8_t *buffer) <br>
     * Deserialize bytes array into TrustLine instance's balance field-member. <br>
     * Invokes in deserializeTrustLineStructFromBytes(uint8_t *buffer, uuids::uuid contractorUUID).
     * @param buffer - uint8_t array with set of bytes.
     * @return balance_value - user's balance value, type of int256_t.
     */
    balance_value parseBalanceData(const byte *buffer);

    /**
     * Function saveTrustLine(TrustLine *trustLine) <br>
     * Save trust lines bytes in bin file, if storage manager returns success result - save trust line struct instance in ram (map). <br>
     * @param trustLine - pointer on trust line structure instance.
     */
    void saveTrustLine(TrustLine *trustLine);

    /**
     * Function removeTrustLine(TrustLine *trustLine) <br>
     * Remove trust lines bytes from bin file, if storage manager returns success result - remove trust line struct instance from ram (map). <br>
     * @param contractorUUID - contractor UUID.
     * @throw ConflictError - if trust line with such contracotr UUID does not exist
     */
    void removeTrustLine(const NodeUUID &contractorUUID);

    /**
     * Function isTrustLineExist(const uuids::uuid contractorUUID) <br>
     * Check if trust line with such contractor exist.
     * @param contractorUUID - contractor UUID.
     * @return true if pointer to TrustLine struct instance by such contractor UUID key exist in map, else return false.
     */
    bool isTrustLineExist(const NodeUUID &contractorUUID);

    void getTrustLinesFromStorage();

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
     * @throw ConflictError - outgoing trust line to such contractor already exist.
     * @throw ValueError - outgoing trust line amount less or equals to zero.
     */
    void open(const NodeUUID &contractorUUID, const trust_amount &amount);

    /**
     * Function close(const uuids::uuid contractorUUID) <br>
     * Close trust line from user to contractor. <br>
     * @param contractorUUID - contractor UUID.
     * @throw ConflictError - ttrust line to such contractor does not exist.
     * @throw ValueError - outgoing trust line amount less or equals to zero.
     * @throw PreconditionError - contractor already used part of amount.
     */
    void close(const NodeUUID &contractorUUID);

    /**
     * Function accept(const uuids::uuid contractorUUID, const trust_amount amount) <br>
     * Accept trust line from contractor to user. <br>
     * @param contractorUUID - contractor UUID.
     * @param amount - trust line value(size, volume who knows how it's write) from contractor.
     * @throw ConflictError - incoming trust line to such contractor already exist.
     * @throw ValueError - incoming trust line amount less or equals to zero.
     */
    void accept(const NodeUUID &contractorUUID, const trust_amount &amount);

    /**
     * Function reject(const uuids::uuid contractorUUID) <br>
     * Reject trust line from contractor to user. <br>
     * @param contractorUUID - contractor UUID.
     * @throw ConflictError - trust line to such contractor does not exist.
     * @throw ValueError - incoming trust line amount less or equals to zero.
     * @throw PreconditionError - user already used part of amount.
     */
    void reject(const NodeUUID &contractorUUID);

    /**
     * Function getTrustLineByContractorUUID(const uuids::uuid contractorUUID) <br>
     * Seek trust line by contractor UUID. <br>
     * @param contractorUUID - contractor UUID.
     * @return TrustLine* - pointer on Trust Line structure instance.
     */
    TrustLine *getTrustLineByContractorUUID(const NodeUUID &contractorUUID);
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H
