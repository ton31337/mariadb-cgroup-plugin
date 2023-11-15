// SPDX-License-Identifier: ISC

/* Copyright (c) 2023, Donatas Abraitis
 *
 * TL;DR; A drop-in replacement for CloudLinux MySQL Governor.
 *
 * Put MySQL threads into cgroups under /sys/fs/cgroup/mysql/<username>
 * to limit specific users separately.
 *
 */

#define MYSQL_DYNAMIC_PLUGIN
#include <mysql.h>
#include <mysql/plugin.h>
#include <mysql/plugin_audit.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>

static my_bool sys_cgroup_enabled = 0;

static void cgroup_toggle(MYSQL_THD thd __attribute__((unused)),
			  struct st_mysql_sys_var *var __attribute__((unused)),
			  void *var_ptr __attribute__((unused)),
			  const void *save __attribute__((unused)))
{
	sys_cgroup_enabled = *(my_bool *)save;
};

static MYSQL_SYSVAR_BOOL(enabled, sys_cgroup_enabled, PLUGIN_VAR_OPCMDARG,
			 "Enable/disable cgroup support", NULL, cgroup_toggle,
			 0);

static void cgroup_set_task(const char *cgroup_name)
{
	FILE *f;
	FILE *fcgpath;
	char cgpath[BUFSIZ] = {};
	pid_t tid = syscall(SYS_gettid);

	/* This might never happen, but avoid crashing MySQL just in case */
	if (!cgroup_name)
		return;

	f = fopen("cgroup.log", "a");
	if (!f)
		return;

	snprintf(cgpath, sizeof(cgpath), "/sys/fs/cgroup/%s/cgroup.threads",
		 cgroup_name);
	fcgpath = fopen(cgpath, "w");
	if (!fcgpath) {
		fprintf(f, "can't open cgroup.threads file: %s\n", cgpath);
		fflush(f);
		fclose(f);
		return;
	}

	fprintf(fcgpath, "%d", tid);
	fclose(fcgpath);

	fflush(f);
	fclose(f);
}

static void cgroup_plugin(MYSQL_THD thd __attribute__((unused)),
			  unsigned int event_class, const void *event)
{
	const struct mysql_event_connection *mec = event;
	char path[BUFSIZ] = {};

	if (event_class != MYSQL_AUDIT_CONNECTION_CLASS)
		return;

	if (!strcmp(mec->user, "root"))
		return;

	if (sys_cgroup_enabled == 0)
		snprintf(path, sizeof(path), "mysql");
	else
		snprintf(path, sizeof(path), "mysql/%s", mec->user);

	switch (mec->event_subclass) {
	case MYSQL_AUDIT_CONNECTION_CONNECT:
	case MYSQL_AUDIT_CONNECTION_CHANGE_USER:
		cgroup_set_task(path);
		break;
	case MYSQL_AUDIT_CONNECTION_DISCONNECT:
		cgroup_set_task("mysql");
		break;
	default:
		break;
	}
}

static struct st_mysql_sys_var *cgroup_vars[] = {MYSQL_SYSVAR(enabled), NULL};

static struct st_mysql_audit cgroup_handler = {
	MYSQL_AUDIT_INTERFACE_VERSION,
	NULL,
	cgroup_plugin,
	{MYSQL_AUDIT_CONNECTION_CHANGE_USER}};

maria_declare_plugin(cgroup)
{
	MYSQL_AUDIT_PLUGIN, &cgroup_handler, "cgroup", "Donatas Abraitis",
		"Attach Linux cgroup to MySQL thread", PLUGIN_LICENSE_GPL, NULL,
		NULL, 0x1, NULL, cgroup_vars, "0.1",
		MariaDB_PLUGIN_MATURITY_STABLE
}
maria_declare_plugin_end;
