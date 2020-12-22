/* (c) 2020 KAI OS TECHNOLOGIES (HONG KONG) LIMITED All rights reserved. This
 * file or any portion thereof may not be reproduced or used in any manner
 * whatsoever without the express written permission of KAI OS TECHNOLOGIES
 * (HONG KONG) LIMITED. KaiOS is the trademark of KAI OS TECHNOLOGIES (HONG
 * KONG) LIMITED or its affiliate company and may be registered in some
 * jurisdictions. All other trademarks are the property of their respective
 * owners.
 */

#include <binder/Parcel.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <utils/String8.h>

#include "GfxDebugger_defs.h"

#define LOGE(format, ...)   fprintf(stderr, "E GD: " format "\n", ##__VA_ARGS__)
#define LOGD(format, ...)   fprintf(stdout, "D GD: " format "\n", ##__VA_ARGS__)

#define GD_SOCKET_NAME  "/dev/socket/gfxdebugger-ipc"
#define ARRAY_SIZE(x) ( sizeof(x) / sizeof((x)[0]) )

using android::Parcel;
using android::String8;

int sock;
int init_client()
{
  socklen_t addrLength;
  struct sockaddr_un address;

  if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
    LOGE("socket error: %s", strerror(errno));
    return -1;
  }

  address.sun_family = AF_UNIX;
  strcpy(address.sun_path, GD_SOCKET_NAME);
  address.sun_path[strlen(GD_SOCKET_NAME)] = 0;
  addrLength = sizeof(address.sun_family) +
    strlen(address.sun_path) + 1;

  if (connect(sock, (struct sockaddr *) &address, addrLength)) {
    LOGE("connect error: %s", strerror(errno));
    return -1;
  }

  return 0;
}

unsigned get_response() {
  Parcel parcel;
  fd_set read_fds;
  uint8_t buffer[128];

  FD_SET(sock, &read_fds);

  select(sock + 1, &read_fds, NULL, NULL, NULL);
  memset(buffer, 0, sizeof(buffer));
  uint32_t size, res = 0;
  if (FD_ISSET(sock, &read_fds)) {
    recv(sock, buffer, sizeof(buffer), 0);
    parcel.setData(buffer, 8);
    size = parcel.readUint32();
    res = parcel.readUint32();
  }

  return res;
}

void end_client()
{
  close(sock);
}

/*---------------------------------------------------------------------------*/

int cmd_gralloc(int argc, char **argv) {
  LOGD("cmd_gralloc+, argc=%d, argv[3]=%s", argc, argv[3]);
  Parcel parcel;
  parcel.writeUint32(sizeof(uint32_t));
  parcel.writeUint32(GD_CMD_GRALLOC);

  int c = 0;
  while ((c = getopt(argc, argv, "d:l")) != -1) {
    LOGD("[gralloc] option=%c, arg=%s", c, optarg);
    switch (c) {
      case 'd': {
        if (optarg) {
          parcel.writeUint32(GRALLOC_OP_DUMP);
          unsigned index = strtoul(optarg, NULL, 10);
          parcel.writeUint32(index);
          LOGD("dump gralloc[%d]", index);

          write(sock, parcel.data(), parcel.dataSize());
          int rc;
          fd_set read_fds;
          uint8_t buffer[PATH_MAX];
          memset(buffer, 0, sizeof(buffer));

          FD_SET(sock, &read_fds);
          rc = select(sock + 1, &read_fds, NULL, NULL, NULL);
          if (FD_ISSET(sock, &read_fds)) {
            size_t recv_size = recv(sock, buffer, sizeof(buffer), 0);
            LOGD("received %zu bytes", recv_size);
            Parcel p2;
            p2.setData(buffer, recv_size);
            LOGD("parcel size: %zu\n", p2.dataSize());
            LOGD("output file: %s\n", p2.readCString());
          }
        } else {
          LOGE("[gralloc] value for option d is null");
        }
        break;
      }

      case 'l': {
        parcel.writeUint32(GRALLOC_OP_LIST);
        write(sock, parcel.data(), parcel.dataSize());

        int rc;
        fd_set read_fds;
        uint8_t buffer[PATH_MAX];
        memset(buffer, 0, sizeof(buffer));

        FD_SET(sock, &read_fds);
        rc = select(sock + 1, &read_fds, NULL, NULL, NULL);
        if (FD_ISSET(sock, &read_fds)) {
          size_t recv_size = recv(sock, buffer, sizeof(buffer), 0);
          LOGD("received %zu bytes", recv_size);
          Parcel reply;
          reply.setData(buffer, recv_size);
          LOGD("parcel size: %zu\n", reply.dataSize());

          uint_t gb_amount = reply.readUint32();
          LOGD("gb_amount: %u\n", gb_amount);
          for (int i = 0; i < gb_amount; i++) {
              LOGD("gralloc: index=%u", reply.readUint32());
          }
        }
        break;
      }

      default:
        LOGE("Invalid option: -%c\n", optopt);
        break;
    }
  }

  return 0;
}

