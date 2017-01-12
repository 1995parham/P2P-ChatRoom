/*
 * In The Name Of God
 * ========================================
 * [] File Name : connections
 *
 * [] Creation Date : 02-07-2015
 *
 * [] Created By : Parham Alvani (parham.alvani@gmail.com)
 * =======================================
 */
/*
 * Copyright (c) 2015 Parham Alvani.
 */
#include "connections.h"
#include "coordinator.h"
#include "peer.h"
#include "common.h"

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int connection_is_run = 1;

void *connections_run(void *data)
{
	FILE *peers_file;
	int peer_port, peer_status;

	if (is_coordinator) {
		peers_file = fopen("/tmp/peers", "a+");
	} else {
		peers_file = fopen("/tmp/peers", "r");
	}

	if (!peers_file)
		sdie("peers_file fopen()");

	while (fscanf(peers_file, "%d %d", &peer_port, &peer_status) == 2) {
		struct peer *new = peer_new(peer_port, peer_status);

		if (peer_status) {
			int client_socket = socket(AF_INET, SOCK_STREAM, 0);

			struct sockaddr_in client_addr;
			socklen_t client_addr_len = sizeof(client_addr);

			client_addr.sin_family = AF_INET;
			client_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			client_addr.sin_port = htons(peer_port);

			connect(client_socket,
					(const struct sockaddr *) &client_addr,
					client_addr_len);
			new->socket = client_socket;
		}

		peer_list_add(new);
	}

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server_addr;
	socklen_t server_addr_len = sizeof(server_addr);

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(0);

	bind(server_socket, (const struct sockaddr *) &server_addr,
			server_addr_len);
	getsockname(server_socket, (struct sockaddr *) &server_addr,
			&server_addr_len);

	if (is_coordinator) {
		fprintf(peers_file, "%d %d\n", server_addr.sin_port, 1);
	}

	while (connection_is_run) {
		int i = 0;
		int max_socket_fd;
		fd_set socket_fds_set;

		FD_ZERO(&socket_fds_set);
		FD_SET(server_socket, &socket_fds_set);

		max_socket_fd = server_socket;

		for (i = 0; i < peer_list_size(); i++) {
			int s;

			s = peer_list_get(i)->socket;
			max_socket_fd = (s > max_socket_fd) ? s : max_socket_fd;
			FD_SET(s, &socket_fds_set);
		}

		if (select(max_socket_fd + 1, &socket_fds_set,
					NULL, NULL, NULL) < 0)
			sdie("select");

		if (FD_ISSET(server_socket, &socket_fds_set)) {
			struct sockaddr_in client_addr;
			socklen_t client_addr_len;

			client_addr_len = sizeof(client_addr);

			int client_socket = accept(server_socket,
					(struct sockaddr *) &client_addr,
					&client_addr_len);

			struct peer *new = peer_new(client_addr.sin_port, 1);
			new->socket = client_socket;
			peer_list_add(new);

			if (is_coordinator) {
				fprintf(peers_file, "%d %d\n", new->port, new->status);
			}
		}
		for (i = 0; i < peer_list_size(); i++) {
			struct peer *p;

			p = peer_list_get(i);
			if (FD_ISSET(p->socket, &socket_fds_set)) {
				fflush(stdout);
				printf("P2PChatroom [%s]: %s", p->name, "Hello");
			}
		}

	}
	return NULL;
}
