#include "Exception.h"

Exception::Exception(const string &message):
    mMessage(message)
{}

const string Exception::message() const
{
    return mMessage;
}
