#include "order.h"

#include <exception>
#include <sstream>

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Parser.h>

#include "../config/config.h"
#include "database.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database {
void Order::init() {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    for (const auto& hint : database::Database::get_all_sharding_hints()) {
      Statement create_stmt(session);
      create_stmt << "CREATE TABLE IF NOT EXISTS `Orders` ("
                  << "`id` INT NOT NULL AUTO_INCREMENT,"
                  << "`user_id` INT NOT NULL,"
                  << "`service_id` INT NOT NULL,"
                  << "PRIMARY KEY (`id`), KEY `uid` (`user_id`)"
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

Poco::JSON::Object::Ptr Order::toJSON() const {
  Poco::JSON::Array service_ids;
  for (const auto& e : get_service_ids()) {
    service_ids.add(e);
  }

  // Oh yeah and that's a memory leak
  // Too bad!
  Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

  root->set("user_id", _user_id);
  root->set("service_ids", service_ids);

  return root;
}

Order Order::fromJSON(const std::string& str) {
  Order order;
  Poco::JSON::Parser parser;
  Poco::Dynamic::Var result = parser.parse(str);
  Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

  order.user_id() = object->getValue<long>("user_id");

  // no i do not care
  auto service_array = object->getArray("service_ids");
  for (size_t i = 0; i < service_array->size(); ++i) {
    order.service_ids().push_back(service_array->get(i));
  }

  return order;
}

Order Order::read_by_user_id(long id) {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement select(session);
    Order c;
    c.user_id() = id;
    select << "SELECT id, user_id, service_id "
              "FROM Orders where user_id=? " +
              c.get_sharding_hint(), use(id);
    select.execute();

    Poco::Data::RecordSet rs(select);

    c.user_id() = id;
    for (auto& row : rs) {
      c.service_ids().push_back(row["service_id"].convert<long>());
    }

    return c;
  }

  catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
  }

  return {};
}

std::string Order::get_sharding_hint() {
  return database::Database::get_sharding_hint(_user_id);
}

void Order::save_to_mysql() {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement insert(session);

    struct {
      long user_id;
      long service_id;
    } entry;

    insert
        << "INSERT INTO Orders (user_id, service_id) "
           "VALUES(?, ?) " + get_sharding_hint(),
        use(entry.user_id), use(entry.service_id);

    entry.user_id = _user_id;
    for (const auto& e : _service_ids) {
      entry.service_id = e;
      insert.execute();
    }

    std::cout << "inserted for user:" << _user_id << std::endl;

  } catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
    throw;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
    throw;
  }
}

long Order::get_user_id() const { return _user_id; }
const std::vector<long>& Order::get_service_ids() const { return _service_ids; }

long& Order::user_id() { return _user_id; }
std::vector<long>& Order::service_ids() { return _service_ids; }

}
