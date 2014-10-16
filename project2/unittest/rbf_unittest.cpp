#include "rbf_unittest.h"

void SlotDirectoryTest::SetUp()
{
    //Setup Page
    s_page_size = PAGE_SIZE;
    s_page = static_cast<void *>(new char[s_page_size]);
    s_freespacePos = 0;
    s_recordSize = 10;

    //Create catalogue data on the page
    writePageCatalogue(s_page, s_page_size, s_recordSize, s_freespacePos);

    s_slotDirectory.LoadPageData(s_page, s_page_size);
//    for(int i = 0; i<3; i++)
//        prepareLargeRecord(i, );

}
void SlotDirectoryTest::TearDown()
{
    delete static_cast<char*>(s_page);
}

//Make sure loading works
TEST_F(SlotDirectoryTest, LoadPageDataTest)
{
    ASSERT_EQ(s_slotDirectory.ok(), true);
    ASSERT_EQ(s_slotDirectory.LoadPageData(s_page, s_page_size), 0);
}

//Check GetNextOffset. Should be 10
TEST_F(SlotDirectoryTest, GetNextSlotID)
{
    ASSERT_EQ(s_slotDirectory.ok(), true);
    ASSERT_EQ(s_slotDirectory.GetNextSlotID(), s_recordSize);
}

//Check next offset
TEST_F(SlotDirectoryTest, GetNextOffset)
{
    ASSERT_EQ(s_slotDirectory.ok(), true);
    ASSERT_EQ(s_slotDirectory.GetNextOffset(), s_freespacePos);
}

//Check get offset of a slot number
TEST_F(SlotDirectoryTest, GetOffset)
{
    int desiredPos = 5;
    ASSERT_LT(desiredPos, s_recordSize);
    ASSERT_EQ(s_slotDirectory.GetOffset(desiredPos), desiredPos);
    ASSERT_EQ(s_slotDirectory.ok(), true);
}

//Check bytes of the size of the index
TEST_F(SlotDirectoryTest, GetDirectorySize)
{
    ASSERT_EQ(s_slotDirectory.ok(), true);
    ASSERT_EQ(s_slotDirectory.GetDirectorySize(), (s_recordSize + 2)*sizeof(unsigned int));
}

TEST_F(SlotDirectoryTest, WriteNewIndex)
{
    ASSERT_EQ(s_slotDirectory.ok(), true);

    unsigned int fakeRecordLength = 45;
    void *test_data = static_cast<char*>(new char[s_page_size]);
    writePageCatalogue(test_data, s_page_size, s_recordSize+1, s_freespacePos + fakeRecordLength + sizeof(unsigned int));

    s_slotDirectory.WriteNewSlot(s_page, s_page_size, fakeRecordLength);

    SlotDirectory a(test_data, PAGE_SIZE);
    SlotDirectory b(s_page, PAGE_SIZE);
    ASSERT_EQ(memcmp(test_data, s_page, s_page_size), 0);

    delete static_cast<char*>(test_data);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

//Creates fake page catalogue with numEntries and free space at freespacePos
void writePageCatalogue(void*page, unsigned int size, unsigned int numEntries, unsigned int freespacePos)
{
    unsigned int offset = size;

    //Write first free-space location (fake)
    offset -= sizeof(unsigned int);
    memcpy(page + offset, &freespacePos, sizeof(unsigned int));

    //Write number of entires
    offset -= sizeof(unsigned int);
    memcpy(page + offset, &numEntries, sizeof(unsigned int));

    //Write entries
    for(unsigned int i = 0; i< numEntries-1; i++)
    {
        offset -= sizeof(unsigned int);
        memcpy(page+offset, &i, sizeof(unsigned int));
    }
    offset -= sizeof(unsigned int);
    memcpy(page+offset, &freespacePos, sizeof(unsigned int));
}

// Function to prepare the data in the correct form to be inserted/read
void prepareRecord(const int nameLength, const string &name, const int age, const float height, const int salary, void *buffer, int *recordSize)
{
    int offset = 0;

    memcpy((char *)buffer + offset, &nameLength, sizeof(int));
    offset += sizeof(int);
    memcpy((char *)buffer + offset, name.c_str(), nameLength);
    offset += nameLength;

    memcpy((char *)buffer + offset, &age, sizeof(int));
    offset += sizeof(int);

    memcpy((char *)buffer + offset, &height, sizeof(float));
    offset += sizeof(float);

    memcpy((char *)buffer + offset, &salary, sizeof(int));
    offset += sizeof(int);

    *recordSize = offset;
}

void prepareLargeRecord(const int index, void *buffer, int *size)
{
    int offset = 0;

    // compute the count
    int count = index % 50 + 1;

    // compute the letter
    char text = index % 26 + 97;

    for(int i = 0; i < 10; i++)
    {
        memcpy((char *)buffer + offset, &count, sizeof(int));
        offset += sizeof(int);

        for(int j = 0; j < count; j++)
        {
            memcpy((char *)buffer + offset, &text, 1);
            offset += 1;
        }

        // compute the integer
        memcpy((char *)buffer + offset, &index, sizeof(int));
        offset += sizeof(int);

        // compute the floating number
        float real = (float)(index + 1);
        memcpy((char *)buffer + offset, &real, sizeof(float));
        offset += sizeof(float);
    }
    *size = offset;
}

void createRecordDescriptor(vector<Attribute> &recordDescriptor) {

    Attribute attr;
    attr.name = "EmpName";
    attr.type = TypeVarChar;
    attr.length = (AttrLength)30;
    recordDescriptor.push_back(attr);

    attr.name = "Age";
    attr.type = TypeInt;
    attr.length = (AttrLength)4;
    recordDescriptor.push_back(attr);

    attr.name = "Height";
    attr.type = TypeReal;
    attr.length = (AttrLength)4;
    recordDescriptor.push_back(attr);

    attr.name = "Salary";
    attr.type = TypeInt;
    attr.length = (AttrLength)4;
    recordDescriptor.push_back(attr);

}

void createLargeRecordDescriptor(vector<Attribute> &recordDescriptor)
{
    int index = 0;
    char *suffix = (char *)malloc(10);
    for(int i = 0; i < 10; i++)
    {
        Attribute attr;
        sprintf(suffix, "%d", index);
        attr.name = "attr";
        attr.name += suffix;
        attr.type = TypeVarChar;
        attr.length = (AttrLength)50;
        recordDescriptor.push_back(attr);
        index++;

        sprintf(suffix, "%d", index);
        attr.name = "attr";
        attr.name += suffix;
        attr.type = TypeInt;
        attr.length = (AttrLength)4;
        recordDescriptor.push_back(attr);
        index++;

        sprintf(suffix, "%d", index);
        attr.name = "attr";
        attr.name += suffix;
        attr.type = TypeReal;
        attr.length = (AttrLength)4;
        recordDescriptor.push_back(attr);
        index++;
    }
    free(suffix);
}
