#include "PathsCollection.h"

PathsCollection::PathsCollection(
    const NodeUUID &sourceUUID,
    const NodeUUID &destinationUUID) :

    mSourceNode(sourceUUID),
    mDestinationNode(destinationUUID),
    mIsDirectPathPresent(false),
    mIsReturnDirectPath(false),
    mCurrentPath(0){}

void PathsCollection::add(
    Path &path) {

    if (path.sourceUUID() != mSourceNode || path.destinationUUID() != mDestinationNode) {
        throw ValueError("PathsCollection::add "
                                 "Added path differs from current collection");
    }
    if (path.containsIntermediateNodes()) {
        mPaths.push_back(path.intermediateUUIDs());
    } else {
        mIsDirectPathPresent = true;
    }
}

void PathsCollection::resetCurrentPath() {

    mCurrentPath = 0;
    mIsReturnDirectPath = false;
}

bool PathsCollection::hasNextPath() {

    return mCurrentPath < mPaths.size();
}

Path::Shared PathsCollection::nextPath() {

    if (mCurrentPath > mPaths.size()) {
        throw IndexError("PathsCollection::nextPath "
                                 "no paths are available");
    }

    if (mCurrentPath == 0) {
        if (mIsDirectPathPresent && !mIsReturnDirectPath) {
            mIsReturnDirectPath = true;
            return make_shared<Path>(
                mSourceNode,
                mDestinationNode);

        } else {
            mCurrentPath++;
            return make_shared<Path>(
                mSourceNode,
                mDestinationNode,
                mPaths.at(mCurrentPath - 1));
        }
    } else {
        mCurrentPath++;
        return make_shared<Path>(
            mSourceNode,
            mDestinationNode,
            mPaths.at(mCurrentPath - 1));
    }
}

size_t PathsCollection::count() const {

    if (mIsDirectPathPresent) {
        return mPaths.size() + 1;
    } else {
        return mPaths.size();
    }
}

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

/*PathsCollection::Iterator::Iterator(
    NodeUUID &sourceNode,
    NodeUUID &destinationNode,
    const vector<SharedIntermediateNodes> &paths) :

    mCurrentPathIndex(0),
    mSourceNode(sourceNode),
    mDestinationNode(destinationNode),
    mPaths(paths){
}

Path PathsCollection::Iterator::next() {

    if (mPaths.size() <= mCurrentPathIndex) {
        Path path(mSourceNode, mDestinationNode, mPaths[mCurrentPathIndex++]);
        return path;
    } else {
        throw IndexError(
            "PathsCollection::Iterator: "
                "no paths are available");
    }
}*/
