/*
 * hello_world.hpp
 *
 *  Created on: 12 сент. 2018 г.
 *      Author: ringo
 */

#ifndef HELLO_WORLD_HPP_
#define HELLO_WORLD_HPP_
#include <iostream>
#include <cstdio>
#include <cstring>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/epoll.h>

enum RUN_METHOD {
	SIMPLE, EPOLL
};

int make_socket(const char *, const char *);
void listen(int sock);
int read_from_sock(int);

constexpr int thread_epoll_max_events = 512;
struct ThreadMaster {
	int epoll_instance;
	struct epoll_event epoll_events[thread_epoll_max_events];
};

#endif /* HELLO_WORLD_HPP_ */
