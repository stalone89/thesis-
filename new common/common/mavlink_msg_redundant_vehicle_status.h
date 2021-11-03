#pragma once
// MESSAGE REDUNDANT_VEHICLE_STATUS PACKING

#define MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS 450


typedef struct __mavlink_redundant_vehicle_status_t {
 uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude of the number.*/
 uint64_t takeoff_time; /*< [us] Time since the drone took off.*/
 uint64_t failsafe_timestamp; /*< [us] time when failsafe was activated.*/
 uint16_t redundant_vehicle_status_flags; /*<  Bitmask to indicate vehicle status flags.*/
 uint8_t target_system; /*<  System ID*/
 uint8_t target_component; /*<  Component ID*/
 uint8_t data_link_lost_counter; /*<  counts unique data link lost events*/
 uint8_t redundant_vehicle_status_failuredetector; /*<  Bitmask to indicate Failure Detector flags.*/
 uint8_t redundant_vehicle_status_main; /*<  Bitmask to indicate vehicle status main flags.*/
} mavlink_redundant_vehicle_status_t;

#define MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN 31
#define MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_MIN_LEN 31
#define MAVLINK_MSG_ID_450_LEN 31
#define MAVLINK_MSG_ID_450_MIN_LEN 31

#define MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_CRC 134
#define MAVLINK_MSG_ID_450_CRC 134



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_REDUNDANT_VEHICLE_STATUS { \
    450, \
    "REDUNDANT_VEHICLE_STATUS", \
    9, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 26, offsetof(mavlink_redundant_vehicle_status_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 27, offsetof(mavlink_redundant_vehicle_status_t, target_component) }, \
         { "time_usec", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_redundant_vehicle_status_t, time_usec) }, \
         { "takeoff_time", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_redundant_vehicle_status_t, takeoff_time) }, \
         { "failsafe_timestamp", NULL, MAVLINK_TYPE_UINT64_T, 0, 16, offsetof(mavlink_redundant_vehicle_status_t, failsafe_timestamp) }, \
         { "data_link_lost_counter", NULL, MAVLINK_TYPE_UINT8_T, 0, 28, offsetof(mavlink_redundant_vehicle_status_t, data_link_lost_counter) }, \
         { "redundant_vehicle_status_flags", NULL, MAVLINK_TYPE_UINT16_T, 0, 24, offsetof(mavlink_redundant_vehicle_status_t, redundant_vehicle_status_flags) }, \
         { "redundant_vehicle_status_failuredetector", NULL, MAVLINK_TYPE_UINT8_T, 0, 29, offsetof(mavlink_redundant_vehicle_status_t, redundant_vehicle_status_failuredetector) }, \
         { "redundant_vehicle_status_main", NULL, MAVLINK_TYPE_UINT8_T, 0, 30, offsetof(mavlink_redundant_vehicle_status_t, redundant_vehicle_status_main) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_REDUNDANT_VEHICLE_STATUS { \
    "REDUNDANT_VEHICLE_STATUS", \
    9, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 26, offsetof(mavlink_redundant_vehicle_status_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 27, offsetof(mavlink_redundant_vehicle_status_t, target_component) }, \
         { "time_usec", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_redundant_vehicle_status_t, time_usec) }, \
         { "takeoff_time", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_redundant_vehicle_status_t, takeoff_time) }, \
         { "failsafe_timestamp", NULL, MAVLINK_TYPE_UINT64_T, 0, 16, offsetof(mavlink_redundant_vehicle_status_t, failsafe_timestamp) }, \
         { "data_link_lost_counter", NULL, MAVLINK_TYPE_UINT8_T, 0, 28, offsetof(mavlink_redundant_vehicle_status_t, data_link_lost_counter) }, \
         { "redundant_vehicle_status_flags", NULL, MAVLINK_TYPE_UINT16_T, 0, 24, offsetof(mavlink_redundant_vehicle_status_t, redundant_vehicle_status_flags) }, \
         { "redundant_vehicle_status_failuredetector", NULL, MAVLINK_TYPE_UINT8_T, 0, 29, offsetof(mavlink_redundant_vehicle_status_t, redundant_vehicle_status_failuredetector) }, \
         { "redundant_vehicle_status_main", NULL, MAVLINK_TYPE_UINT8_T, 0, 30, offsetof(mavlink_redundant_vehicle_status_t, redundant_vehicle_status_main) }, \
         } \
}
#endif

