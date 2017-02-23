//#include "PathsCollection.h"


//PathsCollection::PathsCollection() :
//    mIsDirectPathPresent(false) {

//}

//void PathsCollection::add(
//    Path &path) {

//    if (path.containsIntermediateNodes()) {
//        mIsDirectPathPresent = true;
//    }

//    mPaths.push_back(intermediateNodes);
//}

//PathsCollection::Path::Path(
//    const NodeUUID &sourceNode,
//    const NodeUUID &destinationNode,
//    const PathsCollection::SharedIntermediateNodes intermediateNodes) :

//    mSourceNode(sourceNode),
//    mDestinationNode(destinationNode),
//    mIntermediateNodes(intermediateNodes) {
//}

///*!
// *
// * Throws bad_alloc
// */
//BytesShared PathsCollection::Path::serialize() const {

//    const auto sourceNodeSize = NodeUUID::kBytesSize;
//    const auto destinationNodeSize = NodeUUID::kBytesSize;
//    const auto intermediateNodesCount = mIntermediateNodes->size();
//    const auto intermediateNodesSize = intermediateNodesCount * NodeUUID::kBytesSize;

//    const auto bufferSize =
//        + sourceNodeSize
//        + intermediateNodesSize
//        + destinationNodeSize;

//    auto buffer = tryMalloc(bufferSize);

//    auto sourceNodeOffset = buffer.get();
//    auto intermediateNodesOffset = buffer.get() + sourceNodeSize;
//    auto destinationNodeOffset = intermediateNodesOffset + sourceNodeSize;

//    memcpy(sourceNodeOffset, mSourceNode.data, sourceNodeSize);
//    memcpy(intermediateNodesOffset, mIntermediateNodes->data(), intermediateNodesSize);
//    memcpy(destinationNodeOffset, mSourceNode.data, destinationNodeSize);

//    return buffer;
//}

//PathsCollection::Iterator::Iterator(
//    NodeUUID &sourceNode,
//    NodeUUID &destinationNode,
//    const vector<SharedIntermediateNodes> &paths) :

//    mCurrentPathIndex(0),
//    mSourceNode(sourceNode),
//    mDestinationNode(destinationNode),
//    mPaths(paths){
//}

//PathsCollection::Path PathsCollection::Iterator::next() {

//    if (mPaths.size() <= mCurrentPathIndex) {
//        Path path(mSourceNode, mDestinationNode, mPaths[mCurrentPathIndex++]);
//        return path;

//    } else {
//        throw IndexError(
//            "PathsCollection::Iterator: "
//                "no paths are available");
//    }
//}
