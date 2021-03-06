/*
 * In The Name Of God
 * ========================================
 * [] File Name : main
 *
 * [] Creation Date : 02-07-2015
 *
 * [] Created By : Parham Alvani (parham.alvani@gmail.com)
 * =======================================
*/
/*
 * Copyright (c) 2015 Parham Alvani.
*/

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "common.h"
#include "info.h"
#include "coordinator.h"
#include "connections.h"
#include "command.h"

int is_coordinator = 0;

int main_is_run = 1;

char info_username[255];

int main(int argc, char *argv[])
{
	int coordinator_fd;

	printf("P2PChatroom username: ");
	scanf("%s", info_username);
	getchar();

	coordinator_fd = open("/tmp/coordinator", O_EXCL | O_CREAT | O_WRONLY,
		0664);
	if (coordinator_fd > 0) {
		printf("P2PChatroom rule: coordinator\n");
		is_coordinator = 1;
	} else {
		printf("P2PChatroom rule: peer\n");
		is_coordinator = 0;
	}

	pthread_t id;
	pthread_create(&id, NULL, connections_run, NULL);

	while (main_is_run) {
		char buff[1024];
		printf("P2PChatroom [%s]> ", info_username);
		fgets(buff, 1024, stdin);
		command_dispatcher(buff);
	}

	return 0;
}
