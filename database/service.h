#ifndef SERVICE_H
#define SERVICE_H

#include <optional>
#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

namespace database {
class Service {
 private:
  std::string get_sharding_hint();

  long _id;
  std::string _name;
  long _count;
  long _value;

 public:
  static Service fromJSON(const std::string& str);

  long get_id() const;
  const std::string& get_name() const;
  long get_count() const;
  long get_value() const;

  long& id();
  std::string& name();
  long& count();
  long& value();

  static void init();
  static std::optional<Service> read_by_id(long id);
  static std::vector<Service> read_all();
  void save_to_mysql();

  Poco::JSON::Object::Ptr toJSON() const;
};
}  // namespace database

#endif
