#include <px4_platform_common/log.h>
#include <px4_platform_common/tasks.h>
#include <px4_platform_common/time.h>
#include <systemlib/err.h>
#include <drivers/drv_board_led.h>

#include "redundancy_manager.h"

static px4_task_t g_sim_task = -1;

redundancy_manager *redundancy_manager::instance = nullptr;

void redundancy_manager::parameters_update(bool force)
{
	// check for parameter updates
	if (_parameter_update_sub.updated() || force) {
		// clear update
		parameter_update_s pupdate;
		_parameter_update_sub.copy(&pupdate);

		// update parameters from storage
		updateParams();
	}
}

int redundancy_manager::start(int argc, char *argv[])
{
	instance = new redundancy_manager();

	if (instance) {

		if (strcmp(argv[3], "-p") == 0)
		{
			instance->set_parameter_instance(atoi(argv[4]));

		}
		instance->run();

		return 0;

	} else {
		PX4_WARN("redundancy_manager creation failed");
		return 1;
	}
}

static void usage()
{
	PX4_INFO("Usage: redundancy_manager {start -p px4_instance|stop|status}");
	PX4_INFO("Start redundancy_manager:     redundancy_manager start");

}

__BEGIN_DECLS
extern int redundancy_manager_main(int argc, char *argv[]);
__END_DECLS


int redundancy_manager_main(int argc, char *argv[])
{
	if (argc > 1 && strcmp(argv[1], "start") == 0) {

		if (g_sim_task >= 0) {
			PX4_WARN("redundancy_manager already started");
			return 0;
		}

		g_sim_task = px4_task_spawn_cmd("redundancy_manager",
						SCHED_DEFAULT,
						SCHED_PRIORITY_MAX,
						1500,
						redundancy_manager::start,
						argv);


	} else if (argc == 2 && strcmp(argv[1], "stop") == 0) {
		if (g_sim_task < 0) {
			PX4_WARN("redundancy_manager not running");
			return 1;

		} else {
			px4_task_delete(g_sim_task);
			g_sim_task = -1;
		}

	} else if (argc == 2 && strcmp(argv[1], "status") == 0) {
		if (g_sim_task < 0) {
			PX4_WARN("redundancy_manager not running");
			return 1;

		} else {
			PX4_INFO("running");
		}

	} else {
		usage();
		return 1;
	}

	return 0;
}
