#include <iostream>
#include <gtest/gtest.h>

// TEST(测试名称, 测试用例名称)
// {
//     // 断言宏
//     // ASSERT 系列，如果检测失败，退出当前测试用例
//     // EXPECT 系列，如果检测失败，依然执行后续代码

//     // bool 值检查
//     // ASSERT_TRUE(参数)，期待结果是 true
//     // ASSERT_FALSE(参数)，期待结果是 false

//     // 数值型数据检查
//     // ASSERT_EQ(参数 1，参数 2)，传入的是需要比较的两个数 equal
//     // ASSERT_NE(参数 1，参数 2)， not equal，不等于才返回 true
//     // ASSERT_LT(参数 1，参数 2)， less than，小于才返回 true
//     // ASSERT_GT(参数 1，参数 2)， greater than，大于才返回 true
//     // ASSERT_LE(参数 1，参数 2)， less equal，小于等于才返回 true
//     // ASSERT_GE(参数 1，参数 2)， greater equal，大于等于才返回 true
// }


int add(int x, int y)
{
    return x + y;
}

TEST (test1, add1)
{
    ASSERT_EQ(add(10, 20), 30);
    ASSERT_EQ(add(11, 22), 33);
}

TEST (test2, str1)
{
    std::string str = "hello";
    ASSERT_EQ(str, "hello");
    ASSERT_EQ(str, "cpp");
}


int main(int argc, char* argv[])
{
    // 初始化单元测试框架
    testing::InitGoogleTest(&argc, argv);
   
    // 开始所有单元测试
    return RUN_ALL_TESTS(); // 之前所有的TEST宏函数都会被执行
}