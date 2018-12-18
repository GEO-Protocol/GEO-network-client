#ifndef GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINESMANAGER_H
#define GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINESMANAGER_H

#include "../../common/NodeUUID.h"
#include "TopologyTrustLineWithPtr.h"
#include "TopologyTrustLineWithPtrNew.h"
#include "../../contractors/addresses/BaseAddress.h"
#include "../../common/time/TimeUtils.h"
#include "../../logger/Logger.h"

#include <set>
#include <unordered_set>
#include <unordered_map>
#include <boost/functional/hash.hpp>

class TopologyTrustLinesManager {

public:
    typedef unordered_set<TopologyTrustLineWithPtr*> TrustLineWithPtrHashSet;
    typedef unordered_set<TopologyTrustLineWithPtrNew*> TrustLineWithPtrHashSetNew;

public:
    TopologyTrustLinesManager(
        const SerializedEquivalent equivalent,
        bool iAmGateway,
        NodeUUID &nodeUUID,
        Logger &logger);

    void addTrustLine(
        TopologyTrustLine::Shared trustLine);

    void addTrustLineNew(
        TopologyTrustLineNew::Shared trustLine);

    TrustLineWithPtrHashSet trustLinePtrsSet(
        const NodeUUID &nodeUUID);

    TrustLineWithPtrHashSetNew trustLinePtrsSetNew(
        ContractorID nodeID);

    void resetAllUsedAmounts();

    bool deleteLegacyTrustLines();

    bool deleteLegacyTrustLinesNew();

    size_t trustLinesCounts() const;

    // todo : this code used only for testing and should be deleted in future
    void printTrustLines() const;

    void printTrustLinesNew() const;

    DateTime closestTimeEvent() const;

    void setPreventDeleting(
        bool preventDeleting);

    bool preventDeleting() const;

    void addUsedAmount(
        const NodeUUID &sourceUUID,
        const NodeUUID &targetUUID,
        const TrustLineAmount &amount);

    void addUsedAmountNew(
        ContractorID sourceID,
        ContractorID targetID,
        const TrustLineAmount &amount);

    void makeFullyUsed(
        const NodeUUID &sourceUUID,
        const NodeUUID &targetUUID);

    void makeFullyUsedNew(
        ContractorID sourceID,
        ContractorID targetID);

    void addGateway(const NodeUUID &gateway);

    void addGatewayNew(
        ContractorID gateway);

    const set<NodeUUID> gateways() const;

    const set<ContractorID> gatewaysNew() const;

    void makeFullyUsedTLsFromGatewaysToAllNodesExceptOne(
        const NodeUUID &exceptedNode);

    void makeFullyUsedTLsFromGatewaysToAllNodesExceptOneNew(
        ContractorID exceptedNode);

    const TrustLineAmount& flowAmount(
        const NodeUUID& source,
        const NodeUUID& destination);

    const TrustLineAmount& flowAmountNew(
        ContractorID source,
        ContractorID destination);

    ContractorID getID(
        BaseAddress::Shared address);

    BaseAddress::Shared getAddressByID(
        ContractorID nodeID) const;

private:
    static const byte kResetTrustLinesHours = 0;
    static const byte kResetTrustLinesMinutes = 12;
    static const byte kResetTrustLinesSeconds = 0;

    static Duration& kResetTrustLinesDuration() {
        static auto duration = Duration(
            kResetTrustLinesHours,
            kResetTrustLinesMinutes,
            kResetTrustLinesSeconds);
        return duration;
    }

    static const byte kClearTrustLinesHours = 0;
    static const byte kClearTrustLinesMinutes = 30;
    static const byte kClearTrustLinesSeconds = 0;

    static Duration& kClearTrustLinesDuration() {
        static auto duration = Duration(
            kClearTrustLinesHours,
            kClearTrustLinesMinutes,
            kClearTrustLinesSeconds);
        return duration;
    }

private:
    LoggerStream info() const;
    LoggerStream debug() const;

    const string logHeader() const;

private:
    unordered_map<NodeUUID, TrustLineWithPtrHashSet*, boost::hash<boost::uuids::uuid>> msTrustLines;
    map<DateTime, TopologyTrustLineWithPtr*> mtTrustLines;
    unordered_map<ContractorID, TrustLineWithPtrHashSetNew*> msTrustLinesNew;
    map<DateTime, TopologyTrustLineWithPtrNew*> mtTrustLinesNew;
    vector<pair<BaseAddress::Shared, ContractorID>> mParticipantsAddresses;
    ContractorID mHigherFreeID;
    SerializedEquivalent mEquivalent;
    Logger &mLog;
    bool mPreventDeleting;
    set<NodeUUID> mGateways;
    set<ContractorID> mGatewaysNew;
    DateTime mLastTrustLineTimeAdding;
};

#endif //GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINESMANAGER_H
