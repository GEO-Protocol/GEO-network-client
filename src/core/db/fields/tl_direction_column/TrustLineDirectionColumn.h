#ifndef GEO_NETWORK_CLIENT_TRUSTLINEDIRECTIONCOLUMN_H
#define GEO_NETWORK_CLIENT_TRUSTLINEDIRECTIONCOLUMN_H


#include "../common/AbstractFileDescriptorHandler.h"
#include "../common/AbstractRecordsHandler.h"

namespace db {
namespace fields {


class TrustLineDirectionColumn:
    protected AbstractFileDescriptorHandler,
    protected AbstractRecordsHandler{

public:
    enum Direction {
        Incoming,
        Outgoing,
        Both,
    };

public:
    TrustLineDirectionColumn(
        const char *filename,
        const char *path);

    void set(
        const RecordNumber recN,
        const Direction direction);

    bool remove(
        const RecordNumber recN);

    const Direction direction(
        const RecordNumber recN);
};


} // namespace fields
} // db

#endif //GEO_NETWORK_CLIENT_TRUSTLINEDIRECTIONCOLUMN_H
