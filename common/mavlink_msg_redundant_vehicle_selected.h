#pragma once
// MESSAGE REDUNDANT_VEHICLE_SELECTED PACKING

#define MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED 451


typedef struct __mavlink_redundant_vehicle_selected_t {
 uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude of the number.*/
 uint8_t target_system; /*<  System ID*/
 uint8_t target_component; /*<  Component ID*/
 uint8_t redundant_px4_selected; /*<  The redundant autopilot selected after making the calculations.*/
} mavlink_redundant_vehicle_selected_t;

#define MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN 11
#define MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_MIN_LEN 11
#define MAVLINK_MSG_ID_451_LEN 11
#define MAVLINK_MSG_ID_451_MIN_LEN 11

#define MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_CRC 170
#define MAVLINK_MSG_ID_451_CRC 170



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_REDUNDANT_VEHICLE_SELECTED { \
    451, \
    "REDUNDANT_VEHICLE_SELECTED", \
    4, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_redundant_vehicle_selected_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 9, offsetof(mavlink_redundant_vehicle_selected_t, target_component) }, \
         { "time_usec", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_redundant_vehicle_selected_t, time_usec) }, \
         { "redundant_px4_selected", NULL, MAVLINK_TYPE_UINT8_T, 0, 10, offsetof(mavlink_redundant_vehicle_selected_t, redundant_px4_selected) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_REDUNDANT_VEHICLE_SELECTED { \
    "REDUNDANT_VEHICLE_SELECTED", \
    4, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_redundant_vehicle_selected_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 9, offsetof(mavlink_redundant_vehicle_selected_t, target_component) }, \
         { "time_usec", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_redundant_vehicle_selected_t, time_usec) }, \
         { "redundant_px4_selected", NULL, MAVLINK_TYPE_UINT8_T, 0, 10, offsetof(mavlink_redundant_vehicle_selected_t, redundant_px4_selected) }, \
         } \
}
#endif

