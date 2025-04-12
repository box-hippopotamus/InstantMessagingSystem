#pragma once
#include <string>
#include <cstddef> // std::size_t
#include <boost/date_time/posix_time/posix_time.hpp>
/*
在 C++ 中，要使用 ODB 将类声明为持久化类，需要包含 ODB 的核心头文
件，并使用 #pragma db object 指令
#pragma db object 指示 ODB 编译器将 person 类视为一个持久化类。
*/
#include <odb/core.hxx>
#include <odb/nullable.hxx>
typedef boost::posix_time::ptime ptime;

// #pragma db table("TestTable") // 指明表名称
// #pragma db object // 声明以下对象映射到数据库
#pragma db object table("TestTable") 
class Person
{
public:
    Person(const std::string &name, int age, const ptime &update) : _name(name), _age(age), _update(update) {}
    void age(int val) { _age = val; }
    int age() { return _age; }
    void name(const std::string &val) { _name = val; }
    std::string name() { return _name; }
    void update(const ptime &update) { _update = update; }
    std::string update() { return boost::posix_time::to_simple_string(_update); }

private:
        // 将 odb：： access 类作为 person 类的朋友。
        // 这是使数据库支持代码可访问默认构造函数和数据成员所必需的。
        // 如果类具有公共默认构造函数和公共数据成员或数据成员的公共访问器和修饰符，则不需要友元声明
    friend class odb::access;
    Person() {}
    //_id 成员前面的 pragma 告诉 ODB 编译器，以下成员是对象的标识符。 
    // auto: 说明符指示它是数据库分配的 ID。 -> 自增长
    // id: 说明这是主键
    #pragma db id auto // 表示 ID 字段将自动生成（通常是数据库中的主键）。 
    unsigned long _id;

    // default:("") 指定默认值
    // column:("")指定列名称，否则默认使用成员名(自动去除前后的 _)
    #pragma db column("user_age") 
    unsigned short _age;

    // unique: 唯一键
    #pragma db unique
    std::string _name;

    // type: 指定类型 -> 可以指定decimal，因为C++的默认类型无法自动映射为decimal
    // not_null: 非空, 但是所有字段默认是非空的
    // index: 创建普通索引
    #pragma db type("TIMESTAMP") not_null index
    boost::posix_time::ptime _update;

    // null: 可以为空 -> 需要额外指定
    #pragma db type("DECIMAL") null
    double _math_score;

    // odb::nullable 对成员包装后，也可以指定为空，此时相当于一个智能指针
    // 使用前需要判断指针是否为空
    #pragma db type("DECIMAL")
    odb::nullable<double> _english_score;
};

//  odb -d mysql --std c++11 --generate-query --generate-schema --profile boost/date-time person.hxx