// MESSAGE REDUNDANT_VEHICLE_STATUS support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief REDUNDANT_VEHICLE_STATUS message
 *
 * Vehicle data to make the decision about the autopilot selection in a triple redundant system
 */
struct REDUNDANT_VEHICLE_STATUS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 450;
    static constexpr size_t LENGTH = 31;
    static constexpr size_t MIN_LENGTH = 31;
    static constexpr uint8_t CRC_EXTRA = 134;
    static constexpr auto NAME = "REDUNDANT_VEHICLE_STATUS";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude of the number. */
    uint64_t takeoff_time; /*< [us] Time since the drone took off. */
    uint64_t failsafe_timestamp; /*< [us] time when failsafe was activated. */
    uint8_t data_link_lost_counter; /*<  counts unique data link lost events */
    uint16_t redundant_vehicle_status_flags; /*<  Bitmask to indicate vehicle status flags. */
    uint8_t redundant_vehicle_status_failuredetector; /*<  Bitmask to indicate Failure Detector flags. */
    uint8_t redundant_vehicle_status_main; /*<  Bitmask to indicate vehicle status main flags. */


    inline std::string get_name(void) const override
    {
            return NAME;
    }

    inline Info get_message_info(void) const override
    {
            return { MSG_ID, LENGTH, MIN_LENGTH, CRC_EXTRA };
    }

    inline std::string to_yaml(void) const override
    {
        std::stringstream ss;

        ss << NAME << ":" << std::endl;
        ss << "  target_system: " << +target_system << std::endl;
        ss << "  target_component: " << +target_component << std::endl;
        ss << "  time_usec: " << time_usec << std::endl;
        ss << "  takeoff_time: " << takeoff_time << std::endl;
        ss << "  failsafe_timestamp: " << failsafe_timestamp << std::endl;
        ss << "  data_link_lost_counter: " << +data_link_lost_counter << std::endl;
        ss << "  redundant_vehicle_status_flags: " << redundant_vehicle_status_flags << std::endl;
        ss << "  redundant_vehicle_status_failuredetector: " << +redundant_vehicle_status_failuredetector << std::endl;
        ss << "  redundant_vehicle_status_main: " << +redundant_vehicle_status_main << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << takeoff_time;                  // offset: 8
        map << failsafe_timestamp;            // offset: 16
        map << redundant_vehicle_status_flags; // offset: 24
        map << target_system;                 // offset: 26
        map << target_component;              // offset: 27
        map << data_link_lost_counter;        // offset: 28
        map << redundant_vehicle_status_failuredetector; // offset: 29
        map << redundant_vehicle_status_main; // offset: 30
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> takeoff_time;                  // offset: 8
        map >> failsafe_timestamp;            // offset: 16
        map >> redundant_vehicle_status_flags; // offset: 24
        map >> target_system;                 // offset: 26
        map >> target_component;              // offset: 27
        map >> data_link_lost_counter;        // offset: 28
        map >> redundant_vehicle_status_failuredetector; // offset: 29
        map >> redundant_vehicle_status_main; // offset: 30
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
