#pragma once
#include <string>
#include <cstddef> // std::size_t
#include <boost/date_time/posix_time/posix_time.hpp>
#include <odb/core.hxx>
#include <odb/nullable.hxx>

#pragma db object
class Student
{
public:
    Student() = default;
    Student(unsigned long sn,  std::string name,
             unsigned short age, unsigned long class_id)
        : _sn(sn)
        , _name(name)
        , _age(age)
        , _class_id(class_id)
    {}

    friend odb::access; // 设置友元类，这样odb才能访问私有

public:
    #pragma db id auto
    unsigned long _id;
    #pragma db unique
    unsigned long _sn;
    std::string _name;
    odb::nullable<unsigned short> _age;
    #pragma db index
    unsigned long _class_id;
};

#pragma db object
class Classes
{
public:
    Classes() = default;
    Classes(unsigned long id, std::string name)
        : _id(id)
        , _name(name)
    {}

    Classes(std::string name)
        : _name(name)
    {}

    friend odb::access; // 设置友元类，这样odb才能访问私有

public:
    #pragma db id auto
    unsigned long _id;
    std::string _name;
};

// 查询指令
// view 构建视图,指定表的连接条件，视图最后映射为C++的结构体
// 视图连接了两张表
#pragma db view object(Student)\
                object(Classes = classes : Student::_class_id == classes::_id)\
                query((?))
struct classes_student
{
    #pragma db column(Student::_id) // 指定表中的关联字段
    unsigned long _id;

    #pragma db column(Student::_sn)
    unsigned long _sn;

    #pragma db column(Student::_name)
    std::string _name;

    #pragma db column(Student::_age)
    odb::nullable<unsigned short> _age;

    #pragma db column(classes::_name) // 此处使用小写，因为前面重命名了
    std::string _class_name;
};

// 只查询学生姓名
#pragma db view query("select name form Student" + (?)) // (?) 传入外部条件
struct all_name
{
    std::string _name;
};

// odb -d mysql --std c++11 --generate-query --generate-schema --profile boost/date-time student.hxx