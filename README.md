# thesis- Redundant autopilot system based on COTs open source solution
solution

To run the PX4 triple redundancy system software follow these steps:


1 - Download the PX4 Autopilot Firmware code - "git clone https://github.com/PX4/PX4-Autopilot.git --recursive"

2 - Inside the directory "PX4-Autopilot/src/modules/" add the new module "redundancy_manager" from this repository "thesis-/PX4files_git/"

3 - Inside the directory "PX4-Autopilot/ROMFS/px4fmu_common/init.d-posix/" replace the runControlScript initialization files: "px4-rc.mavlink", "px4-rc.simulator", "rcS" by the same named files which are inside the directory thesis-/PX4files_git/rcS_scripts/

4 - Replace the "common" and "minimal" directory folders in the "PX4-Autopilot/mavlink/include/mavlink/v2.0/" directory by the "common" and "minimal" directory folders inside "thesis-/new common/" directory. Repeat the same process for the other files inside the "thesis-/new common/" directory.

5 - Download the QgroundControl software from "https://docs.qgroundcontrol.com/master/en/getting_started/download_and_install.html"

6 - Open the Terminal and go to the "PX4-Autopilot/" directory. Run the command "make px4_sitl gazebo"

7 - Open another terminal and use the following commands replacing the (PX4 directory location) by the PX4 firmware directory location from your computer:
export PX4_SIM_MODEL="iris"
cd Developer/PX4-Autopilot/build/px4_sitl_default/instance_1
../bin/px4 -i 1 (PX4 directory location)/PX4-Autopilot/build/px4_sitl_default/etc/ -s etc/init.d-posix/rcS -t "(PX4 directory location)/PX4-Autopilot"/test_data

8 - Open another terminal and use the following commands replacing the (PX4 directory location) by the PX4 firmware directory location from your computer: 
export PX4_SIM_MODEL="iris"
cd Developer/PX4-Autopilot/build/px4_sitl_default/instance_2
../bin/px4 -i 2 (PX4 directory location)/PX4-Autopilot/build/px4_sitl_default/etc/ -s etc/init.d-posix/rcS -t "(PX4 directory location)/Developper/PX4-Autopilot"/test_data
