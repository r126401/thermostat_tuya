/**
 * @file ble_trsmitr.C
 * @brief Bluetooth Low Energy (BLE) data transmission module.
 * This module provides functions to create and manage BLE data transmission,
 * including encoding and decoding BLE packets for communication between
 * devices.
 *
 * @copyright Copyright (c) 2021-2024 Tuya Inc. All Rights Reserved.
 *
 */

#include "tal_api.h"
#include "ble_trsmitr.h"
#include "ble_mgr.h"
#include "ble_dp.h"

#define __MUTLI_TSF_PROTOCOL_GLOBALS

/***********************************************************
*************************micro define***********************
***********************************************************/

/***********************************************************
*************************variable define********************
***********************************************************/
static ble_frame_seq_t s_ble_frame_seq = 0;
static uint16_t s_ble_frame_packet_len = 1024;

/***********************************************************
*************************function define********************
***********************************************************/
/**
 * @brief Creates a new instance of the ble_frame_trsmitr_t structure.
 *
 * This function allocates memory for the ble_frame_trsmitr_t structure and
 * initializes its members. It also allocates memory for the subpkg member based
 * on the value returned by ble_frame_packet_len_get().
 *
 * @return A pointer to the newly created ble_frame_trsmitr_t structure, or NULL
 * if memory allocation fails.
 */
ble_frame_trsmitr_t *ble_frame_trsmitr_create(void)
{
    ble_frame_trsmitr_t *trsmitr = (ble_frame_trsmitr_t *)tal_malloc(sizeof(ble_frame_trsmitr_t));
    if (NULL == trsmitr) {
        PR_DEBUG("malloc err");
        return NULL;
    }
    memset(trsmitr, 0, sizeof(ble_frame_trsmitr_t));
    trsmitr->subpkg = (uint8_t *)tal_malloc(ble_frame_packet_len_get());
    if (trsmitr->subpkg == NULL) {
        PR_ERR("malloc err:%d", ble_frame_packet_len_get());
        tal_free(trsmitr);
        return NULL;
    }
    memset(trsmitr->subpkg, 0, ble_frame_packet_len_get());

    return trsmitr;
}

/**
 * @brief Deletes a BLE frame transmitter.
 *
 * This function frees the memory allocated for a BLE frame transmitter and its
 * subpackage.
 *
 * @param trsmitr Pointer to the BLE frame transmitter to be deleted.
 */
void ble_frame_trsmitr_delete(ble_frame_trsmitr_t *trsmitr)
{
    tal_free(trsmitr->subpkg);
    tal_free(trsmitr);
}

/**
 * @brief Get the length of the subpacket in a BLE frame transmitter.
 *
 * This function returns the length of the subpacket in a BLE frame transmitter.
 *
 * @param trsmitr The BLE frame transmitter.
 * @return The length of the subpacket.
 */
ble_frame_subpkg_len_t ble_frame_subpacket_len_get(ble_frame_trsmitr_t *trsmitr)
{
    return trsmitr->subpkg_len;
}

/**
 * @brief Get the length of the BLE frame packet.
 *
 * This function returns the length of the BLE frame packet.
 *
 * @return The length of the BLE frame packet.
 */
uint16_t ble_frame_packet_len_get(void)
{
    return s_ble_frame_packet_len;
}

/**
 * @brief Sets the length of the BLE frame packet.
 *
 * This function sets the length of the BLE frame packet to the specified value.
 *
 * @param len The length of the BLE frame packet.
 */
void ble_frame_packet_len_set(uint16_t len)
{
    s_ble_frame_packet_len = len;
    PR_DEBUG("ble sub packet lenth set:%d", s_ble_frame_packet_len);
}

/**
 * @brief Retrieves the subpacket from the given BLE frame transmitter.
 *
 * This function returns a pointer to the subpacket data stored in the specified
 * BLE frame transmitter.
 *
 * @param trsmitr The BLE frame transmitter from which to retrieve the
 * subpacket.
 * @return A pointer to the subpacket data.
 */
unsigned char *ble_frame_subpacket_get(ble_frame_trsmitr_t *trsmitr)
{
    return trsmitr->subpkg;
}

static ble_frame_seq_t ble_frame_seq_get(void)
{
    return (s_ble_frame_seq >= BLE_FRAME_SEQ_LMT) ? 0 : s_ble_frame_seq++;
}

