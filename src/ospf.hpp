/*
 * ospf.hpp
 *
 *  Created on: 12 сент. 2018 г.
 *      Author: ringo
 */

#ifndef OSPF_HPP_
#define OSPF_HPP_
#include <netinet/in.h>
#include <arpa/inet.h>

/* OSPF protocol number. */
constexpr int ipproto_ospfigp = 89;
constexpr int ospfv2_allspfrouters = 0xE0000005;

#endif /* OSPF_HPP_ */
