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


def generate_chats(users, n=1):
    user_ids = get_unique_ids('user')

    n = min(n, len(user_ids))

    def generate_chat():
        return {
            "name": ' '.join(fake.words(nb=4)),
            "creator_id": random.choice(user_ids),
        }

    return [generate_chat() for _ in range(n)]

def main():
    connection = None

    for i in range(basemod.SHARD_COUNT):
        connection = basemod.MySQLConnection(
            host="all-db",
            port="6033",
            database="archdb",
            user="stud",
            password="stud"
        )

        connection.execute("""CREATE TABLE IF NOT EXISTS `Chat` (
            `id` INT NOT NULL AUTO_INCREMENT,
            `name` VARCHAR(1024) NOT NULL,
            `creator_id` INT NOT NULL,
            PRIMARY KEY (`id`)); -- sharding:0""")

    connection = basemod.MySQLConnection(
        host="all-db",
        port="6033",
        database="archdb",
        user="stud",
        password="stud"
    )

    values = generate_chats(300)
    connection.insert_values(("INSERT INTO `Chat` "
                              "(`name`, `creator_id`) "
                              "VALUES (%(name)s, %(creator_id)s)"), values, lambda x : 0)

if __name__ == "__main__":
    main()
