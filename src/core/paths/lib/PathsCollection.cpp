#include "PathsCollection.h"

PathsCollection::PathsCollection(
    BaseAddress::Shared destinationAddress) :
    mDestinationNode(destinationAddress),
    mCurrentPath(0)
{}

void PathsCollection::add(
        Path::Shared &path)
{
    if (path->length() > 5) {
        throw ValueError("PathsCollection::add "
                             "Added path is too long");
    }
    mPaths.push_back(path);
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
    return mPaths.at(mCurrentPath - 1);
}

size_t PathsCollection::count() const
{
    return mPaths.size();
}