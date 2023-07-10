setup:
	mkdir -p /sys/fs/cgroup/cpu
	mount -t cgroup -o cpu cpu /sys/fs/cgroup/cpu
	mkdir -p /sys/fs/cgroup/cpu/mysql
	chown mysql:mysql /sys/fs/cgroup/cpu/mysql

all:
	gcc cgroup.c -o cgroup.so -lcgroup `mysql_config --libs` `mysql_config --include | xargs -n1 | xargs -I {} echo {}/server` -shared

.DEFAULT_GOAL := all