/**
 * @brief Encodes and sends a package over BLE.
 *
 * This function encodes and sends a package over BLE. It takes the version,
 * buffer, and length of the package as input parameters. The function also
 * updates the package descriptor, subpackage number, and package transmission
 * count.
 *
 * @param trsmitr Pointer to the ble_frame_trsmitr_t structure.
 * @param version The version of the package.
 * @param buf Pointer to the buffer containing the package data.
 * @param len The length of the package data.
 * @return Returns OPRT_INVALID_PARM if trsmitr is NULL, OPRT_COM_ERROR if the
 * subpackage number or length exceeds the limit,
 *         OPRT_SVC_BT_API_TRSMITR_CONTINUE if there are more subpackages to
 * send, or OPRT_OK if the package transmission is complete.
 */
int ble_frame_trsmitr_send_pkg_encode(ble_frame_trsmitr_t *trsmitr, unsigned char version, unsigned char *buf,
                                      unsigned int len)
{
    if (((void *)0) == trsmitr) {
        return OPRT_INVALID_PARM;
    }

    if (BLE_FRAME_PKG_INIT == trsmitr->pkg_desc) {
        trsmitr->total = len;
        trsmitr->version = version;
        trsmitr->seq = ble_frame_seq_get();
        trsmitr->subpkg_num = 0;
        trsmitr->pkg_trsmitr_cnt = 0;
    }

    if (trsmitr->subpkg_num >= 0x10000000 || len >= 0x10000000) {
        return OPRT_COM_ERROR;
    }

    unsigned char sunpkg_offset = 0;

    // package code
    // subpackage num encode
    int i;
    unsigned int tmp = 0;
    tmp = trsmitr->subpkg_num;
    for (i = 0; i < 4; i++) {
        trsmitr->subpkg[sunpkg_offset] = tmp % 0x80;
        if ((tmp / 0x80)) {
            trsmitr->subpkg[sunpkg_offset] |= 0x80;
        }
        sunpkg_offset++;
        tmp /= 0x80;
        if (0 == tmp) {
            break;
        }
    }

    // the first package include the frame total len
    if (0 == trsmitr->subpkg_num) {
        // frame len encode
        tmp = len;
        for (i = 0; i < 4; i++) {
            trsmitr->subpkg[sunpkg_offset] = tmp % 0x80;
            if ((tmp / 0x80)) {
                trsmitr->subpkg[sunpkg_offset] |= 0x80;
            }
            sunpkg_offset++;
            tmp /= 0x80;
            if (0 == tmp) {
                break;
            }
        }

        // frame type and frame seq
        trsmitr->subpkg[sunpkg_offset++] = (trsmitr->version << 0x04) | (trsmitr->seq & 0x0f);
    }

    // frame data transfer
    uint16_t send_data = (ble_frame_packet_len_get() - sunpkg_offset);
    if ((len - trsmitr->pkg_trsmitr_cnt) < send_data) {
        send_data = len - trsmitr->pkg_trsmitr_cnt;
    }

    PR_TRACE("pkg max len:%d, sunpkg_offset:%d, send_data:%d", ble_frame_packet_len_get(), sunpkg_offset, send_data);

    memcpy(&(trsmitr->subpkg[sunpkg_offset]), buf + trsmitr->pkg_trsmitr_cnt, send_data);
    trsmitr->subpkg_len = sunpkg_offset + send_data;

    trsmitr->pkg_trsmitr_cnt += send_data;
    if (0 == trsmitr->subpkg_num) {
        trsmitr->pkg_desc = BLE_FRAME_PKG_FIRST;
    } else {
        trsmitr->pkg_desc = BLE_FRAME_PKG_MIDDLE;
    }

    if (trsmitr->pkg_trsmitr_cnt < trsmitr->total) {
        trsmitr->subpkg_num++;
        return OPRT_SVC_BT_API_TRSMITR_CONTINUE;
    }

    trsmitr->pkg_desc = BLE_FRAME_PKG_END;
    return OPRT_OK;
}

/**
 * @brief Decodes the received package and updates the ble_frame_trsmitr_t
 * structure.
 *
 * This function decodes the received package data and updates the fields of the
 * ble_frame_trsmitr_t structure. It checks for invalid parameters and validates
 * the subpackage number and package description. It also decodes the frame
 * length, frame type, and frame sequence. Finally, it copies the decoded data
 * to the transmitter subpackage buffer and updates the package transmit count.
 *
 * @param trsmitr Pointer to the ble_frame_trsmitr_t structure.
 * @param raw_data Pointer to the raw data of the received package.
 * @param raw_data_len Length of the raw data.
 * @return Returns the operation result status.
 *     - OPRT_INVALID_PARM: If the raw_data or trsmitr pointer is NULL, or if
 * the raw_data_len is greater than the maximum packet length.
 *     - OPRT_COM_ERROR: If the subpackage number exceeds the maximum value.
 *     - OPRT_SVC_BT_API_TRSMITR_ERROR: If the received subpackage number is
 * less than the current subpackage number.
 *     - OPRT_SVC_BT_API_TRSMITR_CONTINUE: If the received subpackage number is
 * the same as the current subpackage number.
 *     - OPRT_OK: If the decoding and updating process is successful.
 */
