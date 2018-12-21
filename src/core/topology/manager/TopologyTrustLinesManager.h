#ifndef GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINESMANAGER_H
#define GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINESMANAGER_H

#include "TopologyTrustLineWithPtr.h"
#include "../../contractors/addresses/BaseAddress.h"
#include "../../common/time/TimeUtils.h"
#include "../../logger/Logger.h"

#include <set>
#include <unordered_map>

class TopologyTrustLinesManager {

public:
    typedef unordered_set<TopologyTrustLineWithPtr*> TrustLineWithPtrHashSet;

public:
    TopologyTrustLinesManager(
        const SerializedEquivalent equivalent,
        bool iAmGateway,
        Logger &logger);

    void addTrustLine(
        TopologyTrustLine::Shared trustLine);

    TrustLineWithPtrHashSet trustLinePtrsSet(
        ContractorID nodeID);

    void resetAllUsedAmounts();

    bool deleteLegacyTrustLines();

    size_t trustLinesCounts() const;

    // todo : this code used only for testing and should be deleted in future
    void printTrustLines() const;

    DateTime closestTimeEvent() const;

    void setPreventDeleting(
        bool preventDeleting);

    bool preventDeleting() const;

    void addUsedAmount(
        ContractorID sourceID,
        ContractorID targetID,
        const TrustLineAmount &amount);

    void makeFullyUsed(
        ContractorID sourceID,
        ContractorID targetID);

    void addGateway(
        ContractorID gateway);

    const set<ContractorID> gateways() const;

    void makeFullyUsedTLsFromGatewaysToAllNodesExceptOne(
        ContractorID exceptedNode);

    const TrustLineAmount& flowAmount(
        ContractorID source,
        ContractorID destination);

    ContractorID getID(
        BaseAddress::Shared address);

    BaseAddress::Shared getAddressByID(
        ContractorID nodeID) const;

public:
    static const ContractorID kCurrentNodeID = 0;

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
    unordered_map<ContractorID, TrustLineWithPtrHashSet*> msTrustLines;
    map<DateTime, TopologyTrustLineWithPtr*> mtTrustLines;
    vector<pair<BaseAddress::Shared, ContractorID>> mParticipantsAddresses;
    ContractorID mHigherFreeID;
    SerializedEquivalent mEquivalent;
    Logger &mLog;
    bool mPreventDeleting;
    set<ContractorID> mGateways;
    DateTime mLastTrustLineTimeAdding;
};

#endif //GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINESMANAGER_H
