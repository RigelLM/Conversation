#pragma once

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #endif

  #include <winsock2.h>
  #include <ws2tcpip.h>

  using SocketHandle = SOCKET;
  constexpr SocketHandle kInvalidSocket = INVALID_SOCKET;

  inline bool NetInit() {
      WSADATA wsa{};
      return WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
  }

  inline void NetShutdown() { WSACleanup(); }

  inline int NetClose(SocketHandle s) { return closesocket(s); }

  inline int NetLastError() { return WSAGetLastError(); }

#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <errno.h>

  using SocketHandle = int;
  constexpr SocketHandle kInvalidSocket = -1;

  inline bool NetInit() { return true; }

  inline void NetShutdown() {}

  inline int NetClose(SocketHandle s) { return close(s); }

  inline int NetLastError() { return errno; }
#endif
