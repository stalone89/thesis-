# thesis- Redundant autopilot system based on COTs open source
solution

To run the PX4 triple redundancy system software follow these steps:


1 - PX4 Auto-pilot Firmware should be cloned from "https://github.com/PX4/PX4-Autopilot" 

2 - Inside the directory "PX4-Autopilot/src/modules/" add the new module "redundancy_manager" from this repository "thesis-/PX4files_git/"

3 - Inside the directory "PX4-Autopilot/ROMFS/px4fmu_common/init.d-posix/" replace the runControlScript initialization files: "px4-rc.mavlink", "px4-rc.simulator", "rcS" by the same named files which are inside the directory thesis-/PX4files_git/rcS_scripts/

4-  Replace the "common" directory from the "PX4-Autopilot/mavlink/include/mavlink" directory by the "new common" in the "thesis-/" directory. Rename the replaced "new common" directory to "common" inside the "PX4-Autopilot/mavlink/include/mavlink" directory.

5 - Download the QgroundControl software from "https://docs.qgroundcontrol.com/master/en/getting_started/download_and_install.html"
