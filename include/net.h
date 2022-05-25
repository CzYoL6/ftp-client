#pragma once

#ifndef _WIN32
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#define _socket int
#define _invalid_socket -1
#define _sock_error -1

#define _sock_init( )
#define _sock_exit( )
#define _sock_err( )	errno
#define _sock_close( sockfd ) ::close( sockfd )

#else

#include <Ws2tcpip.h>

#define _socket SOCKET
#define _invalid_socket INVALID_SOCKET
#define _sock_error SOCKET_ERROR

#define _sock_init( )	{ WSADATA wsaData; WSAStartup( MAKEWORD(2, 2), &wsaData ); }
#define _sock_exit( )	{ WSACleanup(); }
#define _sock_err( )	WSAGetLastError()
#define _sock_close( sockfd ) ::closesocket( sockfd )

#endif