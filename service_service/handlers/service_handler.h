#ifndef USEPRODUCTHANDLER_H
#define USEPRODUCTHANDLER_H

#include <fstream>
#include <iostream>
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Exception.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/ThreadPool.h"
#include "Poco/Timestamp.h"
#include "Poco/URI.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/ServerApplication.h"

#include <Poco/Base64Decoder.h>
#include <Poco/Base64Encoder.h>

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::NameValueCollection;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include "../../database/service.h"
#include "../../helper.h"

class ServiceHandler : public HTTPRequestHandler {
 public:
  ServiceHandler(const std::string& format) : _format(format) {}

  std::optional<std::string> do_get(const std::string& url,
                                    const std::string& login,
                                    const std::string& password) {
    std::string string_result;
    try {
      std::string token = login + ":" + password;
      std::ostringstream os;
      Poco::Base64Encoder b64in(os);
      b64in << token;
      b64in.close();
      std::string identity = "Basic " + os.str();

      Poco::URI uri(url);
      Poco::Net::HTTPClientSession s(uri.getHost(), uri.getPort());
      Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET,
                                     uri.toString());
      request.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
      request.setContentType("application/json");
      request.set("Authorization", identity);
      request.set("Accept", "application/json");
      request.setKeepAlive(true);

      s.sendRequest(request);

      Poco::Net::HTTPResponse response;
      std::istream& rs = s.receiveResponse(response);

      while (rs) {
        char c{};
        rs.read(&c, 1);
        if (rs)
          string_result += c;
      }

      if (response.getStatus() != 200)
        return {};
    } catch (Poco::Exception& ex) {
      std::cout << "exception:" << ex.what() << std::endl;
      return std::optional<std::string>();
    }

    return string_result;
  }

  bool authRequest(HTTPServerRequest& request) {
    HTMLForm form(request, request.stream());
    std::string scheme;
    std::string info;
    std::string login, password;

    request.getCredentials(scheme, info);
    if (scheme == "Basic") {
      get_identity(info, login, password);
      std::cout << "login:" << login << std::endl;
      std::cout << "password:" << password << std::endl;
      std::string host = "localhost";
      std::string url;

      if (std::getenv("AUTH_HOST") != nullptr) {
        host = std::getenv("AUTH_HOST");
      }
      url = "http://" + host + ":8080/auth";

      return do_get(url, login, password) ? true : false;
    }

    return false;
  }

  void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
    HTMLForm form(request, request.stream());

    if (!authRequest(request)) {
      response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
      response.setChunkedTransferEncoding(true);
      response.setContentType("application/json");
      Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
      root->set("type", "/errors/unauthorized");
      root->set("title", "Unauthorized");
      root->set("detail", "invalid login or password");
      root->set("instance", "/user");
      std::ostream& ostr = response.send();
      Poco::JSON::Stringifier::stringify(root, ostr);

      return;
    }

    if (hasSubstr(request.getURI(), "/get") &&
          (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)) {
      handleGet(request, response);
      return;
    }

    if (hasSubstr(request.getURI(), "/create") &&
          (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)) {
      handleCreate(request, response);
      return;
    }

    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");
    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    root->set("type", "/errors/not_found");
    root->set("title", "Internal exception");
    root->set("detail", "request not found");
    root->set("instance", "/user");
    std::ostream& ostr = response.send();
    Poco::JSON::Stringifier::stringify(root, ostr);
  }

  void handleGet(HTTPServerRequest& request, HTTPServerResponse& response) {
    HTMLForm form(request, request.stream());

    Poco::JSON::Array arr;

    if (form.has("id")) {
      long id = atol(form.get("id").c_str());
      std::optional<database::Service> result = database::Service::read_by_id(id);
      if (result) {
        arr.add(result->toJSON());
      }
    } else {
      std::vector<database::Service> results = database::Service::read_all();
      for (const auto& service : results) {
        arr.add(service.toJSON());
      }
    }

    if (!arr.empty()) {
      response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
      response.setChunkedTransferEncoding(true);
      response.setContentType("application/json");
      std::ostream& ostr = response.send();
      arr.stringify(ostr);
      return;
    } else {
      response.setStatus(
          Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
      response.setChunkedTransferEncoding(true);
      response.setContentType("application/json");
      Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
      root->set("type", "/errors/not_found");
      root->set("title", "Service(s) not found");
      root->set("detail", "Service(s) not found");
      root->set("instance", "/service");
      std::ostream& ostr = response.send();
      Poco::JSON::Stringifier::stringify(root, ostr);
      return;
    }
  }

  void handleCreate(HTTPServerRequest& request, HTTPServerResponse& response) {
    HTMLForm form(request, request.stream());

    const std::vector<std::string> form_fields{ "name", "count", "value" };
    std::vector<size_t> missing_fields;

    for (size_t i = 0; i < form_fields.size(); ++i) {
      if (!form.has(form_fields[i])) {
        missing_fields.push_back(i);
      }
    }

    if (missing_fields.empty()) {
      database::Service service;
      service.name() = form.get("name");
      service.count() = atol(form.get("count").c_str());
      service.value() = atol(form.get("value").c_str());
      service.save_to_mysql();

      response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
      response.setChunkedTransferEncoding(true);
      response.setContentType("application/json");
      std::ostream& ostr = response.send();
      ostr << service.get_id();
    }

    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");
    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    root->set("type", "/errors/bad_request");
    root->set("title", "Missing fields in create request");
    root->set("detail", "Missing fields in create request");
    root->set("instance", "/service");
    std::ostream& ostr = response.send();
    Poco::JSON::Stringifier::stringify(root, ostr);
  }

 private:
  std::string _format;
};
#endif