/**
 * @brief Pack a redundant_vehicle_status message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target_system  System ID
 * @param target_component  Component ID
 * @param time_usec [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude of the number.
 * @param takeoff_time [us] Time since the drone took off.
 * @param failsafe_timestamp [us] time when failsafe was activated.
 * @param data_link_lost_counter  counts unique data link lost events
 * @param redundant_vehicle_status_flags  Bitmask to indicate vehicle status flags.
 * @param redundant_vehicle_status_failuredetector  Bitmask to indicate Failure Detector flags.
 * @param redundant_vehicle_status_main  Bitmask to indicate vehicle status main flags.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_redundant_vehicle_status_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t target_system, uint8_t target_component, uint64_t time_usec, uint64_t takeoff_time, uint64_t failsafe_timestamp, uint8_t data_link_lost_counter, uint16_t redundant_vehicle_status_flags, uint8_t redundant_vehicle_status_failuredetector, uint8_t redundant_vehicle_status_main)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN];
    _mav_put_uint64_t(buf, 0, time_usec);
    _mav_put_uint64_t(buf, 8, takeoff_time);
    _mav_put_uint64_t(buf, 16, failsafe_timestamp);
    _mav_put_uint16_t(buf, 24, redundant_vehicle_status_flags);
    _mav_put_uint8_t(buf, 26, target_system);
    _mav_put_uint8_t(buf, 27, target_component);
    _mav_put_uint8_t(buf, 28, data_link_lost_counter);
    _mav_put_uint8_t(buf, 29, redundant_vehicle_status_failuredetector);
    _mav_put_uint8_t(buf, 30, redundant_vehicle_status_main);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN);
#else
    mavlink_redundant_vehicle_status_t packet;
    packet.time_usec = time_usec;
    packet.takeoff_time = takeoff_time;
    packet.failsafe_timestamp = failsafe_timestamp;
    packet.redundant_vehicle_status_flags = redundant_vehicle_status_flags;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.data_link_lost_counter = data_link_lost_counter;
    packet.redundant_vehicle_status_failuredetector = redundant_vehicle_status_failuredetector;
    packet.redundant_vehicle_status_main = redundant_vehicle_status_main;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_MIN_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_CRC);
}

/**
 * @brief Pack a redundant_vehicle_status message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param target_system  System ID
 * @param target_component  Component ID
 * @param time_usec [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude of the number.
 * @param takeoff_time [us] Time since the drone took off.
 * @param failsafe_timestamp [us] time when failsafe was activated.
 * @param data_link_lost_counter  counts unique data link lost events
 * @param redundant_vehicle_status_flags  Bitmask to indicate vehicle status flags.
 * @param redundant_vehicle_status_failuredetector  Bitmask to indicate Failure Detector flags.
 * @param redundant_vehicle_status_main  Bitmask to indicate vehicle status main flags.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_redundant_vehicle_status_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t target_system,uint8_t target_component,uint64_t time_usec,uint64_t takeoff_time,uint64_t failsafe_timestamp,uint8_t data_link_lost_counter,uint16_t redundant_vehicle_status_flags,uint8_t redundant_vehicle_status_failuredetector,uint8_t redundant_vehicle_status_main)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN];
    _mav_put_uint64_t(buf, 0, time_usec);
    _mav_put_uint64_t(buf, 8, takeoff_time);
    _mav_put_uint64_t(buf, 16, failsafe_timestamp);
    _mav_put_uint16_t(buf, 24, redundant_vehicle_status_flags);
    _mav_put_uint8_t(buf, 26, target_system);
    _mav_put_uint8_t(buf, 27, target_component);
    _mav_put_uint8_t(buf, 28, data_link_lost_counter);
    _mav_put_uint8_t(buf, 29, redundant_vehicle_status_failuredetector);
    _mav_put_uint8_t(buf, 30, redundant_vehicle_status_main);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN);
#else
    mavlink_redundant_vehicle_status_t packet;
    packet.time_usec = time_usec;
    packet.takeoff_time = takeoff_time;
    packet.failsafe_timestamp = failsafe_timestamp;
    packet.redundant_vehicle_status_flags = redundant_vehicle_status_flags;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.data_link_lost_counter = data_link_lost_counter;
    packet.redundant_vehicle_status_failuredetector = redundant_vehicle_status_failuredetector;
    packet.redundant_vehicle_status_main = redundant_vehicle_status_main;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_MIN_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_CRC);
}

/**
 * @brief Encode a redundant_vehicle_status struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param redundant_vehicle_status C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_redundant_vehicle_status_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_redundant_vehicle_status_t* redundant_vehicle_status)
{
    return mavlink_msg_redundant_vehicle_status_pack(system_id, component_id, msg, redundant_vehicle_status->target_system, redundant_vehicle_status->target_component, redundant_vehicle_status->time_usec, redundant_vehicle_status->takeoff_time, redundant_vehicle_status->failsafe_timestamp, redundant_vehicle_status->data_link_lost_counter, redundant_vehicle_status->redundant_vehicle_status_flags, redundant_vehicle_status->redundant_vehicle_status_failuredetector, redundant_vehicle_status->redundant_vehicle_status_main);
}

/**
 * @brief Encode a redundant_vehicle_status struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param redundant_vehicle_status C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_redundant_vehicle_status_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_redundant_vehicle_status_t* redundant_vehicle_status)
{
    return mavlink_msg_redundant_vehicle_status_pack_chan(system_id, component_id, chan, msg, redundant_vehicle_status->target_system, redundant_vehicle_status->target_component, redundant_vehicle_status->time_usec, redundant_vehicle_status->takeoff_time, redundant_vehicle_status->failsafe_timestamp, redundant_vehicle_status->data_link_lost_counter, redundant_vehicle_status->redundant_vehicle_status_flags, redundant_vehicle_status->redundant_vehicle_status_failuredetector, redundant_vehicle_status->redundant_vehicle_status_main);
}

/**
 * @brief Send a redundant_vehicle_status message
 * @param chan MAVLink channel to send the message
 *
 * @param target_system  System ID
 * @param target_component  Component ID
 * @param time_usec [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude of the number.
 * @param takeoff_time [us] Time since the drone took off.
 * @param failsafe_timestamp [us] time when failsafe was activated.
 * @param data_link_lost_counter  counts unique data link lost events
 * @param redundant_vehicle_status_flags  Bitmask to indicate vehicle status flags.
 * @param redundant_vehicle_status_failuredetector  Bitmask to indicate Failure Detector flags.
 * @param redundant_vehicle_status_main  Bitmask to indicate vehicle status main flags.
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_redundant_vehicle_status_send(mavlink_channel_t chan, uint8_t target_system, uint8_t target_component, uint64_t time_usec, uint64_t takeoff_time, uint64_t failsafe_timestamp, uint8_t data_link_lost_counter, uint16_t redundant_vehicle_status_flags, uint8_t redundant_vehicle_status_failuredetector, uint8_t redundant_vehicle_status_main)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN];
    _mav_put_uint64_t(buf, 0, time_usec);
    _mav_put_uint64_t(buf, 8, takeoff_time);
    _mav_put_uint64_t(buf, 16, failsafe_timestamp);
    _mav_put_uint16_t(buf, 24, redundant_vehicle_status_flags);
    _mav_put_uint8_t(buf, 26, target_system);
    _mav_put_uint8_t(buf, 27, target_component);
    _mav_put_uint8_t(buf, 28, data_link_lost_counter);
    _mav_put_uint8_t(buf, 29, redundant_vehicle_status_failuredetector);
    _mav_put_uint8_t(buf, 30, redundant_vehicle_status_main);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS, buf, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_MIN_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_CRC);
#else
    mavlink_redundant_vehicle_status_t packet;
    packet.time_usec = time_usec;
    packet.takeoff_time = takeoff_time;
    packet.failsafe_timestamp = failsafe_timestamp;
    packet.redundant_vehicle_status_flags = redundant_vehicle_status_flags;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.data_link_lost_counter = data_link_lost_counter;
    packet.redundant_vehicle_status_failuredetector = redundant_vehicle_status_failuredetector;
    packet.redundant_vehicle_status_main = redundant_vehicle_status_main;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS, (const char *)&packet, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_MIN_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_CRC);
#endif
}

/**
 * @brief Send a redundant_vehicle_status message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_redundant_vehicle_status_send_struct(mavlink_channel_t chan, const mavlink_redundant_vehicle_status_t* redundant_vehicle_status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_redundant_vehicle_status_send(chan, redundant_vehicle_status->target_system, redundant_vehicle_status->target_component, redundant_vehicle_status->time_usec, redundant_vehicle_status->takeoff_time, redundant_vehicle_status->failsafe_timestamp, redundant_vehicle_status->data_link_lost_counter, redundant_vehicle_status->redundant_vehicle_status_flags, redundant_vehicle_status->redundant_vehicle_status_failuredetector, redundant_vehicle_status->redundant_vehicle_status_main);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS, (const char *)redundant_vehicle_status, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_MIN_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_CRC);
#endif
}

#if MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_redundant_vehicle_status_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t target_system, uint8_t target_component, uint64_t time_usec, uint64_t takeoff_time, uint64_t failsafe_timestamp, uint8_t data_link_lost_counter, uint16_t redundant_vehicle_status_flags, uint8_t redundant_vehicle_status_failuredetector, uint8_t redundant_vehicle_status_main)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint64_t(buf, 0, time_usec);
    _mav_put_uint64_t(buf, 8, takeoff_time);
    _mav_put_uint64_t(buf, 16, failsafe_timestamp);
    _mav_put_uint16_t(buf, 24, redundant_vehicle_status_flags);
    _mav_put_uint8_t(buf, 26, target_system);
    _mav_put_uint8_t(buf, 27, target_component);
    _mav_put_uint8_t(buf, 28, data_link_lost_counter);
    _mav_put_uint8_t(buf, 29, redundant_vehicle_status_failuredetector);
    _mav_put_uint8_t(buf, 30, redundant_vehicle_status_main);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS, buf, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_MIN_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_CRC);
#else
    mavlink_redundant_vehicle_status_t *packet = (mavlink_redundant_vehicle_status_t *)msgbuf;
    packet->time_usec = time_usec;
    packet->takeoff_time = takeoff_time;
    packet->failsafe_timestamp = failsafe_timestamp;
    packet->redundant_vehicle_status_flags = redundant_vehicle_status_flags;
    packet->target_system = target_system;
    packet->target_component = target_component;
    packet->data_link_lost_counter = data_link_lost_counter;
    packet->redundant_vehicle_status_failuredetector = redundant_vehicle_status_failuredetector;
    packet->redundant_vehicle_status_main = redundant_vehicle_status_main;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS, (const char *)packet, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_MIN_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_CRC);
#endif
}
#endif

#endif

// MESSAGE REDUNDANT_VEHICLE_STATUS UNPACKING


/**
 * @brief Get field target_system from redundant_vehicle_status message
 *
 * @return  System ID
 */
