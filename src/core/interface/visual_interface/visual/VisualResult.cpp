#include "VisualResult.h"

VisualResult::VisualResult(
    const string &visualIdentifier,
    const string &visualInformation):
    mVisualIdentifier(visualIdentifier),
    mVisualInformation(visualInformation)
{}

const string VisualResult::serialize() const
{
    return mVisualIdentifier + kTokensSeparator +
           mVisualInformation + kCommandsSeparator;
}
