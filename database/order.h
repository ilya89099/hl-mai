#ifndef CART_H
#define CART_H

#include <optional>
#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

namespace database {
class Order {
 private:
  std::string get_sharding_hint();

  long _user_id;
  std::vector<long> _service_ids;

 public:
  static Order fromJSON(const std::string& str);

  long get_user_id() const;
  const std::vector<long>& get_service_ids() const;

  long& user_id();
  std::vector<long>& service_ids();

  static void init();
  static Order read_by_user_id(long id);
  void save_to_mysql();

  Poco::JSON::Object::Ptr toJSON() const;
};
}  // namespace database

#endif