static inline uint8_t mavlink_msg_redundant_vehicle_status_get_target_system(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  26);
}

/**
 * @brief Get field target_component from redundant_vehicle_status message
 *
 * @return  Component ID
 */
static inline uint8_t mavlink_msg_redundant_vehicle_status_get_target_component(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  27);
}

/**
 * @brief Get field time_usec from redundant_vehicle_status message
 *
 * @return [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude of the number.
 */
static inline uint64_t mavlink_msg_redundant_vehicle_status_get_time_usec(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Get field takeoff_time from redundant_vehicle_status message
 *
 * @return [us] Time since the drone took off.
 */
static inline uint64_t mavlink_msg_redundant_vehicle_status_get_takeoff_time(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  8);
}

/**
 * @brief Get field failsafe_timestamp from redundant_vehicle_status message
 *
 * @return [us] time when failsafe was activated.
 */
static inline uint64_t mavlink_msg_redundant_vehicle_status_get_failsafe_timestamp(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  16);
}

/**
 * @brief Get field data_link_lost_counter from redundant_vehicle_status message
 *
 * @return  counts unique data link lost events
 */
static inline uint8_t mavlink_msg_redundant_vehicle_status_get_data_link_lost_counter(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  28);
}

/**
 * @brief Get field redundant_vehicle_status_flags from redundant_vehicle_status message
 *
 * @return  Bitmask to indicate vehicle status flags.
 */
