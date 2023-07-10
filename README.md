## Demo

![](/mariadb-cgroup-plugin.gif)

## Setup/Compile

```
make setup
make
```

## Copy plugin

```
MariaDB [(none)]> show variables where Variable_Name = 'plugin_dir';
+---------------+------------------------+
| Variable_name | Value                  |
+---------------+------------------------+
| plugin_dir    | /usr/lib/mysql/plugin/ |
+---------------+------------------------+
1 row in set (0.002 sec)
```

`cp cgroup.so /usr/lib/mysql/plugin/`

NOTE: The plugin directory might be different.

Restart mysql.

## Verify

```
    INSTALL PLUGIN cgroup SONAME 'cgroup.so';
    MariaDB [(none)]> SHOW VARIABLES LIKE 'cgroup_enabled';
    +----------------+-------+
    | Variable_name  | Value |
    +----------------+-------+
    | cgroup_enabled | OFF   |
    +----------------+-------+
    1 row in set (0.00 sec)
```

## Turn it on (default OFF)

`SET GLOBAL cgroup_enabled=ON;`

## Configure cgroups using cgroup-tools

### /etc/cgroup.conf

```
group mysql/donatas {
  perm {
    admin {
      uid = mysql;
      gid = mysql;
    }
    task {
      uid = mysql;
      gid = mysql;
    }
  }

  cpu { 
    cpu.shares = 100;
    cpu.cfs_quota_us = 1000;
  }
}
    
group mysql/donatas2 {
  perm {
    admin {
      uid = mysql;
      gid = mysql;
    }
    task {
      uid = mysql;
      gid = mysql;
    }
  }

  cpu {
    cpu.shares = 100;
    cpu.cfs_quota_us = 1000000;
  }
}
```

### Reload config

`cgconfigparser -l /etc/cgconfig.conf`

## Logs

`tail -f /var/lib/mysql/cgroup.log`

## Extra troubleshooting commands

Create two MySQL users donatas1, and donatas2, then generate some fake CPU for each user.

```
$ for x in $(seq 1 1000); do mysql -u donatas1 -ptest -e 'select * from information_schema.columns'; done
$ for x in $(seq 1 1000); do mysql -u donatas2 -ptest -e 'select * from information_schema.columns'; done
$ systemd-cgtop -P -d 1 --order=path /mysql
$ watch -n1 'cat /sys/fs/cgroup/cpu/mysql/donatas/tasks'
```
