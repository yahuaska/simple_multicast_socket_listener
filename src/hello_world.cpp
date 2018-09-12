//============================================================================
// Name        : hello_world.cpp
// Author      : Ringo
// Version     :
// Copyright   : CC
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <unistd.h>
#include "ospf.hpp"
#include "hello_world.hpp"

constexpr int method = SIMPLE;
constexpr int thread_num = 16;
constexpr const char *default_ifname = "eth0";
constexpr const char *default_address = "0.0.0.0";

int main(int argc, char *argv[]) {
	const char *ifname = default_ifname;
	const char *addr = default_address;
	if (argc > 1) {
		ifname = argv[1];
	}
	if (argc > 2) {
		addr = argv[2];
	}
	if (method == SIMPLE) {
		std::cout << "Running" << std::endl;
		int sock = -1;
		sock = make_socket(ifname, addr);
		if (sock < 0) {
			return -1;
		}
		listen(sock);
	} else {
		std::cout << "Running epoll" << std::endl;
		ThreadMaster tm = { epoll_create1(0) };

		epoll_event events[thread_num];
		int epoll_fd_num;
		;
		for (int i = 0; i < thread_num; i++) {
			events[i].data.fd = make_socket(ifname, addr);
			if (events[i].data.fd < 0) {
				return -1;
			} else {
				std::cout << "fd" << i << ": " << events[i].data.fd
						<< std::endl;
			}
			events[i].events = EPOLLIN;
			epoll_ctl(tm.epoll_instance, EPOLL_CTL_ADD, events[i].data.fd,
					&(events[i]));
		}
		while (1) {
			epoll_fd_num = epoll_wait(tm.epoll_instance, tm.epoll_events,
					thread_epoll_max_events, -1);
			std::cout << "Ready: " << epoll_fd_num << " sockets" << std::endl;
			for (int i = 0; i < epoll_fd_num; ++i) {
				int fd = tm.epoll_events[i].data.fd;
				read_from_sock(fd);
			}
			std::cout << "\n================================\n" << std::endl;
		}
	}
	return 0;
}

/*
 * @brief It will try to read from socket and to print general IP packet data, such as src ip.
 * @param socket — socket file descriptor.
 */
int read_from_sock(int socket) {
	struct in_addr addr;
	int cnt;
	char ip_hdr_size = 0;
	char message[128];
	cnt = read(socket, (void *) message, 64);
	if (cnt < 0) {
		std::cerr << "read returned -1 " << std::endl;
		return -1;
	} else if (cnt == 0) {
		std::cerr << "read returned 0 " << std::endl;
		return -1;
	}
	ip_hdr_size = message[0] & 0xf;
	printf("IP version: %d, header size: %d\n", message[0] >> 4, ip_hdr_size);
	addr.s_addr = *((int *) (message + 12));
	printf("SRC IP: %s, ", inet_ntoa(addr));
	addr.s_addr = *((int *) (message + 16));
	printf("DST IP: %s\n", inet_ntoa(addr));
	return 0;
}

/*
 * @brief It will try to read from socket and to print general IP packet data, such as src ip.
 * NB! It **will** wait for data on socket.
 * @param fd — socket file descriptor.
 */
void listen(int sock) {
	struct sockaddr_in addr;
	struct in_addr addr2;
	int addrlen = sizeof(addr);
	int cnt;
	char ip_hdr_size = 0;
	char message[128];
	std::cerr << "Listening" << std::endl;
	while (1) {
		cnt = recvfrom(sock, (void *) message, sizeof(message), 0,
				(struct sockaddr *) &addr, (socklen_t *) &addrlen);
		if (cnt < 0) {
			std::cerr << "recvfrom" << std::endl;
			break;
		} else if (cnt == 0) {
			break;
		}
		ip_hdr_size = message[0] & 0xf;
		printf("\n\n%s: size = %d\n", inet_ntoa(addr.sin_addr), cnt);
		printf("IP version: %d, header size: %d\n", message[0] >> 4,
				(message[0] & 0xf) * 4);
		addr2.s_addr = *((int *) (message + 12));
		printf("SRC IP: %s, ", inet_ntoa(addr2));
		addr2.s_addr = *((int *) (message + 16));
		printf("DST IP: %s\n", inet_ntoa(addr2));
		for (int i = ip_hdr_size * 4; i < cnt; i++) {
			printf("0x%0X ", message[i]);
		}
	}
}

/*
 * @brief Socket creation, creates socket and sets socket options.
 * @param ifname — interface for socket binding
 * @param interface_ip — interface IP address
 * @return socket file descriptor on success or -1 on error.
 */
int make_socket(const char *ifname, const char *interface_ip) {
	constexpr int tos = 0xc0;
	constexpr int state = 1;
	constexpr int size = 512 * 1024;
	int ttl = 42;
	int sock = -1;
	int ret = -1;
	struct ip_mreq mreq;

	sock = socket(AF_INET, SOCK_RAW, ipproto_ospfigp);
	if (sock < 0) {
		perror("socket(AF_INET, SOCK_RAW, ipproto_ospfigp)");
		return sock;
	}

	if ((setsockopt(sock, IPPROTO_IP, IP_TOS, &tos, sizeof(tos))) < 0) {
		perror("setsockopt IP_TOS");
		close(sock);
		return -1;
	}

	ttl = 1;
	ret = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
	if (ret < 0) {
		perror("setsockopt IP_MULTICAST_TTL");
		close(sock);
		return -1;
	}
	ttl = 255;
	ret = setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
	if (ret < 0) {
		perror("setsockopt IP_TTL");
		close(sock);
		return -1;
	}
	ret = setsockopt(sock, IPPROTO_IP, IP_PKTINFO, &state, sizeof(state));
	if (ret < 0) {
		perror("setsockopt IP_PKTINFO");
		close(sock);
		return -1;
	}

	setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, ifname, strlen(ifname) + 1);
	if (ret < 0) {
		perror("setsockopt SO_BINDTODEVICE");
		close(sock);
		return -1;
	}

	ret = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
	if (ret < 0) {
		perror("setsockopt SO_RCVBUF");
		close(sock);
		return -1;
	}
	ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
	if (ret < 0) {
		perror("setsockopt SO_SNDBUF");
		close(sock);
		return -1;
	}

	mreq.imr_multiaddr.s_addr = htonl(ospfv2_allspfrouters);
	mreq.imr_interface.s_addr = inet_addr(interface_ip);
	ret = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
	if (ret < 0) {
		perror("setsockopt IP_ADD_MEMBERSHIP");
	}

	return sock;
}

