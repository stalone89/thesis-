/****************************************************************************
 *
 *   Copyright (c) 2018 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#pragma once

#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <arpa/inet.h>
#include <signal.h>
#include <bits/sigaction.h>
#include <mutex>



#include <lib/ecl/geo/geo.h>
#include <lib/perf/perf_counter.h>
#include <lib/mathlib/mathlib.h>



#include <px4_platform_common/atomic.h>
#include <px4_platform_common/bitmask.h>
#include <px4_platform_common/defines.h>
#include <px4_platform_common/posix.h>
#include <px4_platform_common/module.h>
#include <px4_platform_common/time.h>
#include <px4_platform_common/module_params.h>




#include <uORB/Publication.hpp>
#include <uORB/Subscription.hpp>
#include <uORB/SubscriptionInterval.hpp>
#include <uORB/topics/parameter_update.h>
#include <uORB/topics/estimator_selector_status.h>
#include <uORB/topics/estimator_status.h>
#include <uORB/topics/estimator_status_flags.h>
#include <uORB/topics/estimator_event_flags.h>
#include <uORB/topics/vehicle_status.h>
#include <uORB/topics/vehicle_status_flags.h>
#include <uORB/topics/redundancy_firstpilot.h>
#include <uORB/topics/redundancy_firstpilot_sensors.h>
#include <uORB/topics/redundancy_firstpilot_heartbeat.h>
#include <uORB/topics/redundancy_secondpilot.h>
#include <uORB/topics/redundancy_secondpilot_sensors.h>
#include <uORB/topics/redundancy_secondpilot_heartbeat.h>
#include <uORB/topics/redundancy_selection.h>


#include <v2.0/common/mavlink.h>
#include <v2.0/mavlink_types.h>


using namespace time_literals;


ENABLE_BIT_OPERATORS(REDUNDANT_VEHICLE_STATUS_FLAGS)
ENABLE_BIT_OPERATORS(REDUNDANT_VEHICLE_STATUS_MAIN)



extern "C" __EXPORT int redundancy_manager_main(int argc, char *argv[]);


class redundancy_manager: public ModuleParams
{
public:
	redundancy_manager(int example_param, bool example_flag);

	virtual ~redundancy_manager() = default;

	static int start(int argc, char *argv[]);

	/** @see ModuleBase */
	static int task_spawn(int argc, char *argv[]);

	/** @see ModuleBase */
	static redundancy_manager *instantiate(int argc, char *argv[]);

	/** @see ModuleBase */
	static int custom_command(int argc, char *argv[]);

	/** @see ModuleBase */
	static int print_usage(const char *reason = nullptr);

	/** @see ModuleBase::run() */


	/** @see ModuleBase::print_status() */
	//int print_status() override;



	void set_parameter_instance(int px4_inst) {px4_instance = px4_inst;}


