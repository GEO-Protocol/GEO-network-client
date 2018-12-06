#include "BaseAddress.h"

bool operator== (
    BaseAddress::Shared address1,
    BaseAddress::Shared address2)
{
    if (address1->typeID() != address2->typeID()) {
        return false;
    }
    return address1->fullAddress() == address2->fullAddress();
}