int ble_frame_trsmitr_recv_pkg_decode(ble_frame_trsmitr_t *trsmitr, unsigned char *raw_data, uint16_t raw_data_len)
{
    if (NULL == raw_data || NULL == trsmitr) { //|| (raw_data_len > ble_frame_packet_len_get())
        return OPRT_INVALID_PARM;
    }

    if (BLE_FRAME_PKG_INIT == trsmitr->pkg_desc) {
        trsmitr->total = 0;
        trsmitr->version = 0;
        trsmitr->seq = 0;
        trsmitr->pkg_trsmitr_cnt = 0;
    }

    unsigned char sunpkg_offset = 0;
    // package code
    // subpackage num decode
    int i;
    unsigned int multiplier = 1;
    unsigned char digit;
    ble_frame_subpkg_num_t subpkg_num = 0;
    // Package number
    for (i = 0; i < 4; i++) {
        digit = raw_data[sunpkg_offset++];
        subpkg_num += (digit & 0x7f) * multiplier;
        multiplier *= 0x80;

        if (0 == (digit & 0x80)) {
            break;
        }
    }
    // PR_DEBUG("subpkg_num:%d, trsmitr->subpkg_num:%d", subpkg_num,
    // trsmitr->subpkg_num);
    if (0 == subpkg_num) {
        trsmitr->total = 0;
        trsmitr->version = 0;
        trsmitr->seq = 0;
        trsmitr->pkg_trsmitr_cnt = 0;
        trsmitr->pkg_desc = BLE_FRAME_PKG_FIRST;
    } else {
        trsmitr->pkg_desc = BLE_FRAME_PKG_MIDDLE;
    }

    if (trsmitr->subpkg_num >= 0x10000000) {
        return OPRT_COM_ERROR;
    }
    // is receive the subpackage num valid?
    if (trsmitr->pkg_desc != BLE_FRAME_PKG_FIRST) {
        if (subpkg_num < trsmitr->subpkg_num) {
            return OPRT_SVC_BT_API_TRSMITR_ERROR;
        } else if (subpkg_num == trsmitr->subpkg_num) {
            return OPRT_SVC_BT_API_TRSMITR_CONTINUE;
        }

        if (subpkg_num - trsmitr->subpkg_num > 1) {
            return OPRT_SVC_BT_API_TRSMITR_ERROR;
        }
    }
    trsmitr->subpkg_num = subpkg_num;

    if (0 == trsmitr->subpkg_num) {
        // frame len decode
        multiplier = 1;
        for (i = 0; i < 4; i++) {
            digit = raw_data[sunpkg_offset++];
            trsmitr->total += (digit & 0x7f) * multiplier;
            multiplier *= 0x80;

            if (0 == (digit & 0x80)) {
                break;
            }
        }

        if (trsmitr->total >= 0x10000000) {
            return OPRT_COM_ERROR;
        }

        // frame type and frame seq decode
        trsmitr->version = (raw_data[sunpkg_offset] & BLE_FRAME_VERSION_OFFSET) >> 4;
        trsmitr->seq = raw_data[sunpkg_offset++] & BLE_FRAME_SEQ_OFFSET;
    }

    uint16_t recv_data = raw_data_len - sunpkg_offset;
    if ((trsmitr->total - trsmitr->pkg_trsmitr_cnt) < recv_data) {
        recv_data = trsmitr->total - trsmitr->pkg_trsmitr_cnt;
    }

    // decode data cp to transmitter subpackage buf
    memcpy(trsmitr->subpkg, &raw_data[sunpkg_offset], recv_data);
    trsmitr->subpkg_len = recv_data;
    trsmitr->pkg_trsmitr_cnt += recv_data;

    if (trsmitr->pkg_trsmitr_cnt < trsmitr->total) {
        return OPRT_SVC_BT_API_TRSMITR_CONTINUE;
    }
    trsmitr->pkg_desc = BLE_FRAME_PKG_END;

    return OPRT_OK;
}
