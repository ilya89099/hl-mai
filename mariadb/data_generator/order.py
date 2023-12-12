import random
import string

from faker import Faker

import basemod


fake = Faker()

def flatten(l):
    return [item for sublist in l for item in sublist]

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

def generate_order_list(n=1):
    user_ids = get_unique_ids('user')
    service_ids = get_unique_ids('services')

    n = min(n, len(user_ids))

    def generate_order_entries(user_id):
        order_size = random.randrange(len(service_ids))
        entries = random.choices(service_ids, k=order_size)

        return [{"user_id": user_id, "service_id": pi} for pi in entries]

    return flatten([generate_order_entries(i) for i in range(n)])

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

        connection.execute(f"""CREATE TABLE IF NOT EXISTS `Orders` (
            `id`         INT NOT NULL AUTO_INCREMENT,
            `user_id`    INT NOT NULL,
            `service_id` INT NOT NULL,
            PRIMARY KEY (`id`), KEY `uid` (`user_id`)
        ); {basemod.get_hint(i)}""")

    connection = basemod.MySQLConnection(
        host="all-db",
        port="6033",
        database="archdb",
        user="stud",
        password="stud"
    )

    values = generate_order_list(200)
    connection.insert_values(("INSERT INTO `Orders` "
        "(`user_id`, `service_id`) "
        "VALUES (%(user_id)s, %(service_id)s)"), values, lambda x : x['user_id'])

if __name__ == "__main__":
    main()
