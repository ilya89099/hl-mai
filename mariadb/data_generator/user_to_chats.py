import random
import string

from faker import Faker

import basemod


fake = Faker()


def get_unique_ids(table):
    connection = basemod.MySQLConnection(
        host="all-db",
        port="6033",
        database="archdb",
        user="stud",
        password="stud"
    )
    ids_list = connection.get(f"SELECT id FROM {table.title()};")
    ids_list = list(map(lambda x : x[0], ids_list))
    print(ids_list)
    return ids_list


def generate_users_chats(n=1):
    user_ids = get_unique_ids('user')
    chat_ids = get_unique_ids('chat')

    n = min(n, len(user_ids), len(chat_ids))

    random_users = random.sample(user_ids, n)
    random_chats = random.sample(chat_ids, n)

    def generate_user_chat(i):
        return {
            "user_id": random_users[i],
            "chat_id": random_chats[i],
        }

    return [generate_user_chat(i) for i in range(n)]

def main():
    connection = None
    connection = basemod.MySQLConnection(
        host="proxysql",
        port="6033",
        database="archdb",
        user="stud",
        password="stud"
    )

    connection.execute("""CREATE TABLE IF NOT EXISTS `UserToChat` (
        `chat_id` INT NOT NULL,
        `user_id` INT NOT NULL,
        PRIMARY KEY(chat_id,user_id)); -- sharding:0""")

    connection = basemod.MySQLConnection(
        host="proxysql",
        port="6033",
        database="archdb",
        user="stud",
        password="stud"
    )

    user_to_chats = generate_users_chats(100)

    connection.insert_values(("INSERT INTO `UserToChat` "
                              "(`chat_id`, `user_id`) "
                              "VALUES (%(chat_id)s, %(user_id)s)"), user_to_chats, lambda x : 0)

if __name__ == "__main__":
    main()
