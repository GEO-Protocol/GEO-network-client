#include "GetTopologyAndBalancesMessage.h"

void GetTopologyAndBalancesMessage::deserializeFromBytes(BytesShared buffer) {
    Message::deserializeFromBytes(buffer);
}

GetTopologyAndBalancesMessage::GetTopologyAndBalancesMessage(const TrustLineAmount maxFlow, const byte max_depth) {
    mMaxFlow = maxFlow;
    mMax_depth = max_depth;
//    mPath.push_back(Settings.NodeUUID());

}

GetTopologyAndBalancesMessage::GetTopologyAndBalancesMessage(BytesShared buffer) {

}

void GetTopologyAndBalancesMessage::serializedToBytes() {

}