private:

	redundancy_manager() : ModuleParams(nullptr)
	{
	}


	unsigned int _port_1{} , _port_2{}, _port_3{}, _port_4{};
	int px4_instance;

	redundancy_firstpilot_s redundant_firstpilotuorb = {};
	redundancy_secondpilot_s redundant_secondpilotuorb = {};
	redundancy_selection_s redundancy_selection_uorb = {};

	static redundancy_manager *instance;


	/**
	 * Check for parameter changes and update them if needed.
	 * @param parameter_update_sub uorb subscription to parameter_update
	 * @param force for a parameter update
	 */





	void parameters_update(bool force);


	DEFINE_PARAMETERS(
		(ParamInt<px4::params::SYS_AUTOSTART>) _param_sys_autostart,   /**< example parameter */
		(ParamInt<px4::params::SYS_AUTOCONFIG>) _param_sys_autoconfig,  /**< another parameter */
		(ParamInt<px4::params::MAV_SYS_ID>) _param_mav_sys_id,
		(ParamInt<px4::params::MAV_COMP_ID>) _param_mav_comp_id
	)

	//publications
	uORB::Publication<redundancy_firstpilot_heartbeat_s>		_redundancy_firstpilot_hearbeat_pub{ORB_ID(redundancy_firstpilot_heartbeat)};
	uORB::Publication<redundancy_firstpilot_s>			_redundancy_firstpilot_pub{ORB_ID(redundancy_firstpilot)};
	uORB::Publication<redundancy_firstpilot_sensors_s>		_redundancy_firstpilot_sensors_pub{ORB_ID(redundancy_firstpilot_sensors)};
	uORB::Publication<redundancy_secondpilot_heartbeat_s>		_redundancy_secondpilot_hearbeat_pub{ORB_ID(redundancy_secondpilot_heartbeat)};
	uORB::Publication<redundancy_secondpilot_s>			_redundancy_secondpilot_pub{ORB_ID(redundancy_secondpilot)};
	uORB::Publication<redundancy_secondpilot_sensors_s>		_redundancy_secondpilot_sensors_pub{ORB_ID(redundancy_secondpilot_sensors)};
	uORB::Publication<redundancy_selection_s>			_redundancy_selection_pub{ORB_ID(redundancy_selection)};




	bool estimator_selector_failing{false};
	bool estimator_failing{false};
	bool vehicle_failing{false};



	int error_flags_number = 12;

	bool mypilotfailing{false};
	bool firstpilotfailing{false};
	bool secondpilotfailing{false};
	bool mypilotfailed{false};
	bool firstpilotfailed{false};
	bool secondpilotfailed{false};

	bool mypilotfaults[12], firstpilotfaults[12], secondpilotfaults[12], mypilotfaults_1[12], firstpilotfaults_1[12], secondpilotfaults_1[12];

	int counter_pilotfailing[3], pilot_priority_1[3], pilot_error_count[3];
	int counter_failing_worst, counter_failing_worst_2;



	hrt_abstime time_last_mypilot_pass{0};
	hrt_abstime time_last_mypilot_fail{0};
	hrt_abstime time_last_firstpilot_pass{0};
	hrt_abstime time_last_firstpilot_fail{0};
	hrt_abstime time_last_secondpilot_pass{0};
	hrt_abstime time_last_secondpilot_fail{0};
	hrt_abstime communication_firstpilot{0} , communication_secondpilot{0}, test_seconds{0};

	long int first_microseconds, second_microseconds;
	bool communication_fail_firstpilot{false}, communication_fail_secondpilot{false};


	hrt_abstime time_last_mypilot_fault_fail[12],time_last_mypilot_fault_pass[12], time_last_firstpilot_fault_fail[12], time_last_firstpilot_fault_pass[12], time_last_secondpilot_fault_fail[12],time_last_secondpilot_fault_pass[12];




	struct sockaddr_in addr_selection {} , addr_qgc_selection {};
	socklen_t addr_selection_len {}, addr_qgc_selection_len {};
	int qgc_selection_socket, selection_socket;


	bool pilot_healthy[3];
	uint8_t pilot_priority[3], best_pilot{1}, best_pilot_priority{3}, previous_best_pilot{1};




	void run();
	void send();
	static void *sending_trampoline(void *);
	void send_heartbeat();
	void send_mavlink_message(const mavlink_message_t &aMsg, int system);
	void redundancy_vehicle_status(mavlink_redundant_vehicle_status_t  *msg, int target_system);
	void redundancy_sensors_state(mavlink_sys_status_t *msg);
	void handle_message(const mavlink_message_t *msg, bool firstpilotreceiving);
	void handle_message_redundant(const mavlink_message_t *msg, bool firstpilotreceiving);
	void handle_message_sensors(const mavlink_message_t *msg, bool firstpilotreceiving);
	void handle_message_heartbeat(const mavlink_message_t *msg, bool firstpilotreceiving);
	void check_pilots();
	void check_failing_flags();
	void define_pilots_condition();
	void voting_best_pilot();
	void send_selection();

	estimator_selector_status_s _estimator_selector_status{};
	estimator_status_s _estimator_status{};
	estimator_status_flags_s _estimator_status_flags{};
	estimator_event_flags_s _estimator_event_flags{};
	vehicle_status_s _vehicle_status{};
	vehicle_status_flags_s _vehicle_status_flags{};
	redundancy_firstpilot_s _redundancy_firstpilot{};


	int _vehicle_status2_sub{-1};


	// Subscriptions
	uORB::SubscriptionInterval _parameter_update_sub{ORB_ID(parameter_update), 1_s};
	uORB::Subscription                                      _estimator_selector_status_sub{ORB_ID(estimator_selector_status)};
	uORB::Subscription                                      _vehicle_status_sub{ORB_ID(vehicle_status)};
	uORB::Subscription                                      _vehicle_status_flags_sub{ORB_ID(vehicle_status_flags)};
	uORB::Subscription                                      _redundancy_firstpilot_sub{ORB_ID(redundancy_firstpilot)};
	uORB::SubscriptionData<estimator_status_s>		_estimator_status_sub{ORB_ID(estimator_status)};
	uORB::SubscriptionData<estimator_status_flags_s>	_estimator_status_flags_sub{ORB_ID(estimator_status_flags)};
	uORB::SubscriptionData<estimator_event_flags_s>		_estimator_event_flags_sub{ORB_ID(estimator_event_flags)};


};

