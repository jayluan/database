#include "gtest/gtest.h"

TEST(jayTest, assertTest)
{
    int i = 1000;
    ASSERT_EQ(1000, i);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
