import random
import string

from faker import Faker

import basemod


fake = Faker()

def generate_service_list(n=1):
    def generate_service():
        return {
            "name": ' '.join(fake.words(nb=3)),
            "count": random.randint(1, 200),
            "value": random.randint(1, 10000),
        }

    return [generate_service() for _ in range(n)]

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

        connection.execute(f"""CREATE TABLE IF NOT EXISTS `Services` (
            `id`    INT NOT NULL AUTO_INCREMENT,
            `name`  VARCHAR(1024) NOT NULL,
            `count` INT NOT NULL,
            `value` INT NOT NULL,
            PRIMARY KEY (`id`), KEY `an` (`name`)
        ); {basemod.get_hint(i)}""")

    connection = basemod.MySQLConnection(
        host="all-db",
        port="6033",
        database="archdb",
        user="stud",
        password="stud"
    )

    values = generate_service_list(200)
    connection.insert_values(("INSERT INTO `Services` "
        "(`name`, `count`, `value`) "
        "VALUES (%(name)s, %(count)s, %(value)s)"), values, lambda x : basemod.get_hash(x['name']))

if __name__ == "__main__":
    main()
