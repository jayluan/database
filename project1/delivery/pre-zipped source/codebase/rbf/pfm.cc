#include "pfm.h"
#include <fstream>
#include <map>
#include <cstring>

PagedFileManager* PagedFileManager::_pf_manager = 0;

PagedFileManager* PagedFileManager::instance()
{
    if(!_pf_manager)
        _pf_manager = new PagedFileManager();

    return _pf_manager;
}


PagedFileManager::PagedFileManager()
{

}


PagedFileManager::~PagedFileManager()
{
    if(_pf_manager != NULL)
        delete _pf_manager;
}


RC PagedFileManager::createFile(const char *fileName)
{
    //Check if the file exists
    std::ifstream f(fileName);
    bool exist = f.good();
    f.close();

    //If doesnt exist, create it.
    if(!exist)
    {
        std::ofstream outFile(fileName, std::ios::binary);
        outFile.write(DB_HEADER_NAME.c_str(), DB_HEADER_NAME.size());
        outFile.close();
        return 0;   
    }

    //If it does exist, return fail
    return -1;
}


RC PagedFileManager::destroyFile(const char *fileName)
{
    std::remove(fileName);
    return 0;
}


RC PagedFileManager::openFile(const char *fileName, FileHandle &fileHandle)
{
    std::fstream file(fileName);
    if(!file || fileHandle.isUsed())
        return -1;  //File doesn't exist or is already a handle for another file
    else
    {
        return fileHandle.loadFile(fileName);
    }
    return 0;
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
    fileHandle.close();
    return 0;
}


FileHandle::FileHandle()
{
    m_fname.clear();
}


FileHandle::~FileHandle()
{
}

FileHandle::FileHandle(const char *filename)
{
    loadFile(filename);
}


RC FileHandle::readPage(PageNum pageNum, void *data)
{
    //First go to the page we want with the GET pointer for reading
    unsigned long seekPos = PAGE_SIZE*pageNum + DB_HEADER_NAME.size();
    m_fhandle.seekg(seekPos, std::ios::beg);

    //Attempt to read the page
    if(!m_fhandle.read(static_cast<char*>(data), PAGE_SIZE))
        return -1;
    return 0;
}


RC FileHandle::writePage(PageNum pageNum, const void *data)
{
    //Check that we're not going over bounds
    if(getNumberOfPages() <= pageNum) return -1;

    //First go to the page we want with the PUT pointer for writing
    unsigned long seekPos = PAGE_SIZE*pageNum + DB_HEADER_NAME.size();
    m_fhandle.seekp(seekPos, std::ios::beg);
    if(!m_fhandle.write(static_cast<const char*>(data), PAGE_SIZE))
        return -1;
    return 0;
}


RC FileHandle::appendPage(const void *data)
{
    m_fhandle.seekp(0, std::ios::end);
    m_fhandle.write(static_cast<const char*>(data), PAGE_SIZE);
    return 0;
}


unsigned FileHandle::getNumberOfPages()
{
    m_fhandle.seekg(0, std::ios::beg);
    unsigned long start = m_fhandle.tellg() + DB_HEADER_NAME.size();    //account for offset of header
    m_fhandle.seekg(0, std::ios::end);
    unsigned long stop = m_fhandle.tellg();
    unsigned long nPages = (stop - start)/PAGE_SIZE;
    return nPages;
}


bool FileHandle::isUsed()
{
    return m_fhandle.is_open();
}

bool FileHandle::isDbFile()
{
    char name[512];
    memset(&name[0], 0, sizeof(name));
    if(!m_fhandle.is_open()) return false;  //file has to be open

    //compare header to see if the name matches target name
    m_fhandle.seekg(0, std::ios::beg);
    m_fhandle.read(name, DB_HEADER_NAME.size());
    std::string fileHeader(name);
    if(fileHeader == DB_HEADER_NAME)
        return true;

    //Default return false
    return false;
}

void FileHandle::close()
{
    if(m_fhandle.is_open())
        m_fhandle.close();
}

RC FileHandle::loadFile(const char *filename)
{
    //Close previous file
    if(m_fhandle.is_open())
        m_fhandle.close();

    //Open new file
    m_fname = filename;
    //m_fhandle.open(filename, std::ios::binary|std::ios::in|std::ios::out|std::ios::app);
    m_fhandle.open(filename, std::ios::binary|std::ios::in|std::ios::out);

    //Checks if file is actually a db file
    if(!m_fhandle.is_open() || !this->isDbFile()) return -1;

    return 0;
}
