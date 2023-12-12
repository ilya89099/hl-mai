#ifndef AUTHOR_H
#define AUTHOR_H

#include <optional>
#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

namespace database {
class User {
 private:
  std::string get_sharding_hint();

  long _id;
  std::string _first_name;
  std::string _last_name;
  std::string _email;
  std::string _gender;
  std::string _login;
  std::string _password;

 public:
  static User fromJSON(const std::string& str);

  long get_id() const;
  const std::string& get_first_name() const;
  const std::string& get_last_name() const;
  const std::string& get_email() const;
  const std::string& get_gender() const;
  const std::string& get_login() const;
  const std::string& get_password() const;

  long& id();
  std::string& first_name();
  std::string& last_name();
  std::string& email();
  std::string& gender();
  std::string& login();
  std::string& password();

  static void init();
  static std::optional<User> read_by_id(long id, bool use_cache = true);
  static std::optional<long> auth(std::string& login, std::string& password);
  static std::vector<User> read_all();
  static std::vector<User> search(std::string first_name,
                                  std::string last_name);

  void save_to_mysql();
  void send_to_queue();
  void save_to_cache();
  static std::optional<User> read_from_cache_by_id(long id);

  Poco::JSON::Object::Ptr toJSON() const;
};
}  // namespace database

#endif
