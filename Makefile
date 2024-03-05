setup:
	mkdir -p /sys/fs/cgroup/mysql
	chown -R mysql:mysql /sys/fs/cgroup/mysql
	echo threaded > /sys/fs/cgroup/mysql/cgroup.type

all:
	gcc cgroup.c -o cgroup.so -lcgroup `mysql_config --libs` `mysql_config --include | xargs -n1 | xargs -I {} echo {}/server` -shared

docker:
	sudo ./contrib/docker/build.sh

.DEFAULT_GOAL := all
