#ifndef _rbfm_h_
#define _rbfm_h_

#include <string>
#include <vector>

#include "../rbf/pfm.h"
#include "pfm.h"
#include <map>

using namespace std;

class PageIndexTracker;

typedef struct
{
    int pageNum;    //page number of the record
    int slotNum;
}RecordEntry;

// Record ID
typedef struct
{
  unsigned pageNum;
  unsigned slotNum;
} RID;


// Attribute
typedef enum { TypeInt = 0, TypeReal, TypeVarChar } AttrType;

typedef unsigned AttrLength;

struct Attribute {
    string   name;     // attribute name
    AttrType type;     // attribute type
    AttrLength length; // attribute length
};

// Comparison Operator (NOT needed for part 1 of the project)
typedef enum { EQ_OP = 0,  // =
           LT_OP,      // <
           GT_OP,      // >
           LE_OP,      // <=
           GE_OP,      // >=
           NE_OP,      // !=
           NO_OP       // no condition
} CompOp;



/****************************************************************************
The scan iterator is NOT required to be implemented for part 1 of the project 
*****************************************************************************/

# define RBFM_EOF (-1)  // end of a scan operator

// RBFM_ScanIterator is an iteratr to go through records
// The way to use it is like the following:
//  RBFM_ScanIterator rbfmScanIterator;
//  rbfm.open(..., rbfmScanIterator);
//  while (rbfmScanIterator(rid, data) != RBFM_EOF) {
//    process the data;
//  }
//  rbfmScanIterator.close();fileHandle.getNumberOfPages()-1


class RBFM_ScanIterator {
public:
  RBFM_ScanIterator() {};
  ~RBFM_ScanIterator() {};

  // "data" follows the same format as RecordBasedFileManager::insertRecord()
  RC getNextRecord(RID &rid, void *data) { return RBFM_EOF; };
  RC close() { return -1; };
};


class RecordBasedFileManager
{
public:
  static RecordBasedFileManager* instance();

  RC createFile(const string &fileName);
  
  RC destroyFile(const string &fileName);
  
  RC openFile(const string &fileName, FileHandle &fileHandle);
  
  RC closeFile(FileHandle &fileHandle);

  //  Format of the data passed into the function is the following:
  //  1) data is a concatenation of values of the attributes
  //  2) For int and real: use 4 bytes to store the value;
  //     For varchar: use 4 bytes to store the length of characters, then store the actual characters.
  //  !!!The same format is used for updateRecord(), the returned data of readRecord(), and readAttribute()
  RC insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid);

  RC readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data);
  
  // This method will be mainly used for debugging/testing
  RC printRecord(const vector<Attribute> &recordDescriptor, const void *data);

/**************************************************************************************************************************************************************
***************************************************************************************************************************************************************
IMPORTANT, PLEASE READ: All methods below this comment (other than the constructor and destructor) are NOT required to be implemented for part 1 of the project
***************************************************************************************************************************************************************
***************************************************************************************************************************************************************/
  RC deleteRecords(FileHandle &fileHandle);

  RC deleteRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid);

  // Assume the rid does not change after update
  RC updateRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, const RID &rid);

  RC readAttribute(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, const string attributeName, void *data);

  RC reorganizePage(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const unsigned pageNumber);

  // scan returns an iterator to allow the caller to go through the results one by one. 
  RC scan(FileHandle &fileHandle,
      const vector<Attribute> &recordDescriptor,
      const string &conditionAttribute,
      const CompOp compOp,                  // comparision type such as "<" and "="
      const void *value,                    // used in the comparison
      const vector<string> &attributeNames, // a list of projected attributes
      RBFM_ScanIterator &rbfm_ScanIterator);


// Extra credit for part 2 of the project, please ignore for part 1 of the project
public:

  RC reorganizeFile(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor);


protected:
  RecordBasedFileManager();
  ~RecordBasedFileManager();

private:
  static RecordBasedFileManager *_rbf_manager;
  PagedFileManager *m_fileManager;
  PageIndexTracker *m_pageIndexTracker;
  static const std::string RECORD_FILE_NAME;
};


/**
 * @brief The PageIndexTracker class can read a buffer and parse the index data
 *
 * TODO:
 *      - Implement an index that contains both slot ID and offset for when we
 *        need to actually edit entries in the pages. Right now we just store
 *        everything contiguously so what I'm doing is okay but it's temporary.
 *
 *      - Implement logic to handle deletion of slot values
 */
class PageIndexTracker
{
public:
    PageIndexTracker();
    PageIndexTracker(const void *data, unsigned int size = PAGE_SIZE);
    ~PageIndexTracker();

    int LoadPageData(const void *data, unsigned int size = PAGE_SIZE);
    int GetNextSlotID();
    int GetNextOffset();
    unsigned int GetOffset(int slotNum);
    unsigned int GetIndexSize();
    bool ok();
    void WriteNewIndex(void* data, unsigned int size, unsigned int length);

private:
    unsigned int m_pageSize;
    int m_status;
    unsigned int m_freeSpaceIndex;
    std::vector<unsigned int>m_indices;
};

void write_defaul_index_bytes(void* data, unsigned long size);
std::pair<unsigned int, int> get_free_index_and_entries(const void* data, unsigned long size);
unsigned long compute_descriptor_size(std::vector<Attribute> recordDescriptor, const void* data = NULL);
#endif

//TODO create a class to parses, reads, and appends a header data to the file
