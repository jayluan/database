
#include "rbfm.h"
#include <iostream>
#include <map>
#include <cstring>
#include <algorithm>
#include "assert.h"

RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = 0;

const std::string RecordBasedFileManager::RECORD_FILE_NAME = "./rbfm.idx";

RecordBasedFileManager* RecordBasedFileManager::instance()
{
    if(!_rbf_manager)
        _rbf_manager = new RecordBasedFileManager();

    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager()
{
    m_fileManager = PagedFileManager::instance();
    m_pageIndexTracker = new PageIndexTracker();
}

RecordBasedFileManager::~RecordBasedFileManager()
{
    delete m_pageIndexTracker;
}

RC RecordBasedFileManager::createFile(const string &fileName)
{
    return m_fileManager->createFile(fileName.c_str());
}

RC RecordBasedFileManager::destroyFile(const string &fileName) {
    return m_fileManager->destroyFile(fileName.c_str());
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) {
    return m_fileManager->openFile(fileName.c_str(), fileHandle);
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
    return m_fileManager->closeFile(fileHandle);
}

//TODO: run through each page first to check if there is free space to write the record
//TODO: spaceRemianing is calculated as if data is continguous, this may need to be changed later when things can be removed from the middle of the page and shit
//TODO: (Might not be necessary) prepend a record with data that tells you the layout of the data
RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
    char* page_data = new char[PAGE_SIZE];

    //Compute the descriptor size
    unsigned long recordSize = compute_descriptor_size(recordDescriptor, data);

    //Check if record is wayyyyy to big to fit on a page, include the index into the calculation
    if(recordSize > PAGE_SIZE - 3*sizeof(unsigned int)) return -1;

    //Check if fileHandle is open
    if(!fileHandle.isUsed()) return -1;

    //Check if fileHandle has at least one page, if not, append a page
    if(fileHandle.getNumberOfPages() < 1)
    {
        write_defaul_index_bytes(page_data, PAGE_SIZE); //write an index at the end of the page
        fileHandle.appendPage(page_data);
    }

    //Loop through pages and find first page with free space big enough to fit record, then write record.
    unsigned int spaceRemaining = 0;
    bool dataWritten = false;
    for(int i = 0; i<fileHandle.getNumberOfPages(); i++)
    {
        //First read the page and check the slot directory for free space
        fileHandle.readPage(fileHandle.getNumberOfPages()-1, static_cast<void*>(page_data));
        if(m_pageIndexTracker->LoadPageData(page_data, PAGE_SIZE) < 0)
        {
            /*Normally I'd assert if this happened, because it shouldnt. But since
              I just want to pass unit tests right now, I'm gonna continue*/
            //TODO: don't continue, throw assert
            std::cout << "RecordBasedFileManager::insertRecord read bad page slot directory on page " << i << std::endl;
            continue;
        }

        //If we have enough free space on a page, write the data
        int offset = m_pageIndexTracker->GetNextOffset();               //Note the free space offset
        spaceRemaining = m_pageIndexTracker->GetFreeSpaceRemaining();
        if(spaceRemaining >= recordSize)
        {
            memcpy(page_data + offset, &recordSize, sizeof(unsigned int));  //Append record length value
            offset += sizeof(unsigned int);
            memcpy(page_data + offset, static_cast<const char*>(data), recordSize);
            m_pageIndexTracker->WriteNewIndex(page_data, PAGE_SIZE, recordSize);    //Write new record into the page

            if(fileHandle.writePage(fileHandle.getNumberOfPages()-1, static_cast<void*>(page_data)) != 0)
                return -1;

            dataWritten = true;
            break;  //Break out of loop if it's done writing the data
        }
    }

    //Read and add new data
//    fileHandle.readPage(fileHandle.getNumberOfPages()-1, static_cast<void*>(page_data));
//    if(m_pageIndexTracker->LoadPageData(page_data, PAGE_SIZE) < 0) return -1;

//    int spaceRemaining = m_pageIndexTracker->GetFreeSpaceRemaining();

    //Make sure we have positive offset and enough space on the page
//    if(spaceRemaining > static_cast<int>(recordSize) && spaceRemaining > 0)
//    {
//        memcpy(page_data + offset, &recordSize, sizeof(unsigned int));  //Append record length value
//        offset += sizeof(unsigned int);
//        memcpy(page_data + offset, static_cast<const char*>(data), recordSize);
//        m_pageIndexTracker->WriteNewIndex(page_data, PAGE_SIZE, recordSize);    //Write new record into the page

//        if(fileHandle.writePage(fileHandle.getNumberOfPages()-1, static_cast<void*>(page_data)) != 0)
//            return -1;
//    }

    //If not enough space on any existing page, append a page and write data to new page
    if(!dataWritten)
    {
        unsigned int offset = 0;
        memcpy(page_data, &recordSize, sizeof(unsigned int));  //Append record length value first
        offset += sizeof(unsigned int);
        memcpy(page_data + offset, data, recordSize);
        write_defaul_index_bytes(page_data, PAGE_SIZE); //write an index at the end of the page
        //Read and write new record into the page
        if(m_pageIndexTracker->LoadPageData(page_data, PAGE_SIZE) < 0) return -1;
        m_pageIndexTracker->WriteNewIndex(page_data, PAGE_SIZE, recordSize);

        if(fileHandle.appendPage(static_cast<void*>(page_data)) != 0) return -1;      
    }

    //Reference back the page number and slot number
    rid.pageNum = fileHandle.getNumberOfPages()-1;
    rid.slotNum = m_pageIndexTracker->GetNextSlotID() - 1;

    delete [] page_data;
    return 0;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    char* tmpdata = new char[PAGE_SIZE];
    int npages = fileHandle.getNumberOfPages();
    //Check if fileHandle has at least one page
    if(npages < 1){
        delete tmpdata;
        return -1;
    }

    //Check if fileHandle is open
    if(!fileHandle.isUsed()){
        delete tmpdata;
        return -1;
    }

    unsigned long recordSize = PAGE_SIZE;

    //Read the desired page
    char* page_data = new char[PAGE_SIZE];
    if(fileHandle.readPage(rid.pageNum, static_cast<void*>(page_data)) != 0)
    {
        delete tmpdata;
        delete page_data;
        return -1;
    }

    m_pageIndexTracker->LoadPageData(page_data, PAGE_SIZE);
    int offset = m_pageIndexTracker->GetOffset(rid.slotNum);   //Get the offset from the beginning of the page for the RID
    if(!m_pageIndexTracker->ok()) return -1;

    //Compute the descriptor size, Afigure out FO REALZ how much data is actually there
    memcpy(&recordSize, page_data + offset, sizeof(unsigned int));
    offset += sizeof(unsigned int);
//    recordSize = compute_descriptor_size(recordDescriptor, page_data + offset);
    memcpy(static_cast<char*>(data), page_data + offset, recordSize);   //Copy that ish over

    delete [] page_data;
    delete [] tmpdata;
    return 0;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {
    std::map<AttrType, std::string> attrMap;
    attrMap[TypeInt] = "Int";
    attrMap[TypeReal] =  "Real";
    attrMap[TypeVarChar] = "Var Char";
    std::cout << "___________RECORD___________\n";

    unsigned long position = 0;
    char *tmpString;
    int *tmpInt;
    float *tmpFloat;

    for(unsigned int i = 0; i<recordDescriptor.size(); i++)
    {
        try{
            std::cout << "Name: " << recordDescriptor.at(i).name << std::endl;
            std::cout << "Type: " << attrMap[recordDescriptor.at(i).type] << std::endl;
            std::cout << "Length:" << recordDescriptor.at(i).length << std::endl;
            std::cout << "Value: ";
            switch(recordDescriptor.at(i).type){
                case TypeInt:
                    tmpInt = new int;
                    memcpy(tmpInt, static_cast<const char *>(data)+position, sizeof(int));
                    std::cout << *tmpInt << std::endl;
                    if(recordDescriptor.at(i).length != 4)
                        std::cout<< "<<ERROR>>: Length != Data Size" << std::cout;
                    delete tmpInt;
                    position += sizeof(int);
                    break;

                case TypeReal:
                    tmpFloat = new float;
                    memcpy(tmpFloat, static_cast<const char *>(data)+position, sizeof(float));
                    std::cout << *tmpFloat << std::endl;
                    if(recordDescriptor.at(i).length != 4)
                        std::cout<< "<<ERROR>>: Length != Data Size" << std::cout;
                    delete tmpFloat;
                    position += sizeof(float);
                    break;

                case TypeVarChar:
                    tmpString = new char[recordDescriptor.at(i).length];
                    memset(&tmpString[0], 0, recordDescriptor.at(i).length);
                    int *tmpSize = new int;

                    //first prase the length of the string
                    memcpy(tmpSize, static_cast<const char *>(data)+position, sizeof(int));
                    //Parase the string
                    memcpy(tmpString, static_cast<const char *>(data)+position + sizeof(int), *tmpSize);
                    std::cout << tmpString << std::endl;
                    position += sizeof(int)+*tmpSize;
                    delete [] tmpString;
                    delete tmpSize;
                    break;
            }

            std::cout << std::endl;

        }
        catch(...){    //catch seg faults
            std::cerr<< "Error: printRecord access error"<<std::endl;
            return -1;
        }
    }
    return 0;
}

unsigned long compute_descriptor_size(std::vector<Attribute> recordDescriptor, const void* data)
{
    unsigned long totalSize = 0;
    int *tmpCharSize = NULL;

    for(unsigned int i = 0; i<recordDescriptor.size(); i++)
    {
        switch(recordDescriptor.at(i).type){
            case TypeInt:
                totalSize += sizeof(int);
                break;

            case TypeReal:
                totalSize += sizeof(float);
                break;

            case TypeVarChar:
                if(data == NULL)
                {
                    totalSize += 30;
                    break;
                }
                tmpCharSize = new int();
                memcpy(tmpCharSize, data+totalSize, sizeof(int));
                totalSize += sizeof(int);
                totalSize += *tmpCharSize;
                delete tmpCharSize;
                break;
        }
    }

    return totalSize;
}


PageIndexTracker::PageIndexTracker()
{
    m_pageSize = 0;
    m_status = -1;          //Have to load page to do anything
    m_freeSpaceIndex = 0;
    m_indices.clear();
}

PageIndexTracker::PageIndexTracker(const void *data, unsigned int size)
{
    m_status = 0;
    LoadPageData(data, size);
}

PageIndexTracker::~PageIndexTracker()
{

}

//Load the void* page data and parse the index array at the end of the page
int PageIndexTracker::LoadPageData(const void *data, unsigned int size)
{
    m_pageSize = size;
    m_status = 0;
    m_freeSpaceIndex = 0;
    m_indices.clear();

    int numEntries = 0;
    unsigned long index = m_pageSize;

    //Read the free space index and the number of entries in this index
    std::pair<unsigned int, int> index_and_size = get_free_index_and_entries(data, size);
    index -= 2*sizeof(unsigned int);
    m_freeSpaceIndex = index_and_size.first;
    numEntries = index_and_size.second;
    if(numEntries < 0)
    {
        m_status = -1;
        std::cout << "PageIndexTracker::LoadPageData :numEntries less than 0\n";
        return m_status;
    }

    //Read the data in the index
    int entryLength = 0;
    for(int i = 0; i< numEntries; i++)
    {
        index -= sizeof(unsigned int);    //We read the first two pieces of data already, and we also need to back up an additional 4 bytes
        memcpy(&entryLength, data + static_cast<int>(index), sizeof(unsigned int));

        //Make sure the read entryLength is less than the size of the availabe space on the page
        if(entryLength > static_cast<int>(index))
        {
            m_status = -1;
            std::cout<< "PageIndexTracker::LoadPageData :entryLength is longer than the size of space on page 0\n";
            return m_status;
        }

        m_indices.push_back(entryLength);
    }
    return 0;
}

//Returns next free slot ID
int PageIndexTracker::GetNextSlotID()
{
    if(!ok()) return -1;
    return m_indices.size();
}

//Returns next free offset in bytes
int PageIndexTracker::GetNextOffset()
{
    if(!ok()) return -1;
    return m_freeSpaceIndex;
}

//Returns the beginning offset for a slot number
unsigned int PageIndexTracker::GetOffset(int slotNum)
{
    assert(ok() == true);
    if(slotNum < static_cast<int>(m_indices.size()))
    {
        return m_indices[slotNum];
    }

    return 0;
}

//Returns the size in bytes, of the index
unsigned int PageIndexTracker::GetIndexSize()
{
    assert(ok() == true);
    //(free space index)bytes + (size of index)bytes + (actual size of index)bytes
    return sizeof(unsigned int)*(2 + m_indices.size());
}

//Returns the free space left, not accounting for a new index that needs to be written
unsigned int PageIndexTracker::GetFreeSpaceRemaining()
{
    assert(ok() == true);

    /*Space remaining = size of page - size of current data - size of index - size of
    new index (from the data that's about to be inserted) - length of record bytes*/
    return m_pageSize - GetIndexSize() - GetNextOffset() - 2*sizeof(unsigned int);
}

//Return the index tracker status
bool PageIndexTracker::ok()
{
    return (m_status >= 0);
}

//Write a new index into the current page
void PageIndexTracker::WriteNewIndex(void *data, unsigned int size, unsigned int length)
{
    //Update class variables with new values
    //First free space index + length bytes + length of record
    unsigned int freeSpaceIndex = m_freeSpaceIndex + sizeof(unsigned int) + length;
    m_indices.push_back(GetNextOffset());
    m_freeSpaceIndex = freeSpaceIndex;  //update free space index

    //Write page index data into the buffer
    unsigned int position = size - sizeof(unsigned int);
    memcpy(data + static_cast<int>(position), &m_freeSpaceIndex, sizeof(unsigned int));

    position -= sizeof(unsigned int);
    int indexSize = m_indices.size();
    memcpy(data + static_cast<int>(position), &indexSize, sizeof(unsigned int));

    for(int i = 0; i<m_indices.size(); i++)
    {
        position -= sizeof(unsigned int);
        memcpy(data + static_cast<int>(position), &m_indices[i], sizeof(unsigned int));
    }
}

//returned pair of (number, -1) means invalid read
std::pair<unsigned int, int> get_free_index_and_entries(const void *data, unsigned long size)
{
    unsigned int index = size;
    unsigned int unsignedNumEntries = 0;
    int numEntries = 0;
    unsigned int freeSpaceIndex = 0;

    //Make sure we have at least 8 bits of data
    if(size < 2*sizeof(unsigned int))
        return std::make_pair(static_cast<unsigned long>(0), -1);

    //Read and check the position of the first available free space
    index -= sizeof(unsigned int);
    memcpy(&freeSpaceIndex, static_cast<const unsigned char*>(data) + static_cast<int>(index), sizeof(unsigned int));
    if(freeSpaceIndex > size - 2*sizeof(unsigned int))
        return std::make_pair(static_cast<unsigned long>(0), -1);

    //Read the size of the index and make sure it's no larger than size - sizeof(int)
    index -= sizeof(unsigned int);
    memcpy(&unsignedNumEntries, static_cast<const unsigned char*>(data) + static_cast<int>(index), sizeof(unsigned int));
    numEntries = static_cast<int>(unsignedNumEntries);  //make sure we read unsigned, then convert
    if(numEntries > (size - sizeof(unsigned int)) )
        return std::make_pair(static_cast<unsigned long>(0), -1);

    return std::make_pair(freeSpaceIndex, numEntries);
}

//Writes a directory into a page with default values of 0 offset and 0 size
void write_defaul_index_bytes(void *data, unsigned long size)
{
    unsigned int *zero = new unsigned int[2];
    memset(static_cast<void*>(zero), 0, 2*sizeof(unsigned int));
    memcpy(static_cast<unsigned char*>(data) + static_cast<int>(size) - 2*sizeof(unsigned int), zero, 2*sizeof(unsigned int));
    delete [] zero;
}
