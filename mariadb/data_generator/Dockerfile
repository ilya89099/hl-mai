# syntax=docker/dockerfile:1

FROM python:3.8-slim-buster

WORKDIR /app

COPY requirements.txt requirements.txt
RUN pip3 install -r requirements.txt

COPY messages.py basemod.py chat.py user_to_chats.py user.py entrypoint.sh ./

CMD [ "sh", "./entrypoint.sh" ]
