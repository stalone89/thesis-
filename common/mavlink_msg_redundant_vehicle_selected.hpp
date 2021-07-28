// MESSAGE REDUNDANT_VEHICLE_SELECTED support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief REDUNDANT_VEHICLE_SELECTED message
 *
 * Autopilot selected on a triple redundant syste
 */
struct REDUNDANT_VEHICLE_SELECTED : mavlink::Message {
    static constexpr msgid_t MSG_ID = 451;
    static constexpr size_t LENGTH = 11;
    static constexpr size_t MIN_LENGTH = 11;
    static constexpr uint8_t CRC_EXTRA = 170;
    static constexpr auto NAME = "REDUNDANT_VEHICLE_SELECTED";


    uint8_t target_system; /*<  System ID */
    uint8_t target_component; /*<  Component ID */
    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude of the number. */
    uint8_t redundant_px4_selected; /*<  The redundant autopilot selected after making the calculations. */


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
        ss << "  redundant_px4_selected: " << +redundant_px4_selected << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << target_system;                 // offset: 8
        map << target_component;              // offset: 9
        map << redundant_px4_selected;        // offset: 10
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> target_system;                 // offset: 8
        map >> target_component;              // offset: 9
        map >> redundant_px4_selected;        // offset: 10
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
