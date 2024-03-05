FROM debian:12-slim

RUN apt update && apt upgrade -y && apt clean

RUN apt install libcgroup-dev libmariadbd-dev libmariadb-dev-compat build-essential -y

COPY . /app

RUN cd /app && make

COPY contrib/docker/docker-start /usr/sbin/docker-start

CMD ["/usr/sbin/docker-start"]
