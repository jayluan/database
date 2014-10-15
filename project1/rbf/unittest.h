#ifndef UNITTEST_H
#define UNITTEST_H
#include "rbfm.h"
#include "pfm.h"
#include "gtest/gtest.h"

class SlotDirectoryTest: public ::testing::Test {
protected:

    void SetUp();
    void TearDown();

    unsigned int s_recordSize;
    unsigned int s_freespacePos;
    void *s_page;
    unsigned int s_page_size;
    SlotDirectory s_slotDirectory;
};

void writePageCatalogue(void*page, unsigned int size, unsigned int numEntries, unsigned int recordSize);
void createLargeRecordDescriptor(vector<Attribute> &recordDescriptor);
void createRecordDescriptor(vector<Attribute> &recordDescriptor);
void prepareLargeRecord(const int index, void *buffer, int *size);
void prepareRecord(const int nameLength, const string &name, const int age, const float height, const int salary, void *buffer, int *recordSize);

#endif // UNITTEST_H