static inline uint16_t mavlink_msg_redundant_vehicle_status_get_redundant_vehicle_status_flags(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  24);
}

/**
 * @brief Get field redundant_vehicle_status_failuredetector from redundant_vehicle_status message
 *
 * @return  Bitmask to indicate Failure Detector flags.
 */
static inline uint8_t mavlink_msg_redundant_vehicle_status_get_redundant_vehicle_status_failuredetector(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  29);
}

/**
 * @brief Get field redundant_vehicle_status_main from redundant_vehicle_status message
 *
 * @return  Bitmask to indicate vehicle status main flags.
 */
static inline uint8_t mavlink_msg_redundant_vehicle_status_get_redundant_vehicle_status_main(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  30);
}

/**
 * @brief Decode a redundant_vehicle_status message into a struct
 *
 * @param msg The message to decode
 * @param redundant_vehicle_status C-struct to decode the message contents into
 */
static inline void mavlink_msg_redundant_vehicle_status_decode(const mavlink_message_t* msg, mavlink_redundant_vehicle_status_t* redundant_vehicle_status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    redundant_vehicle_status->time_usec = mavlink_msg_redundant_vehicle_status_get_time_usec(msg);
    redundant_vehicle_status->takeoff_time = mavlink_msg_redundant_vehicle_status_get_takeoff_time(msg);
    redundant_vehicle_status->failsafe_timestamp = mavlink_msg_redundant_vehicle_status_get_failsafe_timestamp(msg);
    redundant_vehicle_status->redundant_vehicle_status_flags = mavlink_msg_redundant_vehicle_status_get_redundant_vehicle_status_flags(msg);
    redundant_vehicle_status->target_system = mavlink_msg_redundant_vehicle_status_get_target_system(msg);
    redundant_vehicle_status->target_component = mavlink_msg_redundant_vehicle_status_get_target_component(msg);
    redundant_vehicle_status->data_link_lost_counter = mavlink_msg_redundant_vehicle_status_get_data_link_lost_counter(msg);
    redundant_vehicle_status->redundant_vehicle_status_failuredetector = mavlink_msg_redundant_vehicle_status_get_redundant_vehicle_status_failuredetector(msg);
    redundant_vehicle_status->redundant_vehicle_status_main = mavlink_msg_redundant_vehicle_status_get_redundant_vehicle_status_main(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN? msg->len : MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN;
        memset(redundant_vehicle_status, 0, MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS_LEN);
    memcpy(redundant_vehicle_status, _MAV_PAYLOAD(msg), len);
#endif
}
