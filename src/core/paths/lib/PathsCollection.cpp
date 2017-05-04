#include "PathsCollection.h"

PathsCollection::PathsCollection(
    const NodeUUID &sourceUUID,
    const NodeUUID &destinationUUID) :

    mSourceNode(sourceUUID),
    mDestinationNode(destinationUUID),
    mCurrentPath(0)
{}

void PathsCollection::add(
    Path &path)
{
    if (path.sourceUUID() != mSourceNode || path.destinationUUID() != mDestinationNode) {
        throw ValueError("PathsCollection::add "
                             "Added path differs from current collection");
    }
    mPaths.push_back(path.intermediateUUIDs());
}

void PathsCollection::resetCurrentPath()
{
    mCurrentPath = 0;
}

bool PathsCollection::hasNextPath()
{
    return (mCurrentPath < mPaths.size());
}

Path::Shared PathsCollection::nextPath()
{
    if (mCurrentPath > mPaths.size()) {
        throw IndexError("PathsCollection::nextPath "
                                 "no paths are available");
    }
    mCurrentPath++;
    return make_shared<Path>(
        mSourceNode,
        mDestinationNode,
        mPaths.at(mCurrentPath - 1));
}

size_t PathsCollection::count() const
{
    return mPaths.size();
}