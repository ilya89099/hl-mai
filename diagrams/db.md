# Основные сущности

## Users (5)
- user_id: UUID (PK)
- first_name: Varchar(100)
- second_name: Varchar(100)
- login: Varchar(100)
- password: Varchar(100)
- status: Enum(ACTIVE, DEACTIVED)
```
ix__login__password
```

## Messages (1)
- message_id: UUID (PK)
- chat_id: UUID (FK)
- message: Varchar(2048)
- created_at: Timestamp
- edited_at: Timestamp
```
ix__chat_id__created_at_desc
```

## Chats (3)
- chat_id: UUID (PK)
- chat_name: Varchar(100)
- description: Varchar(100)
- created_at: Timestamp
- edited_at: Timestamp
- creator_user_id: UUID?
- type: Enum(PERSONAL, PUBLIC)
```
ix__chat_id__type
```

## Users To Chats (2)
- chat_id: UUID (FK)
- user_id: UUID (FK)
- created_at: Timestamp
```
ix__chat_id__user_id__created_at_desc
ix__user_id__chat_id__created_at_desc
```

# Cоединения таблиц

**[Users]** >=(Users To Chats)=< **[Chats]**

**[Chats]** --< **[Messages]**



# Основные функциональности
- Регистрация (добавление поля в таблицу Users)
- Авторизация (получение логина и пароля через индекс ix__login__password)
- Получение списка публичных чатов (из таблицы chats через индекс ix__chat_id__type)
- Получение списка личных чатов (из таблицы chats через индекс ix__chat_id__type)
- Получение сообщений в чате (получение сообщений через chat_id в таблице Messages)
