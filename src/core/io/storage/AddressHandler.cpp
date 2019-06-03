#include "AddressHandler.h"

AddressHandler::AddressHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger &logger) :

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger)
{
    sqlite3_stmt *stmt;
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   "(type INTEGER NOT NULL, "
                   "contractor_id INTEGER NOT NULL, "
                   "address_size INTEGER NOT NULL, "
                   "address BLOB NOT NULL, "
                   "FOREIGN KEY(contractor_id) REFERENCES contractors(id) ON DELETE CASCADE ON UPDATE CASCADE);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("AddressHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("AddressHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_contractor_id on " + mTableName + "(contractor_id);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("AddressHandler::creating index for ContractorID: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("AddressHandler::creating index for ContractorID: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void AddressHandler::saveAddress(
    ContractorID contractorID,
    BaseAddress::Shared address)
{
    string query = "INSERT INTO " + mTableName +
                   "(type, contractor_id, address_size, address) "
                   "VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("AddressHandler::saveAddress: "
                          "Bad query; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_bind_int(stmt, 1, (int)address->typeID());
    if (rc != SQLITE_OK) {
        throw IOError("AddressHandler::saveAddress: "
                          "Bad binding of address type; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, contractorID);
    if (rc != SQLITE_OK) {
        throw IOError("AddressHandler::saveAddress: "
                          "Bad binding of Contractor ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 3, (int)address->serializedSize());
    if (rc != SQLITE_OK) {
        throw IOError("AddressHandler::saveAddress: "
                          "Bad binding of address size; sqlite error: " + to_string(rc));
    }
    auto serializedAddress = address->serializeToBytes();
    rc = sqlite3_bind_blob(stmt, 4, serializedAddress.get(), (int)address->serializedSize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::saveTrustLine: "
                          "Bad binding of address; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting is completed successfully";
#endif
    } else {
        throw IOError("AddressHandler::saveAddress: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

vector<BaseAddress::Shared> AddressHandler::contractorAddresses(
    ContractorID contractorID)
{
    vector<BaseAddress::Shared> result;
    sqlite3_stmt *stmt;
    string query = "SELECT type, address_size, address FROM "
                   + mTableName + " WHERE contractor_id = ?";
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("AddressHandler::contractorAddresses: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, contractorID);
    if (rc != SQLITE_OK) {
        throw IOError("AddressHandler::contractorAddresses: "
                          "Bad binding of Contractor ID; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        auto addressType = (BaseAddress::AddressType)sqlite3_column_int(stmt, 0);
        auto addressSize = (size_t)sqlite3_column_int(stmt, 1);
        auto addressBytes = (byte*)sqlite3_column_blob(stmt, 2);
        try {
            switch (addressType) {
                case BaseAddress::IPv4_IncludingPort: {
                    result.push_back(
                        make_shared<IPv4WithPortAddress>(
                            addressBytes));
                    break;
                }
                case BaseAddress::GNS: {
                    result.push_back(
                        make_shared<GNSAddress>(
                            addressBytes));
                    break;
                }
                default: {
                    throw ValueError("AddressHandler::contractorAddresses: "
                                             "Invalid address type: " + to_string(addressType));
                }
            }
        } catch (std::exception &e) {
            throw Exception("AddressHandler::contractorAddresses. "
                            "Unable to create address instance from DB of type " + to_string(addressType)
                            + " Details: " + e.what());
        } catch (...) {
            throw Exception("AddressHandler::contractorAddresses. "
                                "Unable to create address instance from DB of type " + to_string(addressType));
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

void AddressHandler::removeAddresses(
    ContractorID contractorID)
{
    string query = "DELETE FROM " + mTableName + " WHERE contractor_id = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("AddressHandler::removeAddresses: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, contractorID);
    if (rc != SQLITE_OK) {
        throw IOError("AddressHandler::removeAddresses: "
                          "Bad binding of ContractorID; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "deleting is completed successfully";
#endif
    } else {
        throw IOError("AddressHandler::removeAddresses: "
                          "Run query; sqlite error: " + to_string(rc));
    }
    if (sqlite3_changes(mDataBase) == 0) {
        throw ValueError("No data were deleted");
    }
}

LoggerStream AddressHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream AddressHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string AddressHandler::logHeader() const
{
    stringstream s;
    s << "[AddressHandler]";
    return s.str();
}