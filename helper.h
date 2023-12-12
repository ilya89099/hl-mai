#pragma once
#ifndef HELPER_LIB_H
#define HELPER_LIB_H

#include <sstream>
#include <istream>
#include <ostream>
#include <string>
#include "Poco/Base64Decoder.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/JSON/Object.h"

inline bool hasSubstr(const std::string& str, const std::string& substr) {
  if (str.size() < substr.size())
    return false;
  for (size_t i = 0; i <= str.size() - substr.size(); ++i) {
    bool ok{true};
    for (size_t j = 0; ok && (j < substr.size()); ++j)
      ok = (str[i + j] == substr[j]);
    if (ok)
      return true;
  }
  return false;
}

inline bool startsWith(const std::string& str, const  std::string& substr) {
  return str.find(substr) == 0;
}

inline bool get_identity(const std::string identity, std::string& login,
                  std::string& password) {
  std::istringstream istr(identity);
  std::ostringstream ostr;
  Poco::Base64Decoder b64in(istr);
  copy(std::istreambuf_iterator<char>(b64in), std::istreambuf_iterator<char>(),
       std::ostreambuf_iterator<char>(ostr));
  std::string decoded = ostr.str();

  size_t pos = decoded.find(':');
  login = decoded.substr(0, pos);
  password = decoded.substr(pos + 1);
  return true;
}

inline long get_hash(const std::string& str) {
  long p = 53;
  long m = 61566613;

  long roll = 0;
  for (auto i = str.crbegin(); i < str.crend(); ++i) {
    roll = (roll * p + int(*i)) % m;
  }

  return roll;
}

inline bool isPost(Poco::Net::HTTPServerRequest &request) {
  return request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST;
}

inline bool isGet(Poco::Net::HTTPServerRequest &request) {
  return request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET;
}

inline bool isPut(Poco::Net::HTTPServerRequest &request) {
  return request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT;
}

inline bool isDelete(Poco::Net::HTTPServerRequest &request) {
  return request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE;
}

inline bool contains(const std::string& str, const std::string& substr) {
  return str.find(substr) != std::string::npos;
}

inline void createReponse(
    Poco::Net::HTTPServerResponse &response,
    Poco::Net::HTTPResponse::HTTPStatus status,
    Poco::JSON::Object::Ptr content
) {
  response.setStatus(status);
  response.setChunkedTransferEncoding(true);
  response.setContentType("application/json");
  std::ostream &ostr = response.send();
  Poco::JSON::Stringifier::stringify(content, ostr);
}

inline Poco::JSON::Object::Ptr createError(
    const std::string& detail,
    const std::string& title
) {
  Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
  root->set("title", title);
  root->set("detail", detail);
  return root;
}

inline void badRequest(Poco::Net::HTTPServerResponse &response) {
  createReponse(
      response, Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST,
      createError(
          "Bad request",
          "/errors/bad_request"
          )
  );
}

inline void badRequest(Poco::Net::HTTPServerResponse &response, const std::string& reason) {
  createReponse(
      response, Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST,
      createError(
          reason,
          "/errors/bad_request"
          )
  );
}

inline void notFound(Poco::Net::HTTPServerResponse &response) {
  createReponse(
      response, Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND,
      createError(
          "not found",
          "/errors/not_found"
          )
  );
}

inline void notFound(Poco::Net::HTTPServerResponse &response, const std::string& msg) {
  createReponse(
      response, Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND,
      createError(
          msg,
          "/errors/not_found"
          )
  );
}

inline void unauthorized(Poco::Net::HTTPServerResponse &response) {
  createReponse(
      response, Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED,
      createError(
          "Unauthorized",
          "/errors/unauthorized"
          )
  );
}

inline void forbidden(Poco::Net::HTTPServerResponse &response) {
  createReponse(
      response, Poco::Net::HTTPResponse::HTTPStatus::HTTP_FORBIDDEN,
      createError(
          "Forbidden",
          "/errors/forbidden"
          )
  );
}



inline void internalServerError(Poco::Net::HTTPServerResponse &response) {
  createReponse(
      response, Poco::Net::HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR,
      createError(
          "Internal server Error",
          "/errors/internal"
          )
  );
}



inline void internalServerError(Poco::Net::HTTPServerResponse &response, const std::string& msg) {
  createReponse(
      response, Poco::Net::HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR,
      createError(
          msg,
          "/errors/forbidden"
          )
  );
}

inline void unprocessableEntity(Poco::Net::HTTPServerResponse &response, const std::string& msg) {
  createReponse(
      response, Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNPROCESSABLE_ENTITY,
      createError(
          msg,
          "/errors/forbidden"
          )
  );
}

inline void ok(Poco::Net::HTTPServerResponse &response, Poco::JSON::Object::Ptr& content) {
  createReponse(response, Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK, content);
}

#endif
