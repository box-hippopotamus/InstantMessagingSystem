#include <string>
#include <memory>  // std::auto_ptr
#include <cstdlib> // std::exit
#include <iostream>
#include <odb/database.hxx>
#include <odb/mysql/database.hxx>

#include <gflags/gflags.h>

#include "student.hxx"
#include "student-odb.hxx"

DEFINE_string(host, "127.0.0.1", "MySQL服务器地址");
DEFINE_int32(port, 3306, "MySQL服务器端口");
DEFINE_string(db, "TestDB", "MySQL使用的数据库");
DEFINE_string(user, "root", "MySQL服务器用户名");
DEFINE_string(pswd, "123456", "MySQL服务器密码");
DEFINE_string(char_set, "utf8", "MySQL使用的字符集");
DEFINE_int32(max_pool, 3, "MySQL连接池最大数量");

void insert_classes(odb::mysql::database& db)
{
    try {
        odb::transaction trans(db.begin()); // 构造事务对象
        Classes c1("一年级1班");
        Classes c2("一年级2班");
        db.persist(c1); // 插入数据
        db.persist(c2);
        trans.commit(); // 提交事务
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

void insert_students(odb::mysql::database& db)
{
    try {
        odb::transaction trans(db.begin()); // 构造事务对象
        Student s1(1, "张三", 18, 1);
        Student s2(2, "李四", 19, 1);
        Student s3(3, "王五", 19, 2);
        Student s4(4, "赵六", 17, 2);
        Student s5(5, "孙七", 18, 2);
        db.persist(s1);
        db.persist(s2);
        db.persist(s3);
        db.persist(s4);
        db.persist(s5);
        trans.commit(); // 提交事务
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

void update_students(odb::mysql::database& db, Student stu)
{
    try {
        odb::transaction trans(db.begin()); // 构造事务对象
        db.update(stu);
        trans.commit(); // 提交事务
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

Student select_students(odb::mysql::database& db)
{
    Student res;
    try {
        odb::transaction trans(db.begin()); // 构造事务对象

        using query_stu = odb::query<Student>;
        using result_stu = odb::result<Student>;

        result_stu r(db.query<Student>(query_stu::name == "张三"));
        if (r.size() != 1)
        {
            std::cout << "数据量错误!" << std::endl;
            return Student();
        }

        res = *r.begin(); // 返回一个迭代器，解引用就是第一个数据

        trans.commit(); // 提交事务
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return res;
}

void remove_students(odb::mysql::database& db)
{
    try {
        odb::transaction trans(db.begin()); // 构造事务对象

        using query_stu = odb::query<Student>;

        db.erase_query<Student>(query_stu::name == "赵六");

        trans.commit(); // 提交事务
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

// 多表查询
void students_classes(odb::mysql::database& db)
{
    try {
        odb::transaction trans(db.begin()); // 构造事务对象

        using query_mix = odb::query<classes_student>;
        using result_mix = odb::result<classes_student>;

        result_mix r(db.query<classes_student>(query_mix::classes::name == "一年级1班"));
        if (r.size() == 0)
        {
            std::cout << "没有结果" << std::endl;
            return;
        }

        for (auto it = r.begin(); it != r.end(); it++)
        {
            std::cout << it->_id << std::endl;
            std::cout << it->_sn << std::endl;
            std::cout << it->_name << std::endl;
            std::cout << *it->_age << std::endl;
            std::cout << it->_class_name << std::endl;
            std::cout << "==================" << std::endl;
        }

        trans.commit(); // 提交事务
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);

    // 构造连接池工厂
    std::unique_ptr<odb::mysql::connection_pool_factory> cpf(
            new odb::mysql::connection_pool_factory(FLAGS_max_pool, 0));

    // 构造数据库对象
    odb::mysql::database db(
        FLAGS_user, FLAGS_pswd, FLAGS_db, FLAGS_host,
        FLAGS_port, "", FLAGS_char_set, 0, std::move(cpf));

    // insert_classes(db);
    // insert_students(db);
    // auto stu = select_students(db);
    // stu._age = 15;
    // update_students(db, stu);
    // remove_students(db);
    students_classes(db);
    return 0;
}