/*---------------------------------------------------------------------------*/

int cmd_screencap(int argc, char **argv) {
  LOGD("cmd_screencap+, argc=%d, argv[3]=%s", argc, argv[3]);
  Parcel parcel;
  parcel.writeUint32(sizeof(uint32_t));
  parcel.writeUint32(GD_CMD_SCREENCAP);

  int c = 0;
  unsigned displayId = 0;
  while ((c = getopt(argc, argv, "d:p:")) != -1) {
    LOGD("[screencap] option=%c, arg=%s", c, optarg);
    switch (c) {
      case 'd': {
        if (optarg) {
          displayId = strtoul(optarg, NULL, 10);
        } else {
          LOGE("[screencap] value for option d is null");
        }
        break;
      }

      case 'p': {
        if (optarg) {
          parcel.writeUint32(SCREENCAP_OP_CAPTURE);
          parcel.writeUint32(displayId);
          parcel.writeCString(optarg);
          LOGD("screencap[%s]", optarg);

          write(sock, parcel.data(), parcel.dataSize());
          int rc;
          fd_set read_fds;
          uint8_t buffer[PATH_MAX];
          memset(buffer, 0, sizeof(buffer));

          FD_SET(sock, &read_fds);
          rc = select(sock + 1, &read_fds, NULL, NULL, NULL);
          if (FD_ISSET(sock, &read_fds)) {
            size_t recv_size = recv(sock, buffer, sizeof(buffer), 0);
            LOGD("received %zu bytes", recv_size);
            Parcel p2;
            p2.setData(buffer, recv_size);
            LOGD("parcel size: %zu\n", p2.dataSize());
            LOGD("screencap result: %u\n", p2.readUint32());
          }
        } else {
          LOGE("[screencap] value for option p is null");
        }
        break;
      }

      default:
        LOGE("Invalid option: -%c\n", optopt);
        break;
    }
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
struct main_cmd {
  const char *name;
  int (*func)(int, char **);
} main_cmds[] =   {
  { "gralloc",   cmd_gralloc },
  { "screencap", cmd_screencap },
  //{ "layer", 		 cmd_layer },
  //{ "apz", 		   cmd_apz },
};

void usage()
{
  printf(
    "usage: \tgfxdebugger [OPTION]\n"
    "  -c gralloc\t-l\t\t"    "list graloc buffers\n"
    "  -c gralloc\t-d\tNUM\t" "dump graloc buffers with given buffer id: NUM\n"
    "  -c screencap\t[-d\tNUM]\t-p\tPATH\t" "capture [0/1:Primary/External]"
                                            "screen and save to specific PATH\n"
  );
}

int main(int argc, char **argv)
{
  if (argc <= 1) {
    usage();
    exit(-1);
  }

  init_client();

  int opt = getopt(argc, argv, "c:");
  if (opt == 'c') {
    unsigned i;
    for (i = 0; i < ARRAY_SIZE(main_cmds); i++) {
      LOGD("comparing argv[1]=%s and cmd[%d]=%s",
        argv[1], i, main_cmds[i].name);
      if (!strcmp(main_cmds[i].name, optarg)) {
        (main_cmds[i].func)(argc, argv);
        break;
      }
    }

    if (i == ARRAY_SIZE(main_cmds)) {
      LOGE("Unknow command: %s\n", argv[2]);
      usage();
      exit(-1);
    }
  }

  end_client();
  return 0;
}
