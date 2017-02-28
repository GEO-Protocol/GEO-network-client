//
// Created by mc on 19.02.17.
//

#include "MaxFlowCalculationTrustLineManager.h"

void MaxFlowCalculationTrustLineManager::addTrustLine(MaxFlowCalculationTrustLine::Shared trustLine) {
    auto const &it = mvTrustLines.find(trustLine->sourceUUID());
    if (it == mvTrustLines.end()) {
        vector<MaxFlowCalculationTrustLine::Shared> newVect;
        newVect.push_back(trustLine);
        mvTrustLines.insert(make_pair(trustLine->sourceUUID(), newVect));
    } else {
        bool trustLineContains = false;
        for (auto &trLine : it->second) {
            if (trLine->sourceUUID() == trustLine->sourceUUID()
                && trLine->targetUUID() == trustLine->targetUUID()) {
                trustLineContains = true;
                break;
            }
        }
        if (!trustLineContains) {
            it->second.push_back(trustLine);
        }
    }
}

vector<MaxFlowCalculationTrustLine::Shared> MaxFlowCalculationTrustLineManager::sortedTrustLines(
    const NodeUUID &nodeUUID) {

    vector<MaxFlowCalculationTrustLine::Shared> result;
    auto const &nodeUUIDAndVector = mvTrustLines.find(nodeUUID);
    if (nodeUUIDAndVector == mvTrustLines.end()) {
        return result;
    }

    for (auto const &trustLine : nodeUUIDAndVector->second) {
        result.push_back(trustLine);
    }

    std::sort(result.begin(), result.end(), customLess);
    return result;
}

void MaxFlowCalculationTrustLineManager::resetAllUsedAmounts() {

    for (auto &nodeUUIDAndTrustLine : mvTrustLines) {
        for (auto &trustLine : nodeUUIDAndTrustLine.second) {
            trustLine->setUsedAmount(0);
        }
    }
}