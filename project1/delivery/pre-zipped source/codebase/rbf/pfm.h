#ifndef _pfm_h_
#define _pfm_h_
#include <fstream>
#include <string>

typedef int RC;
typedef unsigned PageNum;

#define PAGE_SIZE 4096
const static int HEADER_SIZE = 17;
const std::string DB_HEADER_NAME = "jay's_cool_header";

class FileHandle;


class PagedFileManager
{
public:
    static PagedFileManager* instance();                     // Access to the _pf_manager instance

    RC createFile    (const char *fileName);                         // Create a new file
    RC destroyFile   (const char *fileName);                         // Destroy a file
    RC openFile      (const char *fileName, FileHandle &fileHandle); // Open a file
    RC closeFile     (FileHandle &fileHandle);                       // Close a file

protected:
    PagedFileManager();                                   // Constructor
    ~PagedFileManager();                                  // Destructor

private:
    static PagedFileManager *_pf_manager;
    //std::map<const char*, std::fstream>m_fileMap;
};


class FileHandle
{
public:
    FileHandle();                                                    // Default constructor
    FileHandle(const char *filename);                                // Cstor for loading file
    ~FileHandle();                                                   // Destructor

    RC readPage(PageNum pageNum, void *data);                           // Get a specific page
    RC writePage(PageNum pageNum, const void *data);                    // Write a specific page
    RC appendPage(const void *data);                                    // Append a specific page
    unsigned getNumberOfPages();                                        // Get the number of pages in the file

    bool isUsed();
    bool isDbFile();
    void close();
    RC loadFile(const char *filename);
private:
    std::string m_fname;
    std::fstream m_fhandle;
};

 #endif
