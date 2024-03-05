#!/bin/bash

IMAGE_VERSION="${IMAGE_VERSION:=$(git rev-parse --short=10 HEAD)}"
IMAGE_NAME=mariadb-cgroup-plugin

if [[ -v DOCKER_NO_CACHE ]]; then
	docker build --no-cache -t "${IMAGE_NAME}:${IMAGE_VERSION}" -f contrib/docker/Dockerfile .
else
	docker build -t "${IMAGE_NAME}:${IMAGE_VERSION}" -f contrib/docker/Dockerfile .
fi

CONTAINER_ID="$(docker run -d "${IMAGE_NAME}:${IMAGE_VERSION}")"
docker cp "${CONTAINER_ID}:/app/cgroup.so" .

docker stop --time 30 "${CONTAINER_ID}"