/**
 * @brief Pack a redundant_vehicle_selected message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target_system  System ID
 * @param target_component  Component ID
 * @param time_usec [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude of the number.
 * @param redundant_px4_selected  The redundant autopilot selected after making the calculations.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_redundant_vehicle_selected_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t target_system, uint8_t target_component, uint64_t time_usec, uint8_t redundant_px4_selected)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN];
    _mav_put_uint64_t(buf, 0, time_usec);
    _mav_put_uint8_t(buf, 8, target_system);
    _mav_put_uint8_t(buf, 9, target_component);
    _mav_put_uint8_t(buf, 10, redundant_px4_selected);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN);
#else
    mavlink_redundant_vehicle_selected_t packet;
    packet.time_usec = time_usec;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.redundant_px4_selected = redundant_px4_selected;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_MIN_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_CRC);
}

/**
 * @brief Pack a redundant_vehicle_selected message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param target_system  System ID
 * @param target_component  Component ID
 * @param time_usec [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude of the number.
 * @param redundant_px4_selected  The redundant autopilot selected after making the calculations.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_redundant_vehicle_selected_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t target_system,uint8_t target_component,uint64_t time_usec,uint8_t redundant_px4_selected)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN];
    _mav_put_uint64_t(buf, 0, time_usec);
    _mav_put_uint8_t(buf, 8, target_system);
    _mav_put_uint8_t(buf, 9, target_component);
    _mav_put_uint8_t(buf, 10, redundant_px4_selected);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN);
#else
    mavlink_redundant_vehicle_selected_t packet;
    packet.time_usec = time_usec;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.redundant_px4_selected = redundant_px4_selected;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_MIN_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_CRC);
}

/**
 * @brief Encode a redundant_vehicle_selected struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param redundant_vehicle_selected C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_redundant_vehicle_selected_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_redundant_vehicle_selected_t* redundant_vehicle_selected)
{
    return mavlink_msg_redundant_vehicle_selected_pack(system_id, component_id, msg, redundant_vehicle_selected->target_system, redundant_vehicle_selected->target_component, redundant_vehicle_selected->time_usec, redundant_vehicle_selected->redundant_px4_selected);
}

/**
 * @brief Encode a redundant_vehicle_selected struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param redundant_vehicle_selected C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_redundant_vehicle_selected_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_redundant_vehicle_selected_t* redundant_vehicle_selected)
{
    return mavlink_msg_redundant_vehicle_selected_pack_chan(system_id, component_id, chan, msg, redundant_vehicle_selected->target_system, redundant_vehicle_selected->target_component, redundant_vehicle_selected->time_usec, redundant_vehicle_selected->redundant_px4_selected);
}

/**
 * @brief Send a redundant_vehicle_selected message
 * @param chan MAVLink channel to send the message
 *
 * @param target_system  System ID
 * @param target_component  Component ID
 * @param time_usec [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude of the number.
 * @param redundant_px4_selected  The redundant autopilot selected after making the calculations.
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_redundant_vehicle_selected_send(mavlink_channel_t chan, uint8_t target_system, uint8_t target_component, uint64_t time_usec, uint8_t redundant_px4_selected)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN];
    _mav_put_uint64_t(buf, 0, time_usec);
    _mav_put_uint8_t(buf, 8, target_system);
    _mav_put_uint8_t(buf, 9, target_component);
    _mav_put_uint8_t(buf, 10, redundant_px4_selected);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED, buf, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_MIN_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_CRC);
#else
    mavlink_redundant_vehicle_selected_t packet;
    packet.time_usec = time_usec;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.redundant_px4_selected = redundant_px4_selected;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED, (const char *)&packet, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_MIN_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_CRC);
#endif
}

/**
 * @brief Send a redundant_vehicle_selected message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_redundant_vehicle_selected_send_struct(mavlink_channel_t chan, const mavlink_redundant_vehicle_selected_t* redundant_vehicle_selected)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_redundant_vehicle_selected_send(chan, redundant_vehicle_selected->target_system, redundant_vehicle_selected->target_component, redundant_vehicle_selected->time_usec, redundant_vehicle_selected->redundant_px4_selected);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED, (const char *)redundant_vehicle_selected, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_MIN_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_CRC);
#endif
}

#if MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_redundant_vehicle_selected_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t target_system, uint8_t target_component, uint64_t time_usec, uint8_t redundant_px4_selected)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint64_t(buf, 0, time_usec);
    _mav_put_uint8_t(buf, 8, target_system);
    _mav_put_uint8_t(buf, 9, target_component);
    _mav_put_uint8_t(buf, 10, redundant_px4_selected);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED, buf, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_MIN_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_CRC);
#else
    mavlink_redundant_vehicle_selected_t *packet = (mavlink_redundant_vehicle_selected_t *)msgbuf;
    packet->time_usec = time_usec;
    packet->target_system = target_system;
    packet->target_component = target_component;
    packet->redundant_px4_selected = redundant_px4_selected;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED, (const char *)packet, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_MIN_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_CRC);
#endif
}
#endif

#endif

// MESSAGE REDUNDANT_VEHICLE_SELECTED UNPACKING


/**
 * @brief Get field target_system from redundant_vehicle_selected message
 *
 * @return  System ID
 */
static inline uint8_t mavlink_msg_redundant_vehicle_selected_get_target_system(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  8);
}

/**
 * @brief Get field target_component from redundant_vehicle_selected message
 *
 * @return  Component ID
 */
static inline uint8_t mavlink_msg_redundant_vehicle_selected_get_target_component(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  9);
}

/**
 * @brief Get field time_usec from redundant_vehicle_selected message
 *
 * @return [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude of the number.
 */
static inline uint64_t mavlink_msg_redundant_vehicle_selected_get_time_usec(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Get field redundant_px4_selected from redundant_vehicle_selected message
 *
 * @return  The redundant autopilot selected after making the calculations.
 */
static inline uint8_t mavlink_msg_redundant_vehicle_selected_get_redundant_px4_selected(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  10);
}

/**
 * @brief Decode a redundant_vehicle_selected message into a struct
 *
 * @param msg The message to decode
 * @param redundant_vehicle_selected C-struct to decode the message contents into
 */
static inline void mavlink_msg_redundant_vehicle_selected_decode(const mavlink_message_t* msg, mavlink_redundant_vehicle_selected_t* redundant_vehicle_selected)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    redundant_vehicle_selected->time_usec = mavlink_msg_redundant_vehicle_selected_get_time_usec(msg);
    redundant_vehicle_selected->target_system = mavlink_msg_redundant_vehicle_selected_get_target_system(msg);
    redundant_vehicle_selected->target_component = mavlink_msg_redundant_vehicle_selected_get_target_component(msg);
    redundant_vehicle_selected->redundant_px4_selected = mavlink_msg_redundant_vehicle_selected_get_redundant_px4_selected(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN? msg->len : MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN;
        memset(redundant_vehicle_selected, 0, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_SELECTED_LEN);
    memcpy(redundant_vehicle_selected, _MAV_PAYLOAD(msg), len);
#endif
}
