#ifndef GEO_NETWORK_CLIENT_RECORDNUMBERSINDEX_H
#define GEO_NETWORK_CLIENT_RECORDNUMBERSINDEX_H

#include "../../../common/Types.h"
#include "AbstractFileDescriptorHandler.h"
#include "AbstractRecordsHandler.h"
#include "../../../common/exceptions/ValueError.h"


namespace db {
namespace fields {


/*!
 * todo: tests are needed;
 *
 * This index stores offsets of the data blocks (in some field),
 * that are assigned to some record number.
 *
 * The format of the index:
 *    rec 1 -> offset of data block.
 *    rec 2 -> offset of data block.
 *    ...
 *    rec N -> offset of data block.
 *
 * This index is designed with assumption,
 * that records will be added to the table successively,
 * and will be removed rarely, or, even, never.
 *
 *
 *
 * ToDo: this index format is inefficient.
 * In case when records will be massively removed from the file -
 * the "holes" with not used record numbers will be present,
 * and allocate the space in the file.
 * In next versions this index should be refined.
 */
class RecordNumbersIndex: protected AbstractFileDescriptorHandler, AbstractRecordsHandler {
public:
    typedef uint32_t IndexRecordOffset; // Offset of the record into the index.
    typedef uint32_t DataOffset;        // Offset of the data block in column storage.

public:
    RecordNumbersIndex(const char *path, const char *filename);

    void set(
        const RecordNumber recN,
        const DataOffset storageOffset);

    void remove(
        const RecordNumber recN);

    const DataOffset dataOffset(
        const RecordNumber recN) const;

protected:
    struct FileHeader {
        explicit FileHeader();

        // Version is useful in case when data migration is needed.
        uint16_t version;
    };

    struct IndexRecord {
        explicit IndexRecord();
        explicit IndexRecord(const DataOffset offset);

        DataOffset offset;

        // Records are physically removing from the file
        // only when vacuum() is called.
        // Instead of removing, records are only marking as removed.
        //
        // To mark record as removed - special record value is used:
        // max. possible offset.
        static const constexpr DataOffset kRemovedRecordValue =
            std::numeric_limits<DataOffset>::max();
    };

protected:
    void open(
        const char *accessMode);

    FileHeader loadFileHeader() const;
    void updateFileHeader(
        const FileHeader *header) const;

    inline const IndexRecordOffset recordOffset(
        const RecordNumber recN) const;
};

} // namespace fields
} // namespace db

#endif //GEO_NETWORK_CLIENT_RECORDNUMBERSINDEX_H
