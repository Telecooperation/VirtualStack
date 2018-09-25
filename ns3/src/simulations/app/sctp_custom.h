#pragma once

/*
 * 5.2.2 SCTP Header Information Structure (SCTP_SNDRCV)
 *
 *   This cmsghdr structure specifies SCTP options for sendmsg() and
 *   describes SCTP header information about a received message through
 *   recvmsg().
 *
 *   cmsg_level    cmsg_type      cmsg_data[]
 *   ------------  ------------   ----------------------
 *   IPPROTO_SCTP  SCTP_SNDRCV    struct sctp_sndrcvinfo
 *
 */
#include <stdint.h>

#define SCTP_UNORDERED 1
#define SOL_SCTP	132
#define SCTP_NODELAY	3
#define SCTP_DISABLE_FRAGMENTS	8
#define SCTP_DEFAULT_SEND_PARAM	10

typedef __s32 sctp_assoc_t;

struct sctp_sndrcvinfo {
    __u16 sinfo_stream;
    __u16 sinfo_ssn;
    __u16 sinfo_flags;
    __u32 sinfo_ppid;
    __u32 sinfo_context;
    __u32 sinfo_timetolive;
    __u32 sinfo_tsn;
    __u32 sinfo_cumtsn;
    sctp_assoc_t sinfo_assoc_id;
};
