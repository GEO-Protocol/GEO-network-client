/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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