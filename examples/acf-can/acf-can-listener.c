/*
 * Copyright (c) 2024, COVESA
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of COVESA nor the names of its contributors may be
 *      used to endorse or promote products derived from this software without
 *      specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <argp.h>
#include <poll.h>
#include <stdlib.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/ioctl.h>

#include "common/common.h"
#include "avtp/Udp.h"
#include "avtp/acf/Ntscf.h"
#include "avtp/acf/Tscf.h"
#include "avtp/acf/AcfCommon.h"
#include "avtp/acf/Can.h"
#include "avtp/CommonHeader.h"
#include "acf-can-common.h"

#define MAX_PDU_SIZE                1500
#define ARGPARSE_CAN_FD_OPTION      500

static char ifname[IFNAMSIZ];
static uint8_t macaddr[ETH_ALEN];
static uint8_t use_udp;
static uint32_t udp_port = 17220;
static Avtp_CanVariant_t can_variant = AVTP_CAN_CLASSIC;
static char can_ifname[IFNAMSIZ];

static char doc[] = "\nacf-can-listener -- a program designed to receive CAN messages from \
                    a remote CAN bus over Ethernet using Open1722 \
                    \vEXAMPLES\
                    \n\n  acf-can-listener eth0 aa:bb:cc:dd:ee:ff can1\
                    \n\n    (tunnel Open1722 CAN messages received from eth0 to STDOUT)\
                    \n\n  acf-can-listener can1 -up 1722\
                    \n\n    (tunnel Open1722 CAN messages received over UDP from port 1722 to can1)\
                    \n\n  acf-can-listener -up 1722 | canplayer can1=elmcan\
                    \n\n    (another method to tunnel Open1722 CAN messages to can1)";

static char args_doc[] = "[ifname] dst-mac-address [can ifname]";

static struct argp_option options[] = {
    {"port", 'p', "UDP_PORT", 0, "UDP Port to listen on if UDP enabled"},
    {"udp", 'u', 0, 0, "Use UDP"},
    {"fd", ARGPARSE_CAN_FD_OPTION, 0, 0, "Use CAN-FD"},
    {"can ifname", 0, 0, OPTION_DOC, "CAN interface (set to STDOUT by default)"},
    {"dst-mac-address", 0, 0, OPTION_DOC, "Stream destination MAC address (If Ethernet)"},
    {"ifname", 0, 0, OPTION_DOC, "Network interface (If Ethernet)" },
    { 0 }
};

static error_t parser(int key, char *arg, struct argp_state *state)
{
    int res;

    switch (key) {
    case 'p':
        udp_port = atoi(arg);
        break;
    case 'u':
        use_udp = 1;
        break;
    case ARGPARSE_CAN_FD_OPTION:
        can_variant = AVTP_CAN_FD;

    case ARGP_KEY_NO_ARGS:
        break;

    case ARGP_KEY_ARG:

        if(state->argc < 2){
            argp_usage(state);
        }

        if(!use_udp){
            strncpy(ifname, arg, sizeof(ifname) - 1);

            if(state->next < state->argc)
            {
                res = sscanf(state->argv[state->next], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                        &macaddr[0], &macaddr[1], &macaddr[2],
                        &macaddr[3], &macaddr[4], &macaddr[5]);
                if (res != 6) {
                    fprintf(stderr, "Invalid MAC address\n\n");
                    argp_usage(state);
                }
                state->next += 1;
            }

            if(state->next < state->argc)
            {
                strncpy(can_ifname, state->argv[state->next], sizeof(can_ifname) - 1);
                state->next = state->argc;
            }

        }else{
            strncpy(can_ifname, arg, sizeof(can_ifname) - 1);
            state->next = state->argc;
        }

        break;
    }

    return 0;
}

static struct argp argp = { options, parser, args_doc, doc };

static int is_valid_acf_packet(uint8_t* acf_pdu)
{
    Avtp_AcfCommon_t *pdu = (Avtp_AcfCommon_t*) acf_pdu;
    uint8_t acf_msg_type = Avtp_AcfCommon_GetAcfMsgType(pdu);
    if (acf_msg_type != AVTP_ACF_TYPE_CAN) {
        return 0;
    }

    return 1;
}

static int new_packet(int sk_fd, int can_socket) {

    int res = 0;
    uint64_t proc_bytes = 0, msg_proc_bytes = 0;
    uint32_t udp_seq_num;
    uint16_t msg_length, can_payload_length, acf_msg_length;
    uint8_t subtype;
    uint8_t pdu[MAX_PDU_SIZE], i;
    uint8_t *cf_pdu, *acf_pdu, *udp_pdu, *can_payload;
    frame_t frame;
    canid_t can_id;

    memset(&frame, 0, sizeof(struct canfd_frame));
    res = recv(sk_fd, pdu, MAX_PDU_SIZE, 0);
    if (res < 0 || res > MAX_PDU_SIZE) {
        perror("Failed to receive data");
        return 0;
    }

    if (use_udp) {
        udp_pdu = pdu;
        udp_seq_num = Avtp_Udp_GetEncapsulationSeqNo((Avtp_Udp_t *)udp_pdu);
        cf_pdu = pdu + AVTP_UDP_HEADER_LEN;
        proc_bytes += AVTP_UDP_HEADER_LEN;
    } else {
        cf_pdu = pdu;
    }

    subtype = Avtp_CommonHeader_GetSubtype((Avtp_CommonHeader_t*)cf_pdu);
    if (!((subtype == AVTP_SUBTYPE_NTSCF) ||
        (subtype == AVTP_SUBTYPE_TSCF))) {
        return 0;
    }

    if (subtype == AVTP_SUBTYPE_TSCF){
        proc_bytes += AVTP_TSCF_HEADER_LEN;
        msg_length = Avtp_Tscf_GetStreamDataLength((Avtp_Tscf_t*)cf_pdu);
    } else {
        proc_bytes += AVTP_NTSCF_HEADER_LEN;
        msg_length = Avtp_Ntscf_GetNtscfDataLength((Avtp_Ntscf_t*)cf_pdu);
    }

    while (msg_proc_bytes < msg_length) {

        acf_pdu = &pdu[proc_bytes + msg_proc_bytes];

        if (!is_valid_acf_packet(acf_pdu)) {
            return 0;
        }

        can_id = Avtp_Can_GetCanIdentifier((Avtp_Can_t*)acf_pdu);

        can_payload = Avtp_Can_GetPayload((Avtp_Can_t*)acf_pdu);
        acf_msg_length = Avtp_Can_GetAcfMsgLength((Avtp_Can_t*)acf_pdu)*4;
        can_payload_length = Avtp_Can_GetCanPayloadLength((Avtp_Can_t*)acf_pdu);
        msg_proc_bytes += acf_msg_length;

        // Handle EFF Flag
        if (Avtp_Can_GetEff((Avtp_Can_t*)acf_pdu)) {
            can_id |= CAN_EFF_FLAG;
        } else if (can_id > 0x7FF) {
            fprintf(stderr, "Error: CAN ID is > 0x7FF but the EFF bit is not set.\n");
            return 0;
        }

        // Handle RTR Flag
        if (Avtp_Can_GetRtr((Avtp_Can_t*)acf_pdu)) {
            can_id |= CAN_RTR_FLAG;
        }

        if (can_variant == AVTP_CAN_FD) {
            if (Avtp_Can_GetBrs((Avtp_Can_t*)acf_pdu)) {
                frame.fd.flags |= CANFD_BRS;
            }
            if (Avtp_Can_GetFdf((Avtp_Can_t*)acf_pdu)) {
                frame.fd.flags |= CANFD_FDF;
            }
            if (Avtp_Can_GetEsi((Avtp_Can_t*)acf_pdu)) {
                frame.fd.flags |= CANFD_ESI;
            }
            frame.fd.can_id = can_id;
            frame.fd.len = can_payload_length;
            memcpy(frame.fd.data, can_payload, can_payload_length);
            res = write(can_socket, &frame.fd, sizeof(struct canfd_frame));
        } else {
            frame.cc.can_id = can_id;
            frame.cc.len = can_payload_length;
            memcpy(frame.cc.data, can_payload, can_payload_length);
            res = write(can_socket, &frame.cc, sizeof(struct can_frame));
        }

        if(res < 0)
        {
            perror("Failed to write to CAN bus");
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[])
{
    int fd, res;
    struct pollfd fds;

    int can_socket = 0;
    struct sockaddr_can can_addr;
    struct ifreq ifr;

    argp_parse(&argp, argc, argv, 0, NULL, NULL);

    // Configure an appropriate socket: UDP or Ethernet Raw
    if (use_udp) {
        fd = create_listener_socket_udp(udp_port);
    } else {
        fd = create_listener_socket(ifname, macaddr, ETH_P_TSN);
    }
    fds.fd = fd;
    fds.events = POLLIN;

    if (fd < 0)
        return 1;

    // Open a CAN socket for reading frames
    can_socket = setup_can_socket(can_ifname, can_variant);
    if (!can_socket) goto err;

    while (1) {

        res = poll(&fds, 1, -1);
        if (res < 0) {
            perror("Failed to poll() fds");
            goto err;
        }

        if (fds.revents & POLLIN) {
            res = new_packet(fd, can_socket);
            if (res < 0)
                goto err;
        }

    }

    return 0;

err:
    close(fd);
    return 1;

}
