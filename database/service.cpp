#include "service.h"

#include <exception>
#include <sstream>
#include <functional>

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Parser.h>

#include "../config/config.h"
#include "database.h"
#include "../helper.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database {
void Service::init() {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    for (const auto& hint : database::Database::get_all_sharding_hints()) {
      Statement create_stmt(session);
      create_stmt << "CREATE TABLE IF NOT EXISTS `Services` ("
                  << "`id` INT NOT NULL AUTO_INCREMENT,"
                  << "`name` VARCHAR(1024) NOT NULL,"
                  << "`count` INT NOT NULL,"
                  << "`value` INT NOT NULL,"
                  << "PRIMARY KEY (`id`), KEY `an` (`name`)"
                  << ");"
                  << hint,
          now;
    }
  }

  catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
    throw;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
    throw;
  }
}

Poco::JSON::Object::Ptr Service::toJSON() const {
  Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

  root->set("id", _id);
  root->set("name", _name);
  root->set("count", _count);
  root->set("value", _value);

  return root;
}

Service Service::fromJSON(const std::string& str) {
  Service service;
  Poco::JSON::Parser parser;
  Poco::Dynamic::Var result = parser.parse(str);
  Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

  service.id() = object->getValue<long>("id");
  service.name() = object->getValue<std::string>("name");
  service.count() = object->getValue<long>("count");
  service.value() = object->getValue<long>("value");

  return service;
}

std::optional<Service> Service::read_by_id(long id) {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    for (const auto& hint : database::Database::get_all_sharding_hints()) {
      Poco::Data::Statement select(session);
      Service p;
      select << "SELECT id, name, count, value "
                "FROM Services where id=? " + hint,
          into(p._id), into(p._name), into(p._count), into(p._value), use(id),
          range(0, 1);  //  iterate over result set one row at a time

      select.execute();
      Poco::Data::RecordSet rs(select);
      if (rs.moveFirst()) {
        return p;
      }
    }
  }

  catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
  }

  return {};
}

std::vector<Service> Service::read_all() {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    std::vector<Service> result;
    Service p;
    for (const auto& hint : database::Database::get_all_sharding_hints()) {
      Statement select(session);
      select << "SELECT id, name, count, value "
                "FROM Services" + hint,
          into(p._id), into(p._name), into(p._count), into(p._value),
          range(0, 1);  //  iterate over result set one row at a time

      while (!select.done()) {
        if (select.execute())
          result.push_back(p);
      }
    }

    return result;
  }

  catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
    throw;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
    throw;
  }
}

std::string Service::get_sharding_hint() {
  return database::Database::get_sharding_hint(get_hash(_name));
}

void Service::save_to_mysql() {

  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement insert(session);

    insert
        << "INSERT INTO Services (name, count, value) "
           "VALUES(?, ?, ?) " + get_sharding_hint(),
        use(_name), use(_count), use(_value);

    insert.execute();

// "SELECT id, name, count, value "
//                 "FROM Services where id=? " + hint
    Poco::Data::Statement select(session);
    select << "SELECT id from Services where name=? " + get_sharding_hint(), use(_name), into(_id),
        range(0, 1);  //  iterate over result set one row at a time

    if (!select.done()) {
      select.execute();
    }
    std::cout << "inserted:" << _id << std::endl;

  } catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
    throw;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
    throw;
  }
}

long Service::get_id() const { return _id; }
const std::string& Service::get_name() const { return _name; }
long Service::get_count() const { return _count; }
long Service::get_value() const { return _value; }

long& Service::id() { return _id; }
std::string& Service::name() { return _name; }
long& Service::count() { return _count; }
long& Service::value() { return _value; }

}
