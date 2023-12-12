# Компонентная архитектура
<!-- Состав и взаимосвязи компонентов системы между собой и внешними системами с указанием протоколов, ключевые технологии, используемые для реализации компонентов.
Диаграмма контейнеров C4 и текстовое описание. 
-->
## Компонентная диаграмма

```plantuml
@startuml
!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4_Container.puml

AddElementTag("microService", $shape=EightSidedShape(), $bgColor="CornflowerBlue", $fontColor="white", $legendText="microservice")
AddElementTag("storage", $shape=RoundedBoxShape(), $bgColor="lightSkyBlue", $fontColor="white")

Person(creator, "Создатель чата")
Person(participant, "Участник чата")
Person(user, "Пользователь")

System_Ext(web_messanger, "Web-Мессенджер", "HTML, CSS, JavaScript", "Веб-интерфейс")

System_Boundary(conference_site, "Мессенджер") {
   Container(auth_service, "Сервис авторизаций", "C++", "Сервис авторизаций пользователей", $tags = "microService")   
   Container(message_service, "Сервис сообщений", "C++", "Сервис управления публичными чатами и личными сообщениями", $tags = "microservice")
   Container(user_service, "Сервис управления пользователями", "C++", "Сервис поиска пользователей", $tags = "microservice")
   ContainerDb(db, "База данных", "PostgreSQL", "Хранение данных о чатах, сообщениях, и пользователях", $tags = "storage")   
}

Rel(creator, web_messanger, "Добавление и удаление участников в чата, удаление чата")
Rel(participant, web_messanger, "Просмотр, публикация, редактирование сообщений. Выход из чата")
Rel(user, web_messanger, "Создание чатов, отправка и редактирование личных сообщений")
Rel(web_messanger, auth_service, "Авторизация", "localhost/auth")
Rel(auth_service, db, "INSERT/SELECT/UPDATE", "SQL")

Rel(web_messanger, message_service, "Работа c публичными чатами и личными сообщениями", "localhost/message")
Rel(message_service, db, "INSERT/SELECT/UPDATE", "SQL")
Rel(web_messanger, user_service, "Поиск людей, регистрация новых пользователей", "localhost/user")
Rel(user_service, db, "INSERT/SELECT/UPDATE", "SQL")

@enduml
```
## Список компонентов  

### Сервис авторизаций
**API**:
- Авторизация
    - Пример запроса
      ```http
      POST /auth HTTP/1.1
      Host: localhost
      Content-Type: application/json

      {
        "login": "mySuperLogin",
        "password": "qwerty123"
      }
      ```
    - Пример ответа
      ```http
      HTTP/1.1 200 OK
      Content-Type: application/json

      {
        "token":"auth-token",
        "user_id":"id"
      }
      ```


### Сервис управления пользователями
**API**:
-	Создание нового пользователя
    - Пример запроса
      ```http
      POST /user HTTP/1.1
      Host: localhost
      Content-Type: application/json

      {
          "login": "mySuperLogin",
          "password": "qwerty123",
          "firstname": "Bob",
          "secondname": "Rudolf",
          "email": "bob@rudolf.go"
      } 
      ```
    - Пример ответа
      ```http
      HTTP/1.1 200 OK
      ```
-	Поиск пользователя по логину, или по маске имени, или по маске фамилии, или по маске почты
    - Пример запроса
      ```http
      GET /user/search?login=login&mask-first-name=firstname&mask-second-name=secondname&mask-email=email HTTP/1.1
      Host: localhost
      Authorization: <auth-scheme> <authorization-parameters>
      ```
    - Пример ответа
      ```http
      HTTP/1.1 200 OK
      Content-Type: application/json

      [
        {
          "login": "mySuperLogin",
          "firstname": "Bob",
          "secondname": "Rudolf",
          "email": "bob@rudolf.go"
        },
      ]
      ```
### Сервис сообщений
**API**:
- Создание чата
  - Пример запроса
    ```http
    POST /message/chat HTTP/1.1
    Host: localhost
    Authorization: <auth-scheme> <authorization-parameters>
    Content-Type: application/json

    {
      "chat_name": "chat_name",
      "users": ["user_id_1", "user_id_2"]
    }
    ```
  - Пример ответа
    ```http
    HTTP/1.1 200 OK
    Content-Type: application/json

    {
        "chat_id": "chat_id"
    }
    ```
- Удаление чата
  - Пример запроса
    ```http
    DELETE /message/chat HTTP/1.1
    Host: localhost
    Authorization: <auth-scheme> <authorization-parameters>
    Content-Type: application/json

    {
        "chat_id": "chat_id"
    }
    ```
  - Пример ответа
    ```http
    HTTP/1.1 200 OK
    ```
- Добавление пользователей в чат
  - Пример запроса
    ```http
    POST /message/chat/users&chat_id=chat_id HTTP/1.1
    Host: localhost
    Authorization: <auth-scheme> <authorization-parameters>
    Content-Type: application/json

    {
      "users": ["user_id_1", "user_id_2"]
    }
    ```
  - Пример ответа
    ```http
    HTTP/1.1 200 OK
    ```
- Удаление пользователя из чата
  - Пример запроса
    ```http
    DELETE /message/chat/users&chat_id=chat_id HTTP/1.1
    Host: localhost
    Authorization: <auth-scheme> <authorization-parameters>
    Content-Type: application/json

    {
      "users": ["user_id_1", "user_id_2"]
    }
    ```
  - Пример ответа
    ```http
    HTTP/1.1 200 OK
    ```
- Создать сообщение в чат
  - Пример запроса
    ```http
    POST /message HTTP/1.1
    Host: localhost
    Authorization: <auth-scheme> <authorization-parameters>
    Content-Type: application/json

    {
      "chat_id": "chat_id",
      "message": "message"
    }
    ```
  - Пример ответа
    ```http
    HTTP/1.1 200 OK
    ```
- Создать личное сообщение
  - Пример запроса
    ```http
    POST /message/personal HTTP/1.1
    Host: localhost
    Authorization: <auth-scheme> <authorization-parameters>
    Content-Type: application/json

    {
      "user_id": "user_id",
      "message": "message"
    }
    ```
  - Пример ответа
    ```http
    HTTP/1.1 200 OK
    ```
- Изменить сообщение
  - Пример запроса
    ```http
    PUT /message?message_id=id HTTP/1.1
    Host: localhost
    Authorization: <auth-scheme> <authorization-parameters>
    Content-Type: application/json

    {
      "message": "message"
    }
    ```
  - Пример ответа
    ```http
    HTTP/1.1 200 OK
    ```
- Удалить сообщение
  - Пример запроса
    ```http
    DELETE /message HTTP/1.1
    Host: localhost
    Authorization: <auth-scheme> <authorization-parameters>
    Content-Type: application/json

    {
      "message_id": "id"
    }
    ```
  - Пример ответа
    ```http
    HTTP/1.1 200 OK
    ```
