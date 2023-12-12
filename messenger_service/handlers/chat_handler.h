#ifndef CHATHANDLER_H
#define CHATHANDLER_H

#include "Poco/JSON/Parser.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/URI.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include <Poco/Base64Decoder.h>
#include <Poco/Base64Encoder.h>

#include <iostream>
#include <iostream>
#include <fstream>

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
using Poco::URI;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include "../../database/chat.h"
#include "../../database/user_to_chat.h"
#include "../../helper.h"

class ChatHandler : public HTTPRequestHandler
{
 public:
  ChatHandler()
  {
  }

  std::optional<std::string> do_get(const std::string& url, const std::string& identity) {
    std::string string_result;

    try {
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

  std::optional<std::string> do_get(const std::string& url,
                                    const std::string& login,
                                    const std::string& password) {
    std::string token = login + ":" + password;
    std::ostringstream os;
    Poco::Base64Encoder b64in(os);
    b64in << token;
    b64in.close();
    std::string identity = "Basic " + os.str();

    return do_get(url, identity);
  }

  bool authRequest(HTTPServerRequest& request, long& user_id) {
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

      std::optional<std::string> string_result = do_get(url, login, password);

      if (string_result) {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(*string_result);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
        user_id = object->getValue<long>("id");

        return true;
      }
    }

    return false;
  }

  void handleRequest(HTTPServerRequest &request,
                     HTTPServerResponse &response)
  {
    long user_id;

    HTMLForm form(request, request.stream());

    if (!authRequest(request, user_id)) {
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


    try
    {
      // получить чаты по user_id и получить чат по chat_id
      // GET /chat?chat_id={chat_id}
      // GET /chat?user_id={user_id}
      if (isGet(request)) {
        if (contains(request.getURI(), "searchByChatId")) {
          if (!form.has("chatId")) {
            badRequest(response, "chatId not exist's");
          }
          auto chatId = atol(form.get("chatId").c_str());
          auto chat = database::Chat::read_by_id(chatId);
          if (chat) {
            auto jsonChat = chat->toJSON();
            ok(response, jsonChat);
          } else {
            notFound(response, "cannot find chat with id " + std::to_string(chatId));
          }
          return;
        }

        if (contains(request.getURI(), "searchByUserId")) {
          if (!form.has("userId")) {
            badRequest(response, "userId not exist's");
            return;
          }
          long query_param_id = atol(form.get("userId").c_str());

          auto users_to_chats = database::UserToChat::read_chats_by_user_id(query_param_id);
          Poco::JSON::Object::Ptr content = new Poco::JSON::Object();
          Poco::JSON::Array::Ptr arr = new Poco::JSON::Array();
          for (auto s : users_to_chats) {
            arr->add(s.toJSON());
          }
          content->set("chats", arr);
          ok(response, content);
          return;
        }
      }
            
      if (isPost(request)) { // создать чат
        if (contains(request.getURI(), "addMember")) {
          if (!form.has("chatId") || !form.has("userId")) {
            badRequest(response, "do not have valid parameters chatId and userId");
            return;
          }
          long chatId = atol(form.get("chatId").c_str());
          long userId = atol(form.get("userId").c_str());
          database::UserToChat userToChat;
          userToChat.chat_id() = chatId;
          userToChat.user_id() = userId;

          try {
            userToChat.save_to_mysql();
            Poco::JSON::Object::Ptr content = new Poco::JSON::Object();
            ok(response, content);
          } catch (Poco::Exception& ex) {
            std::cout << "Exception: " << ex.what() << std::endl;
            unprocessableEntity(response, std::string(ex.what()));
          }
          return;
        }
        if(form.has("chatName")) {
          try {
            database::Chat chat;
            database::UserToChat userToChat;
            chat.name() = form.get("chatName");
            chat.creator_id() = user_id;
            std::string message;
            std::string reason;
            Poco::JSON::Object::Ptr content = new Poco::JSON::Object();
            chat.save_to_mysql();
            content->set("chat_id", chat.get_id());
            userToChat.chat_id() = chat.get_id();
            userToChat.user_id() = user_id;
            userToChat.save_to_mysql();
            ok(response, content);
          } catch (const Poco::Exception& ex) {
            std::cout << "Exception: " << ex.what() << std::endl;
            internalServerError(response, std::string(ex.what()));
          }
          return;
        }
      }
    }
    catch (...)
    {
      std::cout << "Unexpected error";
      internalServerError(response);
      return;
    }

    notFound(response, "/chat");
  }
};
#endif