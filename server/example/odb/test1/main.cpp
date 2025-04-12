#include <string>
#include <memory>  // std::auto_ptr
#include <cstdlib> // std::exit
#include <iostream>
#include <odb/database.hxx>
#include <odb/mysql/database.hxx>

#include "person.hxx"
#include "person-odb.hxx"

int main()
{
    std::shared_ptr<odb::core::database> db(new odb::mysql::database("root", "123456", "TestDB", "127.0.0.1", 0, 0, "utf8"));

    if (!db)
        return -1;

    ptime p = boost::posix_time::second_clock::local_time();
    Person zhang("小张", 18, p);
    Person wang("小王", 19, p);
    typedef odb::query<Person> query;
    typedef odb::result<Person> result;

    {
        odb::core::transaction t(db->begin());
        size_t zid = db->persist(zhang);
        size_t wid = db->persist(wang);
        t.commit();
    }

    {
        odb::core::transaction t (db->begin());
        result r (db->query<Person>());
        for (result::iterator i(r.begin()); i != r.end(); ++i) 
        {
            std::cout << "Hello, " << i->name() << " ";
            std::cout << i->age() << " " << i->update() << std::endl;
        }
        t.commit();
    }

    return 0;
}

// 如果用到了 boost 库中的接口，需要链接库： -lodb-boost
// c++ -o mysql_test mysql_test.cpp person-odb.cxx -lodb-mysql -lodb -lodb-boost