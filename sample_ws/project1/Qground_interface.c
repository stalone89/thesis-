#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <netinet/tcp.h>
#include <signal.h>
#include <bits/sigaction.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <v2.0/common/mavlink.h>
#include <v2.0/mavlink_types.h>
#include <pthread.h>
#include <stdbool.h>






// Flag to end infite cycles from sockets and close them    
volatile __sig_atomic_t myflag;    
    
int qgc_socket, firstpilot_socket, secondpilot_socket, thirdpilot_socket, pilots_number;

pthread_mutex_t qgc_lock = PTHREAD_MUTEX_INITIALIZER;

struct pollfd fds_firstpilot[1], fds_secondpilot[1], fds_thirdpilot[1], fds_qgc[1];

bool firstpilotselection, secondpilotselection, thirdpilotselection;

int firstpilot_selection_socket, secondpilot_selection_socket, thirdpilot_selection_socket;

struct sockaddr_in remote_firstpilot_addr, qgc_addr, remote_secondpilot_addr, remote_thirdpilot_addr;
__socklen_t remote_firstpilot_addr_len, remote_secondpilot_addr_len, remote_thirdpilot_addr_len, qgc_addr_len;

pthread_mutex_t selection_mutex;








//Selection Interface

void *selection_communication(void *ptr)
{

     struct sockaddr_in addr_first_selection, addr_second_selection, addr_third_selection, remote_first_selection,remote_second_selection, remote_third_selection ;
    __socklen_t  addr_first_selection_len, addr_second_selection_len, addr_third_selection_len, remote_first_selection_len, remote_second_selection_len, remote_third_selection_len;
    
    struct pollfd fds_selection[6];
    typedef uint32_t in_addr_t; 


    mavlink_status_t mavlink_status;
    mavlink_message_t msg_1, msg_2, msg_3;
    int chan = MAVLINK_COMM_2; 

    char buffer_selection[MAVLINK_MAX_PACKET_LEN];
    int timeout = 3000;
    int fd_count_selection = 6;
    
    /* Addressing the first PX4 and get the socket to poll*/

    int firstpilot_portnumber = 6515;
    int secondpilot_portnumber = 6516;
    int thirdpilot_portnumber = 6517;

    int voter[3], counter[3], total_count, best_votes, best_pilot;
    memset(voter, 1, sizeof(voter));
    bool first_voted, second_voted, third_voted;
    
    counter[0] = 0;
    counter[1] = 0;
    counter[3] = 0;
    total_count = 0;
    best_votes = 0;
    best_pilot = 1;
    first_voted = false;
    second_voted = false;
    third_voted = false;




    memset((char *)&addr_first_selection,0,sizeof(addr_first_selection));
    addr_first_selection.sin_family = AF_INET;
    addr_first_selection_len = sizeof(addr_first_selection);
    addr_first_selection.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_first_selection.sin_port = htons(firstpilot_portnumber);

    memset((char *)&addr_second_selection,0,sizeof(addr_second_selection));
    addr_second_selection.sin_family = AF_INET;
    addr_second_selection_len = sizeof(addr_second_selection);
    addr_second_selection.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_second_selection.sin_port = htons(secondpilot_portnumber);

    memset((char *)&addr_third_selection,0,sizeof(addr_third_selection));
    addr_third_selection.sin_family = AF_INET;
    addr_third_selection_len = sizeof(addr_third_selection);
    addr_third_selection.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_third_selection.sin_port = htons(thirdpilot_portnumber);

    
    memset((char *)&remote_first_selection, 0, sizeof(remote_first_selection_len));
    memset((char *)&remote_second_selection, 0, sizeof(remote_second_selection_len));
    memset((char *)&remote_third_selection, 0, sizeof(remote_third_selection_len));


    struct linger nolinger;
    nolinger.l_onoff = 1;
    nolinger.l_linger = 0;


    //Selection first pilot socket

    if ((firstpilot_selection_socket = socket(AF_INET, SOCK_STREAM,0)) < 0)
    {
        perror("Creating TCP socket failed on selection first pilot, aborting\n");
        abort();    
    }
    
    int yes3;
    
    int result3 = setsockopt(firstpilot_selection_socket, IPPROTO_TCP, TCP_NODELAY, &yes3, sizeof(yes3));
    if (result3 != 0)
    {
        perror("setsockpt failed on selection first pilot, aborting\n");
        abort();
    }

    result3 = setsockopt(firstpilot_selection_socket, SOL_SOCKET, SO_LINGER, &nolinger, sizeof(nolinger));
    if (result3 != 0)
    {
        perror("setsockpt linger failed on selection first pilot, aborting\n");
        abort();
    }

    int socket_reuse = 1;

    result3 = setsockopt(firstpilot_selection_socket, SOL_SOCKET, SO_REUSEADDR, &socket_reuse, sizeof(socket_reuse));
    if (result3 != 0) {
        perror("setsockopt reuse address on selection first pilot failed, aborting\n");
        abort();
      }


    result3 = fcntl(firstpilot_selection_socket, F_SETFL, O_NONBLOCK);

    if (result3 == -1)
    {
        perror("setting selection first pilot socket to non-blocking failed, aborting\n");
        abort();
    }
    
    if (bind(firstpilot_selection_socket,(struct sockaddr *)&addr_first_selection,addr_first_selection_len) < 0)  
    {
        perror("bind on selection first pilot failed, aborting\n");
        abort();
    }
    
    if (listen(firstpilot_selection_socket,0) < 0)
    {
        perror("listen on selection first pilot failed, aborting\n");
        abort();
    }




    //Selection second pilot socket

    if ((secondpilot_selection_socket = socket(AF_INET, SOCK_STREAM,0)) < 0)
    {
        perror("Creating TCP socket failed on selection second pilot, aborting\n");
        abort();    
    }
    
    int yes;
    
    int result = setsockopt(secondpilot_selection_socket, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
    if (result3 != 0)
    {
        perror("setsockpt failed on selection second pilot, aborting\n");
        abort();
    }

    result = setsockopt(secondpilot_selection_socket, SOL_SOCKET, SO_LINGER, &nolinger, sizeof(nolinger));
    if (result != 0)
    {
        perror("setsockpt linger failed on selection second pilot, aborting\n");
        abort();
    }


    result = setsockopt(secondpilot_selection_socket, SOL_SOCKET, SO_REUSEADDR, &socket_reuse, sizeof(socket_reuse));
    if (result != 0) {
        perror("setsockopt reuse address on selection second pilot failed, aborting\n");
        abort();
      }


    result = fcntl(secondpilot_selection_socket, F_SETFL, O_NONBLOCK);

    if (result3 == -1)
    {
        perror("setting selection second pilot socket to non-blocking failed, aborting\n");
        abort();
    }
    
    if (bind(secondpilot_selection_socket,(struct sockaddr *)&addr_second_selection,addr_second_selection_len) < 0)  
    {
        perror("bind on selection second pilot failed, aborting\n");
        abort();
    }
    
    if (listen(secondpilot_selection_socket,0) < 0)
    {
        perror("listen on selection second pilot failed, aborting\n");
        abort();
    }




    //Selection third pilot socket

    if ((thirdpilot_selection_socket = socket(AF_INET, SOCK_STREAM,0)) < 0)
    {
        perror("Creating TCP socket failed on selection third pilot, aborting\n");
        abort();    
    }
    
    int yes1;
    
    int result1 = setsockopt(thirdpilot_selection_socket, IPPROTO_TCP, TCP_NODELAY, &yes1, sizeof(yes1));
    if (result1 != 0)
    {
        perror("setsockpt failed on selection third pilot, aborting\n");
        abort();
    }

    result1 = setsockopt(thirdpilot_selection_socket, SOL_SOCKET, SO_LINGER, &nolinger, sizeof(nolinger));
    if (result1 != 0)
    {
        perror("setsockpt linger failed on selection third pilot, aborting\n");
        abort();
    }


    result1 = setsockopt(thirdpilot_selection_socket, SOL_SOCKET, SO_REUSEADDR, &socket_reuse, sizeof(socket_reuse));
    if (result1 != 0) {
        perror("setsockopt reuse address on selection third pilot failed, aborting\n");
        abort();
      }


    result1 = fcntl(thirdpilot_selection_socket, F_SETFL, O_NONBLOCK);

    if (result3 == -1)
    {
        perror("setting selection third pilot socket to non-blocking failed, aborting\n");
        abort();
    }
    
    if (bind(thirdpilot_selection_socket,(struct sockaddr *)&addr_third_selection,addr_third_selection_len) < 0)  
    {
        perror("bind on selection third pilot failed, aborting\n");
        abort();
    }
    
    if (listen(thirdpilot_selection_socket,0) < 0)
    {
        perror("listen on selection third pilot failed, aborting\n");
        abort();
    }



    //Making the poll with the three sockets

    memset(fds_selection,0,sizeof(fds_selection)); 
    fds_selection[0].fd = firstpilot_selection_socket;
    fds_selection[0].events = POLLIN;
    fds_selection[2].fd = secondpilot_selection_socket;
    fds_selection[2].events = POLLIN;
    fds_selection[4].fd = thirdpilot_selection_socket;
    fds_selection[4].events = POLLIN;


     while(!myflag)
    {   
        pthread_mutex_lock(&selection_mutex);
        int ret = poll(&fds_selection[0],fd_count_selection,5000);
        if (ret < 0)
        {
            perror("poll error on selection socket\n");
            continue;
        }

        if (ret == 0)
        {
            perror("poll timeout on selection socket\n");
            continue;
        }
        
        for (size_t m = 0; m < fd_count_selection; m++)
        {
            if (fds_selection[m].revents == 0)
            {
                continue;
            }
            
            if (!(fds_selection[m].revents & POLLIN))
            {
                continue;
            }
            
            /*If event is raised on the listening socket - accept connections*/
            if (m == 0)
            {
                if (fds_selection[1].fd > 0) /*Already connected */
                {
                    continue;
                }
                
                int rett = accept(fds_selection[0].fd,(struct sockaddr *)&remote_first_selection,&remote_first_selection_len);  

                if (rett < 0)
                {
                    if (errno != EWOULDBLOCK)
                    {
                        printf("accept error %s \n", strerror(errno));
                    }

                    continue;
                }

                fds_selection[1].fd = rett;
                fds_selection[1].events = POLLIN;

            } else  if (m==2)
                    {
                         if (fds_selection[3].fd > 0) /*Already connected */
                        {
                            continue;
                        }
                        int rett = accept(fds_selection[2].fd,(struct sockaddr *)&remote_second_selection,&remote_second_selection_len);  

                        if (rett < 0)
                        {
                            if (errno != EWOULDBLOCK)
                            {
                                printf("accept error %s \n", strerror(errno));
                            }

                            continue;
                        }

                        fds_selection[3].fd = rett;
                        fds_selection[3].events = POLLIN;

                    } else  if (m == 4)
                            {
                                if (fds_selection[5].fd > 0) /*Already connected */
                                {
                                    continue;
                                }
                                
                                int rett = accept(fds_selection[4].fd,(struct sockaddr *)&remote_third_selection,&remote_third_selection_len);  

                                if (rett < 0)
                                {
                                    if (errno != EWOULDBLOCK)
                                    {
                                        printf("accept error %s \n", strerror(errno));
                                    }

                                    continue;
                                }

                                fds_selection[5].fd = rett;
                                fds_selection[5].events = POLLIN;    
                            } else {
                    
                                         /* If event is raised on connection socket */
                        

                                        int ret = recv(fds_selection[m].fd, buffer_selection,sizeof(buffer_selection),0);
                                        printf("Received votes");
                                        if (ret < 0)
                                        {
                                            if (errno != EWOULDBLOCK) /* disconnected from client */
                                            {
                                                printf("recv error : %s \n", strerror(errno));
                                            }
                                            continue;
                                            
                                        }

                                        if (ret == 0) /* Client PX4 closed the connection orderly */
                                        {
                                            printf("Connection closed on selection pilot by client\n");
                                
                                            continue;
                                        }

                                        int len = ret;
                                        mavlink_status_t status;

                                        //Using a switch to parse different pilot messages
                                        for (unsigned k = 0; k < len; ++k)
                                        {   

                                            
                                            
                                            if (mavlink_parse_char(chan,buffer_selection[k],&msg_1, &status))
                                            {
                                                
                                                mavlink_redundant_vehicle_selected_t redundant_pilot_selection;
                                                int autopilot_id_origin = msg_1.sysid; 
                                                mavlink_msg_redundant_vehicle_selected_decode(&msg_1, &redundant_pilot_selection);
                                            
                                                switch (m)
                                                {
                                                case 1:
                                                     //Confirm if the corrrespont ID refers to the autopilot
                                                    if (autopilot_id_origin == 1)
                                                    {
                                                        voter[0] = redundant_pilot_selection.redundant_px4_selected;
                                                        first_voted = true;
                                                        printf("Received vote from autopilot 1\n");
                                                    }
                                                    break;
                                                case 3:

                                                    if (autopilot_id_origin == 2)
                                                    { 
                                                        voter[1] = redundant_pilot_selection.redundant_px4_selected;
                                                        second_voted = true;
                                                        printf("Received vote from autopilot 2\n");
                                                    }
                                                    
                                                    break;
                                                case 5:
                                                    if (autopilot_id_origin == 3)
                                                    {
                                                        voter[2] = redundant_pilot_selection.redundant_px4_selected;
                                                        third_voted = true;
                                                        printf("Received vote from autopilot 3\n");
                                                    }
                                                    break;
                                                default:
                                                    break;
                                                }
                                                
                                                if (first_voted && second_voted && third_voted)
                                                {

                                                    for(unsigned y = 0; y < 3; ++y)
                                                    {
                                                        switch (voter[y])
                                                        {
                                                        case 1:
                                                            counter[0] = counter[0] +1;
                                                            break;
                                                        case 2:
                                                            counter[1] = counter[1] +1;
                                                            break;
                                                        case 3:
                                                            counter[2] = counter[2] +1;
                                                            break;
                                                        
                                                        default:
                                                            break;
                                                        } 
                                                    }

                                                    for (size_t i = 0; i < 3; i++)
                                                    {
                                                        total_count = total_count + counter[i];
                                                    }
                                                    
                                                    
                                                    if (total_count == 3)
                                                    {   
                                                        
                                                    
                                                        for (size_t z = 0; z < 3; z++)
                                                        {
                                                            if (counter[z] > best_votes)
                                                            {
                                                                best_votes = counter[z];
                                                                best_pilot = z+1;
                                                            }

                                                        }
                                                        
                                                        total_count= 0;
                                                        best_votes = 0;
                                                        first_voted = false;
                                                        second_voted = false;
                                                        third_voted = false;
                                                        for (size_t y = 0; y < 3; y++)
                                                        {
                                                            voter[y]= 0;
                                                            counter[y]= 0;
                                                        }
                                                        

                                                        
                                                        switch (best_pilot)
                                                        {
                                                        case 1:
                                                            firstpilotselection = true;
                                                            secondpilotselection = false;
                                                            thirdpilotselection = false;
                                                            break;
                                                        
                                                        
                                                        case 2:
                                                            firstpilotselection = false;
                                                            secondpilotselection = true;
                                                            thirdpilotselection = false;
                                                            break;
                                                        
                                                        
                                                        case 3:
                                                            firstpilotselection = false;
                                                            secondpilotselection = false;
                                                            thirdpilotselection = true;
                                                            break;

                                                        default:
                                                            break;
                                                        }

                                                        sleep(2);

                                                    }
                                                }
                                                

                                                
                                            
                                            }
                                        
                                        }
                                        
                                    }
                                  
        }
        
    pthread_mutex_unlock(&selection_mutex);
    }

}




// Signal handler to activate the flag
void sigint_handler(int signum)    
{
    write(0, "Hello from handler - volatile flag set to 1 \n", 46);
    myflag = 1; 
    
}



//Firstpilot thread

void *firstpilot_poll_client()
{
    //Poll for pilot server

    
    typedef uint32_t in_addr_t; 
    unsigned fd_count=1;
    int timeout = 1;
    
    // Needed for sendto() function
    
    
    
    // The poll structures are activated after defined socket connections

    mavlink_status_t mavlink_status;
    mavlink_message_t msg;
    int chan = MAVLINK_COMM_1; 
    int len;

   
    
    char buffer[MAVLINK_MAX_PACKET_LEN];
    
    
    //The cycle to poll and send messages
    while (!myflag)
    {   
        int ret = poll(&fds_firstpilot[0],fd_count,timeout);  
        if (ret < 0)
        {
            perror("poll firstpilot error\n");
            continue;
        }

        if (ret == 0 && timeout > 0)
        {
            perror("poll firstpilot timeout\n");
            continue;
        }      


        if (fds_firstpilot[0].revents & POLLIN)
        {
            len = recvfrom(fds_firstpilot[0].fd,buffer,sizeof(buffer),0,(struct sockaddr*)&remote_firstpilot_addr, &remote_firstpilot_addr_len);
            if (len > 0)
            {
                if (firstpilotselection == true) 
                {

                    for(unsigned k = 0; k < len; ++k)
                    {
                        if (mavlink_parse_char(MAVLINK_COMM_3,buffer[k],&msg,&mavlink_status))
                        {
                            int component_id=msg.compid;
                            switch (msg.msgid)
                            {

                                  //Added messages

                            case MAVLINK_MSG_ID_PARAM_EXT_VALUE:;
                                mavlink_param_ext_value_t ext_v;
                                mavlink_msg_param_ext_value_decode(&msg,&ext_v);
                                mavlink_msg_param_ext_value_encode(5,component_id,&msg,&ext_v);
                                break;
                            
                            case MAVLINK_MSG_ID_PARAM_EXT_ACK:;
                                mavlink_param_ext_ack_t ext_a;
                                mavlink_msg_param_ext_ack_decode(&msg,&ext_a);
                                mavlink_msg_param_ext_ack_encode(5,component_id,&msg,&ext_a);
                                break;

                            case MAVLINK_MSG_ID_PING:;
                                mavlink_ping_t ping;
                                mavlink_msg_ping_decode(&msg,&ping);
                                ping.target_system=255;
                                ping.target_component=190;
                                mavlink_msg_ping_encode(5,component_id,&msg,&ping);
                                break;

                            case MAVLINK_MSG_ID_VIBRATION:;
                                mavlink_vibration_t vibration;
                                mavlink_msg_vibration_decode(&msg,&vibration);
                                mavlink_msg_vibration_encode(5,component_id,&msg,&vibration);
                                break;

                            case MAVLINK_MSG_ID_VFR_HUD:;
                                mavlink_vfr_hud_t hud;
                                mavlink_msg_vfr_hud_decode(&msg,&hud);
                                mavlink_msg_vfr_hud_encode(5,component_id,&msg,&hud);
                                break;
                            
                            case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:;
                                mavlink_position_target_global_int_t ptg;
                                mavlink_msg_position_target_global_int_decode(&msg,&ptg);
                                mavlink_msg_position_target_global_int_encode(5,component_id,&msg,&ptg);
                                break;
                            
                            case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:;
                                mavlink_position_target_local_ned_t ptln;
                                mavlink_msg_position_target_local_ned_decode(&msg,&ptln);
                                mavlink_msg_position_target_local_ned_encode(5,component_id,&msg,&ptln);
                                break;

                            case MAVLINK_MSG_ID_SERVO_OUTPUT_RAW:;
                                mavlink_servo_output_raw_t servo;
                                mavlink_msg_servo_output_raw_decode(&msg,&servo);
                                mavlink_msg_servo_output_raw_encode(5,component_id,&msg,&servo);
                                break;

                            case MAVLINK_MSG_ID_SYS_STATUS:;
                                mavlink_sys_status_t sys;
                                mavlink_msg_sys_status_decode(&msg,&sys);
                                mavlink_msg_sys_status_encode(5,component_id,&msg,&sys);
                                break;

                            case MAVLINK_MSG_ID_UTM_GLOBAL_POSITION:;
                                mavlink_utm_global_position_t utm;
                                mavlink_msg_utm_global_position_decode(&msg,&utm);
                                mavlink_msg_utm_global_position_encode(5,component_id,&msg,&utm);
                                break;

                            case MAVLINK_MSG_ID_ALTITUDE:;
                                mavlink_altitude_t altitude;
                                mavlink_msg_altitude_decode(&msg,&altitude);
                                mavlink_msg_altitude_encode(5,component_id,&msg,&altitude);
                                break;

                            case MAVLINK_MSG_ID_ATTITUDE:;
                                mavlink_attitude_t attitude;
                                mavlink_msg_attitude_decode(&msg,&attitude);
                                mavlink_msg_attitude_encode(5,component_id,&msg,&attitude);
                                break;
                            
                            case MAVLINK_MSG_ID_ATTITUDE_QUATERNION:;
                                mavlink_attitude_quaternion_t attitude_quat;
                                mavlink_msg_attitude_quaternion_decode(&msg,&attitude_quat);
                                mavlink_msg_attitude_quaternion_encode(5,component_id,&msg,&attitude_quat);
                                break;

                            case MAVLINK_MSG_ID_ATTITUDE_TARGET:;
                                mavlink_attitude_target_t attitude_target;
                                mavlink_msg_attitude_target_decode(&msg,&attitude_target);
                                mavlink_msg_attitude_target_encode(5,component_id,&msg,&attitude_target);
                                break;

                            case MAVLINK_MSG_ID_BATTERY_STATUS:;
                                mavlink_battery_status_t battery_status;
                                mavlink_msg_battery_status_decode(&msg,&battery_status);
                                mavlink_msg_battery_status_encode(5,component_id,&msg,&battery_status);
                                break;

                            case MAVLINK_MSG_ID_ESTIMATOR_STATUS:;
                                mavlink_estimator_status_t estimator;
                                mavlink_msg_estimator_status_decode(&msg,&estimator);
                                mavlink_msg_estimator_status_encode(5,component_id,&msg,&estimator);
                                break;

                            case MAVLINK_MSG_ID_EXTENDED_SYS_STATE:;
                                mavlink_extended_sys_state_t extended;
                                mavlink_msg_extended_sys_state_decode(&msg,&extended);
                                mavlink_msg_extended_sys_state_encode(5,component_id,&msg,&extended);
                                break;

                            case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:;
                                mavlink_global_position_int_t global_p;
                                mavlink_msg_global_position_int_decode(&msg,&global_p);
                                mavlink_msg_global_position_int_encode(5,component_id,&msg,&global_p);
                                break;
                            case MAVLINK_MSG_ID_GPS_RAW_INT:;
                                mavlink_gps_raw_int_t gps_r;
                                mavlink_msg_gps_raw_int_decode(&msg,&gps_r);
                                mavlink_msg_gps_raw_int_encode(5,component_id,&msg,&gps_r);
                                break;

                            case MAVLINK_MSG_ID_HEARTBEAT:;
                                mavlink_heartbeat_t hb;
                                mavlink_msg_heartbeat_decode(&msg,&hb);
                                mavlink_msg_heartbeat_encode(5,component_id,&msg,&hb);
                                break;

                            case MAVLINK_MSG_ID_HOME_POSITION:;
                                mavlink_home_position_t hp;
                                mavlink_msg_home_position_decode(&msg,&hp);
                                mavlink_msg_home_position_encode(5,component_id,&msg,&hp);
                                break;

                            case MAVLINK_MSG_ID_LINK_NODE_STATUS:;
                                mavlink_link_node_status_t link_node;
                                mavlink_msg_link_node_status_decode(&msg,&link_node);
                                mavlink_msg_link_node_status_encode(5,component_id,&msg,&link_node);
                                break;

                            case MAVLINK_MSG_ID_LOCAL_POSITION_NED:;
                                mavlink_local_position_ned_t lpn;
                                mavlink_msg_local_position_ned_decode(&msg,&lpn);
                                mavlink_msg_local_position_ned_encode(5,component_id,&msg,&lpn);
                                break;

                            case MAVLINK_MSG_ID_MISSION_CURRENT:;
                                mavlink_mission_current_t mc;
                                mavlink_msg_mission_current_decode(&msg,&mc);
                                mavlink_msg_mission_current_encode(5,component_id,&msg,&mc);
                                break;

                            case MAVLINK_MSG_ID_MISSION_ITEM_REACHED:;
                                mavlink_mission_item_reached_t item_reached;
                                mavlink_msg_mission_item_reached_decode(&msg, &item_reached);
                                mavlink_msg_mission_item_reached_encode(5,component_id,&msg,&item_reached);
                                break;

                            case MAVLINK_MSG_ID_RC_CHANNELS:;
                                mavlink_rc_channels_t rc;
                                mavlink_msg_rc_channels_decode(&msg, &rc);
                                mavlink_msg_rc_channels_encode(5,component_id,&msg, &rc);
                                break;

                            case MAVLINK_MSG_ID_OPTICAL_FLOW:;
                                mavlink_optical_flow_t flow;
                                mavlink_msg_optical_flow_decode(&msg,&flow);
                                mavlink_msg_optical_flow_encode(5,component_id,&msg,&flow);
                                break;

                            case MAVLINK_MSG_ID_MISSION_ACK:;
                                mavlink_mission_ack_t wpa;
                                mavlink_msg_mission_ack_decode(&msg, &wpa);
                                wpa.target_system=255;
                                wpa.target_component=190;
                                mavlink_msg_mission_ack_encode(5,component_id,&msg,&wpa);
                                break;
                            
                            case MAVLINK_MSG_ID_MISSION_COUNT:;
                                mavlink_mission_count_t wpc_2;
                                mavlink_msg_mission_count_decode(&msg, &wpc_2);
                                wpc_2.target_system=255;
                                wpc_2.target_component=190;
                                mavlink_msg_mission_count_encode(5,component_id,&msg,&wpc_2);
                                break;

                            case MAVLINK_MSG_ID_MISSION_ITEM:;
                                mavlink_mission_item_t wp;
                                mavlink_msg_mission_item_decode(&msg, &wp);
                                wp.target_system=255;
                                wp.target_component=190;
                                mavlink_msg_mission_item_encode(5,component_id, &msg,&wp);
                                break;

                            case MAVLINK_MSG_ID_MISSION_ITEM_INT:;
                                mavlink_mission_item_t wp_1;
                                mavlink_msg_mission_item_decode(&msg, &wp_1);
                                wp_1.target_system=255;
                                wp_1.target_component=190;
                                mavlink_msg_mission_item_encode(5,component_id,&msg,&wp_1);
                                break;
                            

                            case MAVLINK_MSG_ID_MISSION_REQUEST:;
                                mavlink_mission_request_t wpr;
                                mavlink_msg_mission_request_decode(&msg, &wpr);
                                wpr.target_system=255;
                                wpr.target_component=190;
                                mavlink_msg_mission_request_encode(5,component_id,&msg,&wpr);
                                break;

                            case MAVLINK_MSG_ID_MISSION_REQUEST_INT:;
                                mavlink_mission_request_int_t wpr_1;
                                mavlink_msg_mission_request_int_decode(&msg, &wpr_1);
                                wpr_1.target_system=255;
                                wpr_1.target_component=190;
                                mavlink_msg_mission_request_int_encode(5,component_id,&msg,&wpr_1);
                                break;

                            case MAVLINK_MSG_ID_PARAM_VALUE:;
                                mavlink_param_value_t param_1;
                                mavlink_msg_param_value_decode(&msg,&param_1);
                                mavlink_msg_param_value_encode(5,component_id,&msg,&param_1);
                                break;

                            case MAVLINK_MSG_ID_EVENT:;
                                mavlink_event_t event_msg;
                                mavlink_msg_event_decode(&msg,&event_msg);
                                mavlink_msg_event_encode(5,component_id,&msg,&event_msg);
                                break;

                            case MAVLINK_MSG_ID_CURRENT_EVENT_SEQUENCE:;
                                mavlink_current_event_sequence_t current_event;
                                mavlink_msg_current_event_sequence_decode(&msg,&current_event);
                                mavlink_msg_current_event_sequence_encode(5,component_id,&msg,&current_event);
                                break;
                            
                            default:
                                break;
                            }
                            if (fds_qgc[0].fd > 0)
                            {
                                int ret = poll(&fds_qgc[0], fd_count, timeout);

                                if (ret < 0)
                                {
                                    perror("Pollout error on qgc\n");
                                    continue;
                                }

                                if (ret == 0 && timeout > 0)
                                {
                                    perror("Pollout timeout on qgc\n");
                                    continue;
                                }

                                if(!fds_qgc[0].revents & POLLOUT) 
                                {
                                    perror("Invalid events on the qgc file descriptor POLLOUT\n");
                                    continue;
                                }

                                ssize_t len2;
                                int packetlen = mavlink_msg_to_send_buffer(buffer, &msg);
                                if (fds_qgc[0].revents & POLLOUT)
                                {
                                    len2 = sendto(fds_qgc[0].fd,buffer,packetlen,0,(struct sockaddr *)&qgc_addr, qgc_addr_len); 
                                    if (len2 > 0)
                                    {
                                            printf("Message forwarded to the QGC from the first autopilot\n");
                                    }
                                        

                                }

                            }
                            
                          
                            
                                
                        }
                            
                    }
                }    
            }
        }

    }
}


//Secondpilot thread

void *secondpilot_poll_client()
{
    //Poll for the second pilot 

    
    typedef uint32_t in_addr_t; 
    unsigned fd_count=1;
    int timeout = 1;
    
    // Needed for sendto() function
    
    
    
    // The poll structures are activated after defined socket connections

    mavlink_status_t mavlink_status;
    mavlink_message_t msg;
    int chan = MAVLINK_COMM_2; 
    int len;

   
    
    char buffer[MAVLINK_MAX_PACKET_LEN];
    
    
    //The cycle to poll and send messages
    while (!myflag)
    {   
        int ret = poll(&fds_secondpilot[0],fd_count,timeout);  
        if (ret < 0)
        {
            perror("poll firstpilot error\n");
            continue;
        }

        if (ret == 0 && timeout > 0)
        {
            perror("poll firstpilot timeout\n");
            continue;
        }      


        if (fds_secondpilot[0].revents & POLLIN)
        {
            len = recvfrom(fds_secondpilot[0].fd,buffer,sizeof(buffer),0,(struct sockaddr*)&remote_secondpilot_addr, &remote_secondpilot_addr_len);
            if (len > 0)
            {
                if (secondpilotselection == true)
                {

                    for(unsigned k = 0; k < len; ++k)
                    {
                        if (mavlink_parse_char(MAVLINK_COMM_3,buffer[k],&msg,&mavlink_status))
                        {
                            int component_id = msg.compid;
                            switch (msg.msgid)
                            {


                            //Added messages

                            case MAVLINK_MSG_ID_PARAM_EXT_VALUE:;
                                mavlink_param_ext_value_t ext_v;
                                mavlink_msg_param_ext_value_decode(&msg,&ext_v);
                                mavlink_msg_param_ext_value_encode(5,component_id,&msg,&ext_v);
                                break;
                            
                            case MAVLINK_MSG_ID_PARAM_EXT_ACK:;
                                mavlink_param_ext_ack_t ext_a;
                                mavlink_msg_param_ext_ack_decode(&msg,&ext_a);
                                mavlink_msg_param_ext_ack_encode(5,component_id,&msg,&ext_a);
                                break;

                            case MAVLINK_MSG_ID_PING:;
                                mavlink_ping_t ping;
                                mavlink_msg_ping_decode(&msg,&ping);
                                ping.target_system=255;
                                ping.target_component=190;
                                mavlink_msg_ping_encode(5,component_id,&msg,&ping);
                                break;

                            case MAVLINK_MSG_ID_VIBRATION:;
                                mavlink_vibration_t vibration;
                                mavlink_msg_vibration_decode(&msg,&vibration);
                                mavlink_msg_vibration_encode(5,component_id,&msg,&vibration);
                                break;

                            case MAVLINK_MSG_ID_VFR_HUD:;
                                mavlink_vfr_hud_t hud;
                                mavlink_msg_vfr_hud_decode(&msg,&hud);
                                mavlink_msg_vfr_hud_encode(5,component_id,&msg,&hud);
                                break;
                            
                            case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:;
                                mavlink_position_target_global_int_t ptg;
                                mavlink_msg_position_target_global_int_decode(&msg,&ptg);
                                mavlink_msg_position_target_global_int_encode(5,component_id,&msg,&ptg);
                                break;
                            
                            case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:;
                                mavlink_position_target_local_ned_t ptln;
                                mavlink_msg_position_target_local_ned_decode(&msg,&ptln);
                                mavlink_msg_position_target_local_ned_encode(5,component_id,&msg,&ptln);
                                break;

                            case MAVLINK_MSG_ID_SERVO_OUTPUT_RAW:;
                                mavlink_servo_output_raw_t servo;
                                mavlink_msg_servo_output_raw_decode(&msg,&servo);
                                mavlink_msg_servo_output_raw_encode(5,component_id,&msg,&servo);
                                break;

                            case MAVLINK_MSG_ID_SYS_STATUS:;
                                mavlink_sys_status_t sys;
                                mavlink_msg_sys_status_decode(&msg,&sys);
                                mavlink_msg_sys_status_encode(5,component_id,&msg,&sys);
                                break;

                            case MAVLINK_MSG_ID_UTM_GLOBAL_POSITION:;
                                mavlink_utm_global_position_t utm;
                                mavlink_msg_utm_global_position_decode(&msg,&utm);
                                mavlink_msg_utm_global_position_encode(5,component_id,&msg,&utm);
                                break;

                            case MAVLINK_MSG_ID_ALTITUDE:;
                                mavlink_altitude_t altitude;
                                mavlink_msg_altitude_decode(&msg,&altitude);
                                mavlink_msg_altitude_encode(5,component_id,&msg,&altitude);
                                break;

                            case MAVLINK_MSG_ID_ATTITUDE:;
                                mavlink_attitude_t attitude;
                                mavlink_msg_attitude_decode(&msg,&attitude);
                                mavlink_msg_attitude_encode(5,component_id,&msg,&attitude);
                                break;
                            
                            case MAVLINK_MSG_ID_ATTITUDE_QUATERNION:;
                                mavlink_attitude_quaternion_t attitude_quat;
                                mavlink_msg_attitude_quaternion_decode(&msg,&attitude_quat);
                                mavlink_msg_attitude_quaternion_encode(5,component_id,&msg,&attitude_quat);
                                break;

                            case MAVLINK_MSG_ID_ATTITUDE_TARGET:;
                                mavlink_attitude_target_t attitude_target;
                                mavlink_msg_attitude_target_decode(&msg,&attitude_target);
                                mavlink_msg_attitude_target_encode(5,component_id,&msg,&attitude_target);
                                break;

                            case MAVLINK_MSG_ID_BATTERY_STATUS:;
                                mavlink_battery_status_t battery_status;
                                mavlink_msg_battery_status_decode(&msg,&battery_status);
                                mavlink_msg_battery_status_encode(5,component_id,&msg,&battery_status);
                                break;

                            case MAVLINK_MSG_ID_ESTIMATOR_STATUS:;
                                mavlink_estimator_status_t estimator;
                                mavlink_msg_estimator_status_decode(&msg,&estimator);
                                mavlink_msg_estimator_status_encode(5,component_id,&msg,&estimator);
                                break;

                            case MAVLINK_MSG_ID_EXTENDED_SYS_STATE:;
                                mavlink_extended_sys_state_t extended;
                                mavlink_msg_extended_sys_state_decode(&msg,&extended);
                                mavlink_msg_extended_sys_state_encode(5,component_id,&msg,&extended);
                                break;

                            case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:;
                                mavlink_global_position_int_t global_p;
                                mavlink_msg_global_position_int_decode(&msg,&global_p);
                                mavlink_msg_global_position_int_encode(5,component_id,&msg,&global_p);
                                break;
                            case MAVLINK_MSG_ID_GPS_RAW_INT:;
                                mavlink_gps_raw_int_t gps_r;
                                mavlink_msg_gps_raw_int_decode(&msg,&gps_r);
                                mavlink_msg_gps_raw_int_encode(5,component_id,&msg,&gps_r);
                                break;

                            case MAVLINK_MSG_ID_HEARTBEAT:;
                                mavlink_heartbeat_t hb;
                                mavlink_msg_heartbeat_decode(&msg,&hb);
                                mavlink_msg_heartbeat_encode(5,component_id,&msg,&hb);
                                break;

                            case MAVLINK_MSG_ID_HOME_POSITION:;
                                mavlink_home_position_t hp;
                                mavlink_msg_home_position_decode(&msg,&hp);
                                mavlink_msg_home_position_encode(5,component_id,&msg,&hp);
                                break;

                            case MAVLINK_MSG_ID_LINK_NODE_STATUS:;
                                mavlink_link_node_status_t link_node;
                                mavlink_msg_link_node_status_decode(&msg,&link_node);
                                mavlink_msg_link_node_status_encode(5,component_id,&msg,&link_node);
                                break;

                            case MAVLINK_MSG_ID_LOCAL_POSITION_NED:;
                                mavlink_local_position_ned_t lpn;
                                mavlink_msg_local_position_ned_decode(&msg,&lpn);
                                mavlink_msg_local_position_ned_encode(5,component_id,&msg,&lpn);
                                break;

                            case MAVLINK_MSG_ID_MISSION_CURRENT:;
                                mavlink_mission_current_t mc;
                                mavlink_msg_mission_current_decode(&msg,&mc);
                                mavlink_msg_mission_current_encode(5,component_id,&msg,&mc);
                                break;

                            case MAVLINK_MSG_ID_MISSION_ITEM_REACHED:;
                                mavlink_mission_item_reached_t item_reached;
                                mavlink_msg_mission_item_reached_decode(&msg, &item_reached);
                                mavlink_msg_mission_item_reached_encode(5,component_id,&msg,&item_reached);
                                break;

                            case MAVLINK_MSG_ID_RC_CHANNELS:;
                                mavlink_rc_channels_t rc;
                                mavlink_msg_rc_channels_decode(&msg, &rc);
                                mavlink_msg_rc_channels_encode(5,component_id,&msg, &rc);
                                break;

                            case MAVLINK_MSG_ID_OPTICAL_FLOW:;
                                mavlink_optical_flow_t flow;
                                mavlink_msg_optical_flow_decode(&msg,&flow);
                                mavlink_msg_optical_flow_encode(5,component_id,&msg,&flow);
                                break;

                            case MAVLINK_MSG_ID_MISSION_ACK:;
                                mavlink_mission_ack_t wpa;
                                mavlink_msg_mission_ack_decode(&msg, &wpa);
                                wpa.target_system=255;
                                wpa.target_component=190;
                                mavlink_msg_mission_ack_encode(5,component_id,&msg,&wpa);
                                break;
                            
                            case MAVLINK_MSG_ID_MISSION_COUNT:;
                                mavlink_mission_count_t wpc_2;
                                mavlink_msg_mission_count_decode(&msg, &wpc_2);
                                wpc_2.target_system=255;
                                wpc_2.target_component=190;
                                mavlink_msg_mission_count_encode(5,component_id,&msg,&wpc_2);
                                break;

                            case MAVLINK_MSG_ID_MISSION_ITEM:;
                                mavlink_mission_item_t wp;
                                mavlink_msg_mission_item_decode(&msg, &wp);
                                wp.target_system=255;
                                wp.target_component=190;
                                mavlink_msg_mission_item_encode(5,component_id, &msg,&wp);
                                break;

                            case MAVLINK_MSG_ID_MISSION_ITEM_INT:;
                                mavlink_mission_item_int_t wp_1;
                                mavlink_msg_mission_item_int_decode(&msg, &wp_1);
                                wp_1.target_system=255;
                                wp_1.target_component=190;
                                mavlink_msg_mission_item_int_encode(5,component_id,&msg,&wp_1);
                                break;
                            

                            case MAVLINK_MSG_ID_MISSION_REQUEST:;
                                mavlink_mission_request_t wpr;
                                mavlink_msg_mission_request_decode(&msg, &wpr);
                                wpr.target_system=255;
                                wpr.target_component=190;
                                mavlink_msg_mission_request_encode(5,component_id,&msg,&wpr);
                                break;

                            case MAVLINK_MSG_ID_MISSION_REQUEST_INT:;
                                mavlink_mission_request_int_t wpr_1;
                                mavlink_msg_mission_request_int_decode(&msg, &wpr_1);
                                wpr_1.target_system=255;
                                wpr_1.target_component=190;
                                mavlink_msg_mission_request_int_encode(5,component_id,&msg,&wpr_1);
                                break;

                            case MAVLINK_MSG_ID_PARAM_VALUE:;
                                mavlink_param_value_t param_1;
                                mavlink_msg_param_value_decode(&msg,&param_1);
                                mavlink_msg_param_value_encode(5,component_id,&msg,&param_1);
                                break;

                            case MAVLINK_MSG_ID_EVENT:;
                                mavlink_event_t event_msg;
                                mavlink_msg_event_decode(&msg,&event_msg);
                                mavlink_msg_event_encode(5,component_id,&msg,&event_msg);
                                break;

                            case MAVLINK_MSG_ID_CURRENT_EVENT_SEQUENCE:;
                                mavlink_current_event_sequence_t current_event;
                                mavlink_msg_current_event_sequence_decode(&msg,&current_event);
                                mavlink_msg_current_event_sequence_encode(5,component_id,&msg,&current_event);
                                break;
                            
                            default:
                                break;
                            }
                            
                            if (fds_qgc[0].fd > 0)
                            {
                                int ret = poll(&fds_qgc[0], fd_count, timeout);

                                if (ret < 0)
                                {
                                    perror("Pollout error on qgc\n");
                                    continue;
                                }

                                if (ret == 0 && timeout > 0)
                                {
                                    perror("Pollout timeout on qgc\n");
                                    continue;
                                }

                                if(!fds_qgc[0].revents & POLLOUT) 
                                {
                                    perror("Invalid events on the qgc file descriptor POLLOUT\n");
                                    continue;
                                }

                                ssize_t len2;
                                int packetlen = mavlink_msg_to_send_buffer(buffer, &msg);
                                
                                len2 = sendto(fds_qgc[0].fd,buffer,packetlen,0,(struct sockaddr *)&qgc_addr, qgc_addr_len); 
                                if (len2 > 0)
                                {
                                        printf("Message forwarded to the QGC from the second autopilot\n");
                                }    
                            }                               
                        }
                    }
                }    
            }
        }
    }
}




//Secondpilot thread

void *thirdpilot_poll_client()
{
    //Poll for the second pilot 

    
    typedef uint32_t in_addr_t; 
    unsigned fd_count=1;
    int timeout = 1;
    
    // Needed for sendto() function
    
    
    
    // The poll structures are activated after defined socket connections

    mavlink_status_t mavlink_status;
    mavlink_message_t msg;
    int chan = MAVLINK_COMM_3; 
    int len;

   
    
    char buffer[MAVLINK_MAX_PACKET_LEN];
    
    
    //The cycle to poll and send messages
    while (!myflag)
    {   
        int ret = poll(&fds_thirdpilot[0],fd_count,timeout);  
        if (ret < 0)
        {
            perror("poll firstpilot error\n");
            continue;
        }

        if (ret == 0 && timeout > 0)
        {
            perror("poll firstpilot timeout\n");
            continue;
        }      


        if (fds_thirdpilot[0].revents & POLLIN)
        {
            len = recvfrom(fds_thirdpilot[0].fd,buffer,sizeof(buffer),0,(struct sockaddr*)&remote_thirdpilot_addr, &remote_thirdpilot_addr_len);
            if (len > 0)
            {
                if (thirdpilotselection == true)
                {

                    for(unsigned k = 0; k < len; ++k)
                    {
                        if (mavlink_parse_char(MAVLINK_COMM_3,buffer[k],&msg,&mavlink_status))
                        {   
                            int component_id = msg.compid;
                            switch (msg.msgid)
                            {

                                  //Added messages

                            case MAVLINK_MSG_ID_PARAM_EXT_VALUE:;
                                mavlink_param_ext_value_t ext_v;
                                mavlink_msg_param_ext_value_decode(&msg,&ext_v);
                                mavlink_msg_param_ext_value_encode(5,component_id,&msg,&ext_v);
                                break;
                            
                            case MAVLINK_MSG_ID_PARAM_EXT_ACK:;
                                mavlink_param_ext_ack_t ext_a;
                                mavlink_msg_param_ext_ack_decode(&msg,&ext_a);
                                mavlink_msg_param_ext_ack_encode(5,component_id,&msg,&ext_a);
                                break;

                            case MAVLINK_MSG_ID_PING:;
                                mavlink_ping_t ping;
                                mavlink_msg_ping_decode(&msg,&ping);
                                ping.target_system=255;
                                ping.target_component=190;
                                mavlink_msg_ping_encode(5,component_id,&msg,&ping);
                                break;

                            case MAVLINK_MSG_ID_VIBRATION:;
                                mavlink_vibration_t vibration;
                                mavlink_msg_vibration_decode(&msg,&vibration);
                                mavlink_msg_vibration_encode(5,component_id,&msg,&vibration);
                                break;

                            case MAVLINK_MSG_ID_VFR_HUD:;
                                mavlink_vfr_hud_t hud;
                                mavlink_msg_vfr_hud_decode(&msg,&hud);
                                mavlink_msg_vfr_hud_encode(5,component_id,&msg,&hud);
                                break;
                            
                            case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:;
                                mavlink_position_target_global_int_t ptg;
                                mavlink_msg_position_target_global_int_decode(&msg,&ptg);
                                mavlink_msg_position_target_global_int_encode(5,component_id,&msg,&ptg);
                                break;
                            
                            case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:;
                                mavlink_position_target_local_ned_t ptln;
                                mavlink_msg_position_target_local_ned_decode(&msg,&ptln);
                                mavlink_msg_position_target_local_ned_encode(5,component_id,&msg,&ptln);
                                break;

                            case MAVLINK_MSG_ID_SERVO_OUTPUT_RAW:;
                                mavlink_servo_output_raw_t servo;
                                mavlink_msg_servo_output_raw_decode(&msg,&servo);
                                mavlink_msg_servo_output_raw_encode(5,component_id,&msg,&servo);
                                break;

                            case MAVLINK_MSG_ID_SYS_STATUS:;
                                mavlink_sys_status_t sys;
                                mavlink_msg_sys_status_decode(&msg,&sys);
                                mavlink_msg_sys_status_encode(5,component_id,&msg,&sys);
                                break;

                            case MAVLINK_MSG_ID_UTM_GLOBAL_POSITION:;
                                mavlink_utm_global_position_t utm;
                                mavlink_msg_utm_global_position_decode(&msg,&utm);
                                mavlink_msg_utm_global_position_encode(5,component_id,&msg,&utm);
                                break;

                            case MAVLINK_MSG_ID_ALTITUDE:;
                                mavlink_altitude_t altitude;
                                mavlink_msg_altitude_decode(&msg,&altitude);
                                mavlink_msg_altitude_encode(5,component_id,&msg,&altitude);
                                break;

                            case MAVLINK_MSG_ID_ATTITUDE:;
                                mavlink_attitude_t attitude;
                                mavlink_msg_attitude_decode(&msg,&attitude);
                                mavlink_msg_attitude_encode(5,component_id,&msg,&attitude);
                                break;
                            
                            case MAVLINK_MSG_ID_ATTITUDE_QUATERNION:;
                                mavlink_attitude_quaternion_t attitude_quat;
                                mavlink_msg_attitude_quaternion_decode(&msg,&attitude_quat);
                                mavlink_msg_attitude_quaternion_encode(5,component_id,&msg,&attitude_quat);
                                break;

                            case MAVLINK_MSG_ID_ATTITUDE_TARGET:;
                                mavlink_attitude_target_t attitude_target;
                                mavlink_msg_attitude_target_decode(&msg,&attitude_target);
                                mavlink_msg_attitude_target_encode(5,component_id,&msg,&attitude_target);
                                break;

                            case MAVLINK_MSG_ID_BATTERY_STATUS:;
                                mavlink_battery_status_t battery_status;
                                mavlink_msg_battery_status_decode(&msg,&battery_status);
                                mavlink_msg_battery_status_encode(5,component_id,&msg,&battery_status);
                                break;

                            case MAVLINK_MSG_ID_ESTIMATOR_STATUS:;
                                mavlink_estimator_status_t estimator;
                                mavlink_msg_estimator_status_decode(&msg,&estimator);
                                mavlink_msg_estimator_status_encode(5,component_id,&msg,&estimator);
                                break;

                            case MAVLINK_MSG_ID_EXTENDED_SYS_STATE:;
                                mavlink_extended_sys_state_t extended;
                                mavlink_msg_extended_sys_state_decode(&msg,&extended);
                                mavlink_msg_extended_sys_state_encode(5,component_id,&msg,&extended);
                                break;

                            case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:;
                                mavlink_global_position_int_t global_p;
                                mavlink_msg_global_position_int_decode(&msg,&global_p);
                                mavlink_msg_global_position_int_encode(5,component_id,&msg,&global_p);
                                break;
                            case MAVLINK_MSG_ID_GPS_RAW_INT:;
                                mavlink_gps_raw_int_t gps_r;
                                mavlink_msg_gps_raw_int_decode(&msg,&gps_r);
                                mavlink_msg_gps_raw_int_encode(5,component_id,&msg,&gps_r);
                                break;

                            case MAVLINK_MSG_ID_HEARTBEAT:;
                                mavlink_heartbeat_t hb;
                                mavlink_msg_heartbeat_decode(&msg,&hb);
                                mavlink_msg_heartbeat_encode(5,component_id,&msg,&hb);
                                break;

                            case MAVLINK_MSG_ID_HOME_POSITION:;
                                mavlink_home_position_t hp;
                                mavlink_msg_home_position_decode(&msg,&hp);
                                mavlink_msg_home_position_encode(5,component_id,&msg,&hp);
                                break;

                            case MAVLINK_MSG_ID_LINK_NODE_STATUS:;
                                mavlink_link_node_status_t link_node;
                                mavlink_msg_link_node_status_decode(&msg,&link_node);
                                mavlink_msg_link_node_status_encode(5,component_id,&msg,&link_node);
                                break;

                            case MAVLINK_MSG_ID_LOCAL_POSITION_NED:;
                                mavlink_local_position_ned_t lpn;
                                mavlink_msg_local_position_ned_decode(&msg,&lpn);
                                mavlink_msg_local_position_ned_encode(5,component_id,&msg,&lpn);
                                break;

                            case MAVLINK_MSG_ID_MISSION_CURRENT:;
                                mavlink_mission_current_t mc;
                                mavlink_msg_mission_current_decode(&msg,&mc);
                                mavlink_msg_mission_current_encode(5,component_id,&msg,&mc);
                                break;

                            case MAVLINK_MSG_ID_MISSION_ITEM_REACHED:;
                                mavlink_mission_item_reached_t item_reached;
                                mavlink_msg_mission_item_reached_decode(&msg, &item_reached);
                                mavlink_msg_mission_item_reached_encode(5,component_id,&msg,&item_reached);
                                break;

                            case MAVLINK_MSG_ID_RC_CHANNELS:;
                                mavlink_rc_channels_t rc;
                                mavlink_msg_rc_channels_decode(&msg, &rc);
                                mavlink_msg_rc_channels_encode(5,component_id,&msg, &rc);
                                break;

                            case MAVLINK_MSG_ID_OPTICAL_FLOW:;
                                mavlink_optical_flow_t flow;
                                mavlink_msg_optical_flow_decode(&msg,&flow);
                                mavlink_msg_optical_flow_encode(5,component_id,&msg,&flow);
                                break;

                            case MAVLINK_MSG_ID_MISSION_ACK:;
                                mavlink_mission_ack_t wpa;
                                mavlink_msg_mission_ack_decode(&msg, &wpa);
                                wpa.target_system=255;
                                wpa.target_component=190;
                                mavlink_msg_mission_ack_encode(5,component_id,&msg,&wpa);
                                break;
                            
                            case MAVLINK_MSG_ID_MISSION_COUNT:;
                                mavlink_mission_count_t wpc_2;
                                mavlink_msg_mission_count_decode(&msg, &wpc_2);
                                wpc_2.target_system=255;
                                wpc_2.target_component=190;
                                mavlink_msg_mission_count_encode(5,component_id,&msg,&wpc_2);
                                break;

                            case MAVLINK_MSG_ID_MISSION_ITEM:;
                                mavlink_mission_item_t wp;
                                mavlink_msg_mission_item_decode(&msg, &wp);
                                wp.target_system=255;
                                wp.target_component=190;
                                mavlink_msg_mission_item_encode(5,component_id, &msg,&wp);
                                break;

                            case MAVLINK_MSG_ID_MISSION_ITEM_INT:;
                                mavlink_mission_item_int_t wp_1;
                                mavlink_msg_mission_item_int_decode(&msg, &wp_1);
                                wp_1.target_system=255;
                                wp_1.target_component=190;
                                mavlink_msg_mission_item_int_encode(5,component_id,&msg,&wp_1);
                                break;
                            

                            case MAVLINK_MSG_ID_MISSION_REQUEST:;
                                mavlink_mission_request_t wpr;
                                mavlink_msg_mission_request_decode(&msg, &wpr);
                                wpr.target_system=255;
                                wpr.target_component=190;
                                mavlink_msg_mission_request_encode(5,component_id,&msg,&wpr);
                                break;

                            case MAVLINK_MSG_ID_MISSION_REQUEST_INT:;
                                mavlink_mission_request_t wpr_1;
                                mavlink_msg_mission_request_decode(&msg, &wpr_1);
                                wpr_1.target_system=255;
                                wpr_1.target_component=190;
                                mavlink_msg_mission_request_encode(5,component_id,&msg,&wpr_1);
                                break;

                            case MAVLINK_MSG_ID_PARAM_VALUE:;
                                mavlink_param_value_t param_1;
                                mavlink_msg_param_value_decode(&msg,&param_1);
                                mavlink_msg_param_value_encode(5,component_id,&msg,&param_1);
                                break;

                            case MAVLINK_MSG_ID_EVENT:;
                                mavlink_event_t event_msg;
                                mavlink_msg_event_decode(&msg,&event_msg);
                                mavlink_msg_event_encode(5,component_id,&msg,&event_msg);
                                break;

                            case MAVLINK_MSG_ID_CURRENT_EVENT_SEQUENCE:;
                                mavlink_current_event_sequence_t current_event;
                                mavlink_msg_current_event_sequence_decode(&msg,&current_event);
                                mavlink_msg_current_event_sequence_encode(5,component_id,&msg,&current_event);
                                break;
                            
                            default:
                                break;
                            }
                            if (fds_qgc[0].fd > 0)
                            {   
                                
                                int ret = poll(&fds_qgc[0], fd_count, timeout);

                                if (ret < 0)
                                {
                                    perror("Pollout error on qgc\n");
                                    continue;
                                }

                                if (ret == 0 && timeout > 0)
                                {
                                    perror("Pollout timeout on qgc\n");
                                    continue;
                                }

                                if(!fds_qgc[0].revents & POLLOUT) 
                                {
                                    perror("Invalid events on the qgc file descriptor POLLOUT\n");
                                    continue;
                                }

                                ssize_t len2;
                                int packetlen = mavlink_msg_to_send_buffer(buffer, &msg);
                                
                                
                                len2 = sendto(fds_qgc[0].fd,buffer,packetlen,0,(struct sockaddr *)&qgc_addr, qgc_addr_len); 
                                if (len2 > 0)
                                {
                                        printf("Message forwarded to the QGC from the third autopilot\n");
                                }
                            } 
                        } 
                    }
                }    
            }
        }
    }
}






void *qgc_poll_server()
{
    
    
    __socklen_t qgc_addr_len;
    typedef uint32_t in_addr_t;
    unsigned fd_count=1;
    int timeout = 1;
    char buff__[MAVLINK_MAX_PACKET_LEN];
    bool close_connection = false;



    mavlink_status_t mavlink_status;
    mavlink_message_t msg;
    int chan = MAVLINK_COMM_2; 
    uint16_t len;



    while(!myflag)
    {
     
        int ret = poll(&fds_qgc[0],fd_count,timeout);  
        if (ret < 0)
        {
            perror("poll error on qgc POLLIN file descriptor\n");
            continue;
        }

        if (ret == 0 && timeout > 0)
        {
            printf("Hello");
            perror("poll timeout on qgc POLLIN file descriptor\n");
            continue;
        }    

        memset(buff__, 0, MAVLINK_MAX_PACKET_LEN);  

        if(fds_qgc[0].revents & POLLIN)
        {
           
            int ret = recvfrom(fds_qgc[0].fd, buff__,sizeof(buff__),0,(struct sockaddr *)&qgc_addr,&qgc_addr_len);
                        
            if (ret < 0)
            {
                if (errno != EWOULDBLOCK) // disconnected from client 
                {
                    printf("recvfrom error : %s \n", strerror(errno));
                }
                continue;                
            }

            if (ret == 0) // Client QGC closed the connection orderly 
            {
                printf("Connection closed by client\n");
                close_connection = true;
                continue;
            }

            len = ret;

            for(unsigned k = 0; k < len; ++k)
            {
                if (mavlink_parse_char(MAVLINK_COMM_3,buff__[k],&msg,&mavlink_status))
                {   
                    printf("Received message from QGC, forward to the 3 autopilots\n");
                    
                    
                    
                    if (fds_firstpilot[0].fd > 0)
                    {
                        
                        int ret = poll(&fds_firstpilot[0], fd_count, timeout);

                        
                        if (ret < 0)
                        {
                            perror("Poll error on file descriptor POLLOUT on the first autopilot\n");
                            continue;
                        }

                        if (ret == 0 && timeout > 0)
                        {
                            printf("Poll timeout at the file descriptor POLLOUT on the first pilot\n");
                            continue;
                        }

                        if(!fds_firstpilot[0].revents & POLLOUT) 
                        {
                            printf("Invalid events at the file descriptor POLLOUT on the first pilot\n");
                            continue;
                        }

                       
                        int source_component_id = msg.compid;
                        int source_system_id = msg.sysid;
                        switch (msg.msgid) 
                        {
                            
                            case MAVLINK_MSG_ID_COMMAND_LONG:;
                                mavlink_command_long_t cmd_mavlink;
                                mavlink_msg_command_long_decode(&msg, &cmd_mavlink);
                                cmd_mavlink.target_system = 1;
                                mavlink_msg_command_long_encode(source_system_id,source_component_id,&msg, &cmd_mavlink);
                                break;
                                
                            case MAVLINK_MSG_ID_COMMAND_INT:;
                                mavlink_command_int_t cmd_mavlink_1;
                                mavlink_msg_command_int_decode(&msg, &cmd_mavlink_1);
                                cmd_mavlink.target_system =1;
                                mavlink_msg_command_int_encode(source_system_id,source_component_id, &msg, &cmd_mavlink_1);
                                break;

                            case MAVLINK_MSG_ID_COMMAND_ACK:;
                                mavlink_command_ack_t ack;
                                mavlink_msg_command_ack_decode(&msg, &ack);
                                ack.target_system=1;
                                mavlink_msg_command_ack_encode(source_system_id,source_component_id,&msg,&ack);
                                break;

                            case MAVLINK_MSG_ID_PING:;
                                mavlink_ping_t ping;
                                mavlink_msg_ping_decode(&msg, &ping);
                                ping.target_system=1;
                                mavlink_msg_ping_encode(source_system_id,source_component_id,&msg,&ping);
                                break;
                            
                            case MAVLINK_MSG_ID_SET_MODE:;
                                mavlink_set_mode_t new_mode;
                                mavlink_msg_set_mode_decode(&msg, &new_mode);
                                new_mode.target_system = 1;
                                mavlink_msg_set_mode_encode(source_system_id,source_component_id,&msg,&new_mode);
                                break;
                            
                            case MAVLINK_MSG_ID_SET_POSITION_TARGET_LOCAL_NED:;
                                mavlink_set_position_target_local_ned_t target_local_ned;
                                mavlink_msg_set_position_target_local_ned_decode(&msg, &target_local_ned);
                                target_local_ned.target_system=1;
                                mavlink_msg_set_position_target_local_ned_encode(source_system_id,source_component_id,&msg,&target_local_ned);
                                break;

                            case MAVLINK_MSG_ID_SET_POSITION_TARGET_GLOBAL_INT:;
                                mavlink_set_position_target_global_int_t target_global_int;
                                mavlink_msg_set_position_target_global_int_decode(&msg, &target_global_int);
                                target_global_int.target_system=1;
                                mavlink_msg_set_position_target_global_int_encode(source_system_id,source_component_id,&msg,&target_global_int);
                                break;

                            case MAVLINK_MSG_ID_SET_ATTITUDE_TARGET:;
                                mavlink_set_attitude_target_t attitude_target;
                                mavlink_msg_set_attitude_target_decode(&msg, &attitude_target);
                                attitude_target.target_system=1;
                                mavlink_msg_set_attitude_target_encode(source_system_id,source_component_id,&msg,&attitude_target);
                                break;

                            case MAVLINK_MSG_ID_SET_ACTUATOR_CONTROL_TARGET:;
                                mavlink_set_actuator_control_target_t actuator_target;
                                mavlink_msg_set_actuator_control_target_decode(&msg, &actuator_target);
                                actuator_target.target_system =1;
                                mavlink_msg_set_actuator_control_target_encode(source_system_id,source_component_id,&msg,&actuator_target);

                            case MAVLINK_MSG_ID_SET_GPS_GLOBAL_ORIGIN:;
                                mavlink_set_gps_global_origin_t gps_global_origin;
                                mavlink_msg_set_gps_global_origin_decode(&msg, &gps_global_origin);
                                gps_global_origin.target_system =1;
                                mavlink_msg_set_gps_global_origin_encode(source_system_id,source_component_id,&msg,&gps_global_origin);
                                break;

                            case MAVLINK_MSG_ID_MANUAL_CONTROL:;
                                mavlink_manual_control_t man_1;
                                mavlink_msg_manual_control_decode(&msg, &man_1);
                                man_1.target=1;
                                mavlink_msg_manual_control_encode(source_system_id,source_component_id,&msg,&man_1);
                                break;

                            case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:;
                                mavlink_rc_channels_override_t man;
                                mavlink_msg_rc_channels_override_decode(&msg, &man);
                                man.target_system=1;
                                mavlink_msg_rc_channels_override_encode(source_system_id,source_component_id,&msg,&man);
                                break;

                            case MAVLINK_MSG_ID_LANDING_TARGET:;
                                mavlink_landing_target_t landing_target;
                                mavlink_msg_landing_target_decode(&msg, &landing_target);
                                mavlink_msg_landing_target_encode(source_system_id,source_component_id,&msg,&landing_target);
                                break;
                            
                            case MAVLINK_MSG_ID_BATTERY_STATUS:;
                                mavlink_battery_status_t battery_mavlink;
                                mavlink_msg_battery_status_decode(&msg, &battery_mavlink);
                                mavlink_msg_battery_status_encode(source_system_id,source_component_id,&msg,&battery_mavlink);
                                break;

                            case MAVLINK_MSG_ID_PLAY_TUNE:;
                                mavlink_play_tune_t play_tune;
                                mavlink_msg_play_tune_decode(&msg, &play_tune);
                                play_tune.target_system=1;
                                mavlink_msg_play_tune_encode(source_component_id,source_component_id,&msg,&play_tune);
                                break;
                            
                            case MAVLINK_MSG_ID_PLAY_TUNE_V2:;
                                mavlink_play_tune_v2_t play_tune_v2;
                                mavlink_msg_play_tune_v2_decode(&msg, &play_tune_v2);
                                play_tune_v2.target_system=1;
                                mavlink_msg_play_tune_v2_encode(source_system_id,source_component_id,&msg,&play_tune_v2);
                                break;

                            //Mission manager


                            case MAVLINK_MSG_ID_MISSION_ACK:;
                                mavlink_mission_ack_t wpa;
                                mavlink_msg_mission_ack_decode(&msg, &wpa);
                                wpa.target_system=1;
                                mavlink_msg_mission_ack_encode(source_system_id,source_component_id,&msg,&wpa);
                                break;

                            case MAVLINK_MSG_ID_MISSION_SET_CURRENT:;      
                                mavlink_mission_set_current_t wpc;
                                mavlink_msg_mission_set_current_decode(&msg, &wpc);
                                wpc.target_system=1;
                                mavlink_msg_mission_set_current_encode(source_system_id,source_component_id,&msg,&wpc);
                                break;

                            case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:;
                                mavlink_mission_request_list_t wprl;
                                mavlink_msg_mission_request_list_decode(&msg, &wprl);
                                wprl.target_system=1;
                                mavlink_msg_mission_request_list_encode(source_system_id,source_component_id,&msg,&wprl);
                                break;

                            case MAVLINK_MSG_ID_MISSION_REQUEST:;
                                mavlink_mission_request_t wpr;
                                mavlink_msg_mission_request_decode(&msg, &wpr);
                                wpr.target_system=1;
                                mavlink_msg_mission_request_encode(source_system_id,source_component_id,&msg,&wpr);
                                break;

                            case MAVLINK_MSG_ID_MISSION_REQUEST_INT:;
                                mavlink_mission_request_t wpr_1;
                                mavlink_msg_mission_request_decode(&msg, &wpr_1);
                                wpr_1.target_system=1;
                                mavlink_msg_mission_request_encode(source_system_id,source_component_id,&msg,&wpr_1);
                                break;

                            case MAVLINK_MSG_ID_MISSION_COUNT:;
                                mavlink_mission_count_t wpc_2;
                                mavlink_msg_mission_count_decode(&msg, &wpc_2);
                                wpc_2.target_system=1;
                                mavlink_msg_mission_count_encode(source_system_id,source_component_id,&msg,&wpc_2);
                                break;

                            case MAVLINK_MSG_ID_MISSION_ITEM:;
                                mavlink_mission_item_t wp;
                                mavlink_msg_mission_item_decode(&msg, &wp);
                                wp.target_system=1;
                                mavlink_msg_mission_item_encode(source_system_id,source_component_id, &msg,&wp);
                                break;

                            case MAVLINK_MSG_ID_MISSION_ITEM_INT:;
                                mavlink_mission_item_int_t wp_1;
                                mavlink_msg_mission_item_int_decode(&msg, &wp_1);
                                wp_1.target_system=1;
                                mavlink_msg_mission_item_int_encode(source_system_id,source_component_id,&msg,&wp_1);
                                break;


                            case MAVLINK_MSG_ID_MISSION_CLEAR_ALL:;
                                mavlink_mission_clear_all_t wpca;
                                mavlink_msg_mission_clear_all_decode(&msg, &wpca);
                                wpca.target_system=1;
                                mavlink_msg_mission_clear_all_encode(source_system_id,source_component_id,&msg,&wpca);
                                break;

                            case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:; 
                                mavlink_param_request_list_t req_list;
                                mavlink_msg_param_request_list_decode(&msg, &req_list);
                                req_list.target_system = 1;
                                mavlink_msg_param_request_list_encode(source_system_id,source_component_id,&msg,&req_list);
                                break;

                            case MAVLINK_MSG_ID_REQUEST_EVENT:;
                                mavlink_request_event_t request_event;
                                mavlink_msg_request_event_decode(&msg, &request_event);
                                request_event.target_system = 1;
                                mavlink_msg_request_event_encode(source_system_id,source_component_id,&msg,&request_event);
                                break;

                            default:
                            break;
                        }


                            

                        ssize_t len2;
                        int packetlen = mavlink_msg_to_send_buffer(buff__, &msg);

                        len2 = sendto(fds_firstpilot[0].fd,buff__,packetlen,0,(struct sockaddr *)&remote_firstpilot_addr, remote_firstpilot_addr_len);   
                        
                        if (len2 > 0)
                        {
                            printf("Message forwarded to the first autopilot\n");
                        }
                    }
                    
                        
                    if (fds_secondpilot[0].fd > 0)
                    {
                        int ret = poll(&fds_secondpilot[0], fd_count, timeout);
                        
                        
                        if (ret < 0)
                        {
                            perror("Poll error on filede descriptor POLLOUT on the second autopilot\n");
                            continue;
                        }

                        if (ret == 0 && timeout > 0)
                        {
                            printf("Poll timeout at the file descriptor POLLOUT on the second pilot\n");
                            continue;
                        }

                        if(!fds_secondpilot[0].revents & POLLOUT) 
                        {
                            printf("Invalid events at the file descriptor POLLOUT on the second pilot\n");
                            continue;
                        }
                        
                        int source_component_id = msg.compid;
                        int source_system_id = msg.sysid;
                        switch (msg.msgid) 
                        {
                            
                            case MAVLINK_MSG_ID_COMMAND_LONG:;
                                mavlink_command_long_t cmd_mavlink;
                                mavlink_msg_command_long_decode(&msg, &cmd_mavlink);
                                cmd_mavlink.target_system = 2;
                                mavlink_msg_command_long_encode(source_system_id,source_component_id,&msg, &cmd_mavlink);
                                break;
                            

                           //ADDED ID MESSAGES
                            case MAVLINK_MSG_ID_PARAM_EXT_REQUEST_READ:;
                                mavlink_param_ext_request_read_t rd;
                                mavlink_msg_param_ext_request_read_decode(&msg,&rd);
                                rd.target_system=2;
                                mavlink_msg_param_ext_request_read_encode(source_system_id,source_component_id, &msg, &rd);
                                break;

                            case MAVLINK_MSG_ID_PARAM_EXT_SET:;
                                mavlink_param_ext_set_t ext_s;
                                mavlink_msg_param_ext_set_decode(&msg,&ext_s);
                                ext_s.target_system=2;
                                mavlink_msg_param_ext_set_encode(source_system_id,source_component_id,&msg,&ext_s);
                                break;
                            //END ADDED

                            case MAVLINK_MSG_ID_COMMAND_INT:;
                                mavlink_command_int_t cmd_mavlink_1;
                                mavlink_msg_command_int_decode(&msg, &cmd_mavlink_1);
                                cmd_mavlink_1.target_system =2;
                                mavlink_msg_command_int_encode(source_system_id,source_component_id, &msg, &cmd_mavlink_1);
                                break;
                            
                            
                            
                            case MAVLINK_MSG_ID_COMMAND_ACK:;
                                mavlink_command_ack_t ack;
                                mavlink_msg_command_ack_decode(&msg, &ack);
                                ack.target_system=2;
                                mavlink_msg_command_ack_encode(source_system_id,source_component_id,&msg,&ack);
                                break;

                            case MAVLINK_MSG_ID_PING:;
                                mavlink_ping_t ping;
                                mavlink_msg_ping_decode(&msg, &ping);
                                ping.target_system=2;
                                mavlink_msg_ping_encode(source_system_id,source_component_id,&msg,&ping);
                                break;
                            
                            case MAVLINK_MSG_ID_SET_MODE:;
                                mavlink_set_mode_t new_mode;
                                mavlink_msg_set_mode_decode(&msg, &new_mode);
                                new_mode.target_system = 2;
                                mavlink_msg_set_mode_encode(source_system_id,source_component_id,&msg,&new_mode);
                                break;
                            
                            case MAVLINK_MSG_ID_SET_POSITION_TARGET_LOCAL_NED:;
                                mavlink_set_position_target_local_ned_t target_local_ned;
                                mavlink_msg_set_position_target_local_ned_decode(&msg, &target_local_ned);
                                target_local_ned.target_system=2;
                                mavlink_msg_set_position_target_local_ned_encode(source_system_id,source_component_id,&msg,&target_local_ned);
                                break;

                            case MAVLINK_MSG_ID_SET_POSITION_TARGET_GLOBAL_INT:;
                                mavlink_set_position_target_global_int_t target_global_int;
                                mavlink_msg_set_position_target_global_int_decode(&msg, &target_global_int);
                                target_global_int.target_system=2;
                                mavlink_msg_set_position_target_global_int_encode(source_system_id,source_component_id,&msg,&target_global_int);
                                break;

                            case MAVLINK_MSG_ID_SET_ATTITUDE_TARGET:;
                                mavlink_set_attitude_target_t attitude_target;
                                mavlink_msg_set_attitude_target_decode(&msg, &attitude_target);
                                attitude_target.target_system=2;
                                mavlink_msg_set_attitude_target_encode(source_system_id,source_component_id,&msg,&attitude_target);
                                break;

                            case MAVLINK_MSG_ID_SET_ACTUATOR_CONTROL_TARGET:;
                                mavlink_set_actuator_control_target_t actuator_target;
                                mavlink_msg_set_actuator_control_target_decode(&msg, &actuator_target);
                                actuator_target.target_system =2;
                                mavlink_msg_set_actuator_control_target_encode(source_system_id,source_component_id,&msg,&actuator_target);

                            case MAVLINK_MSG_ID_SET_GPS_GLOBAL_ORIGIN:;
                                mavlink_set_gps_global_origin_t gps_global_origin;
                                mavlink_msg_set_gps_global_origin_decode(&msg, &gps_global_origin);
                                gps_global_origin.target_system =2;
                                mavlink_msg_set_gps_global_origin_encode(source_system_id,source_component_id,&msg,&gps_global_origin);
                                break;

                            case MAVLINK_MSG_ID_MANUAL_CONTROL:;
                                mavlink_manual_control_t man;
                                mavlink_msg_manual_control_decode(&msg, &man);
                                man.target=2;
                                mavlink_msg_manual_control_encode(source_system_id,source_component_id,&msg,&man);
                                break;

                            case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:;
                                mavlink_rc_channels_override_t man_1;
                                mavlink_msg_rc_channels_override_decode(&msg, &man_1);
                                man_1.target_system=2;
                                mavlink_msg_rc_channels_override_encode(source_system_id,source_component_id,&msg,&man_1);
                                break;

                            case MAVLINK_MSG_ID_LANDING_TARGET:;
                                mavlink_landing_target_t landing_target;
                                mavlink_msg_landing_target_decode(&msg, &landing_target);
                                mavlink_msg_landing_target_encode(source_system_id,source_component_id,&msg,&landing_target);
                                break;
                            
                            case MAVLINK_MSG_ID_BATTERY_STATUS:;
                                mavlink_battery_status_t battery_mavlink;
                                mavlink_msg_battery_status_decode(&msg, &battery_mavlink);
                                mavlink_msg_battery_status_encode(source_system_id,source_component_id,&msg,&battery_mavlink);
                                break;

                            case MAVLINK_MSG_ID_PLAY_TUNE:;
                                mavlink_play_tune_t play_tune;
                                mavlink_msg_play_tune_decode(&msg, &play_tune);
                                play_tune.target_system=2;
                                mavlink_msg_play_tune_encode(source_system_id,source_component_id,&msg,&play_tune);
                                break;
                            
                            case MAVLINK_MSG_ID_PLAY_TUNE_V2:;
                                mavlink_play_tune_v2_t play_tune_v2;
                                mavlink_msg_play_tune_v2_decode(&msg, &play_tune_v2);
                                play_tune_v2.target_system=2;
                                mavlink_msg_play_tune_v2_encode(source_system_id,source_component_id,&msg,&play_tune_v2);
                                break;


                            //Mission manager


                            case MAVLINK_MSG_ID_MISSION_ACK:;
                                mavlink_mission_ack_t wpa;
                                mavlink_msg_mission_ack_decode(&msg, &wpa);
                                wpa.target_system=2;
                                mavlink_msg_mission_ack_encode(source_system_id,source_component_id,&msg,&wpa);
                                break;

                            case MAVLINK_MSG_ID_MISSION_SET_CURRENT:;      
                                mavlink_mission_set_current_t wpc;
                                mavlink_msg_mission_set_current_decode(&msg, &wpc);
                                wpc.target_system=2;    
                                mavlink_msg_mission_set_current_encode(source_system_id,source_component_id,&msg,&wpc);
                                break;

                            case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:;
                                mavlink_mission_request_list_t wprl;
                                mavlink_msg_mission_request_list_decode(&msg, &wprl);
                                wprl.target_system=2;
                                mavlink_msg_mission_request_list_encode(source_system_id,source_component_id,&msg,&wprl);
                                break;

                            
                            case MAVLINK_MSG_ID_MISSION_REQUEST:;
                                mavlink_mission_request_t wpr;
                                mavlink_msg_mission_request_decode(&msg, &wpr);
                                wpr.target_system=2;
                                mavlink_msg_mission_request_encode(source_system_id,source_component_id,&msg,&wpr);
                                break;

                            

                            case MAVLINK_MSG_ID_MISSION_REQUEST_INT:;
		                    mavlink_mission_request_int_t wpr_1;
                                mavlink_msg_mission_request_int_decode(&msg, &wpr_1);
                                wpr_1.target_system=2;
                                mavlink_msg_mission_request_int_encode(source_system_id,source_component_id,&msg,&wpr_1);
                                break;

                            case MAVLINK_MSG_ID_MISSION_COUNT:;
                            mavlink_mission_count_t wpc_2;
	                        mavlink_msg_mission_count_decode(&msg, &wpc_2);
                            wpc_2.target_system=2;
                            mavlink_msg_mission_count_encode(source_system_id,source_component_id,&msg,&wpc_2);
                            break;

                            //Changed this
                            case MAVLINK_MSG_ID_MISSION_ITEM:;
                                mavlink_mission_item_t wp;
                                mavlink_msg_mission_item_decode(&msg, &wp);
                                wp.target_system=2;
                                mavlink_msg_mission_item_encode(source_system_id,source_component_id, &msg,&wp);
                                break;
                            
                            

                            case MAVLINK_MSG_ID_MISSION_CURRENT:;
                                mavlink_mission_current_t mc_1;
                                mavlink_msg_mission_current_decode(&msg,&mc_1);
                                mavlink_msg_mission_current_encode(source_component_id, source_component_id,&msg,&mc_1);
                                break;

                            case MAVLINK_MSG_ID_MISSION_ITEM_INT:;
                                mavlink_mission_item_int_t wp_1;
                                mavlink_msg_mission_item_int_decode(&msg, &wp_1);
                                wp_1.target_system=2;
                                mavlink_msg_mission_item_int_encode(source_system_id,source_component_id,&msg,&wp_1);
                                break;


                            case MAVLINK_MSG_ID_MISSION_CLEAR_ALL:;
                                mavlink_mission_clear_all_t wpca;
                                mavlink_msg_mission_clear_all_decode(&msg, &wpca);
                                wpca.target_system=2;
                                mavlink_msg_mission_clear_all_encode(source_system_id,source_component_id,&msg,&wpca);
                                break;

                            case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:; 
                                mavlink_param_request_list_t req_list;
                                mavlink_msg_param_request_list_decode(&msg, &req_list);
                                req_list.target_system = 2;
                                mavlink_msg_param_request_list_encode(source_system_id,source_component_id,&msg,&req_list);
                                break;

                            case MAVLINK_MSG_ID_REQUEST_EVENT:;
                                mavlink_request_event_t request_event;
                                mavlink_msg_request_event_decode(&msg, &request_event);
                                request_event.target_system = 2;
                                mavlink_msg_request_event_encode(source_system_id,source_component_id,&msg,&request_event);
                                break;

                            default:
                                break;
                            }

                        ssize_t len2;
                        int packetlen = mavlink_msg_to_send_buffer(buff__, &msg);

                        len2 = sendto(fds_secondpilot[0].fd,buff__,packetlen,0,(struct sockaddr *)&remote_secondpilot_addr, remote_secondpilot_addr_len);   
                        
                        if (len2 > 0)
                        {
                            printf("Message forwarded to the second autopilot\n");
                        }

                    }

                    if (fds_thirdpilot[0].fd > 0)
                    {
                        int ret = poll(&fds_thirdpilot[0], fd_count, timeout);
                        
                        
                        if (ret < 0)
                        {
                            perror("Poll error on filede descriptor POLLOUT on the third autopilot\n");
                            continue;
                        }

                        if (ret == 0 && timeout > 0)
                        {
                            printf("Poll timeout at the file descriptor POLLOUT on the third pilot\n");
                            continue;
                        }

                        if(!fds_thirdpilot[0].revents & POLLOUT) 
                        {
                            printf("Invalid events at the file descriptor POLLOUT on the third pilot\n");
                            continue;
                        }
                        
                        int source_component_id = msg.compid;
                        int source_system_id = msg.sysid;
                        
                        switch (msg.msgid) 
                        {   
                            
                            case MAVLINK_MSG_ID_COMMAND_LONG:;
                                mavlink_command_long_t cmd_mavlink;
                                mavlink_msg_command_long_decode(&msg, &cmd_mavlink);
                                cmd_mavlink.target_system = 3;
                                mavlink_msg_command_long_encode(source_system_id,source_component_id,&msg, &cmd_mavlink);
                                break;
                               
                             //ADDED ID MESSAGES
                            case MAVLINK_MSG_ID_PARAM_EXT_REQUEST_READ:;
                                mavlink_param_ext_request_read_t rd;
                                mavlink_msg_param_ext_request_read_decode(&msg,&rd);
                                rd.target_system=2;
                                mavlink_msg_param_ext_request_read_encode(source_system_id,source_component_id, &msg, &rd);
                                break;

                            case MAVLINK_MSG_ID_PARAM_EXT_SET:;
                                mavlink_param_ext_set_t ext_s;
                                mavlink_msg_param_ext_set_decode(&msg,&ext_s);
                                ext_s.target_system=2;
                                mavlink_msg_param_ext_set_encode(source_system_id,source_component_id,&msg,&ext_s);
                                break;

                            case MAVLINK_MSG_ID_COMMAND_INT:;
                                mavlink_command_int_t cmd_mavlink_1;
                                mavlink_msg_command_int_decode(&msg, &cmd_mavlink_1);
                                cmd_mavlink_1.target_system =3;
                                mavlink_msg_command_int_encode(source_system_id,source_component_id, &msg, &cmd_mavlink_1);
                                break;

                            case MAVLINK_MSG_ID_COMMAND_ACK:;
                                mavlink_command_ack_t ack;
                                mavlink_msg_command_ack_decode(&msg, &ack);
                                ack.target_system=3;
                                mavlink_msg_command_ack_encode(source_system_id,source_component_id,&msg,&ack);
                                break;

                            case MAVLINK_MSG_ID_PING:;
                                mavlink_ping_t ping;
                                mavlink_msg_ping_decode(&msg, &ping);
                                ping.target_system=3;
                                mavlink_msg_ping_encode(source_system_id,source_component_id,&msg,&ping);
                                break;
                            
                            case MAVLINK_MSG_ID_SET_MODE:;
                                mavlink_set_mode_t new_mode;
                                mavlink_msg_set_mode_decode(&msg, &new_mode);
                                new_mode.target_system = 3;
                                mavlink_msg_set_mode_encode(source_system_id,source_component_id,&msg,&new_mode);
                                break;
                            
                            case MAVLINK_MSG_ID_SET_POSITION_TARGET_LOCAL_NED:;
                                mavlink_set_position_target_local_ned_t target_local_ned;
                                mavlink_msg_set_position_target_local_ned_decode(&msg, &target_local_ned);
                                target_local_ned.target_system=3;
                                mavlink_msg_set_position_target_local_ned_encode(source_system_id,source_component_id,&msg,&target_local_ned);
                                break;

                            case MAVLINK_MSG_ID_SET_POSITION_TARGET_GLOBAL_INT:;
                                mavlink_set_position_target_global_int_t target_global_int;
                                mavlink_msg_set_position_target_global_int_decode(&msg, &target_global_int);
                                target_global_int.target_system=3;
                                mavlink_msg_set_position_target_global_int_encode(source_system_id,source_component_id,&msg,&target_global_int);
                                break;

                            case MAVLINK_MSG_ID_SET_ATTITUDE_TARGET:;
                                mavlink_set_attitude_target_t attitude_target;
                                mavlink_msg_set_attitude_target_decode(&msg, &attitude_target);
                                attitude_target.target_system=3;
                                mavlink_msg_set_attitude_target_encode(source_system_id,source_component_id,&msg,&attitude_target);
                                break;

                            case MAVLINK_MSG_ID_SET_ACTUATOR_CONTROL_TARGET:;
                                mavlink_set_actuator_control_target_t actuator_target;
                                mavlink_msg_set_actuator_control_target_decode(&msg, &actuator_target);
                                actuator_target.target_system =3;
                                mavlink_msg_set_actuator_control_target_encode(source_system_id,source_component_id,&msg,&actuator_target);

                            case MAVLINK_MSG_ID_SET_GPS_GLOBAL_ORIGIN:;
                                mavlink_set_gps_global_origin_t gps_global_origin;
                                mavlink_msg_set_gps_global_origin_decode(&msg, &gps_global_origin);
                                gps_global_origin.target_system =3;
                                mavlink_msg_set_gps_global_origin_encode(source_system_id,source_component_id,&msg,&gps_global_origin);
                                break;

                            case MAVLINK_MSG_ID_MANUAL_CONTROL:;
                                mavlink_manual_control_t man;
                                mavlink_msg_manual_control_decode(&msg, &man);
                                man.target=3;
                                mavlink_msg_manual_control_encode(source_system_id,source_component_id,&msg,&man);
                                break;

                            case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:;
                                mavlink_rc_channels_override_t man_1;
                                mavlink_msg_rc_channels_override_decode(&msg, &man_1);
                                man_1.target_system=3;
                                mavlink_msg_rc_channels_override_encode(source_system_id,source_component_id,&msg,&man_1);
                                break;

                            case MAVLINK_MSG_ID_LANDING_TARGET:;
                                mavlink_landing_target_t landing_target;
                                mavlink_msg_landing_target_decode(&msg, &landing_target);
                                mavlink_msg_landing_target_encode(source_system_id,source_component_id,&msg,&landing_target);
                                break;
                            
                            case MAVLINK_MSG_ID_BATTERY_STATUS:;
                                mavlink_battery_status_t battery_mavlink;
                                mavlink_msg_battery_status_decode(&msg, &battery_mavlink);
                                mavlink_msg_battery_status_encode(source_system_id,source_component_id,&msg,&battery_mavlink);
                                break;

                            case MAVLINK_MSG_ID_PLAY_TUNE:;
                                mavlink_play_tune_t play_tune;
                                mavlink_msg_play_tune_decode(&msg, &play_tune);
                                play_tune.target_system=3;
                                mavlink_msg_play_tune_encode(source_system_id,source_component_id,&msg,&play_tune);
                                break;
                            
                            case MAVLINK_MSG_ID_PLAY_TUNE_V2:;
                                mavlink_play_tune_v2_t play_tune_v2;
                                mavlink_msg_play_tune_v2_decode(&msg, &play_tune_v2);
                                play_tune_v2.target_system=3;
                                mavlink_msg_play_tune_v2_encode(source_system_id,source_component_id,&msg,&play_tune_v2);
                                break;

                            //Mission manager


                            case MAVLINK_MSG_ID_MISSION_ACK:;
		                    mavlink_mission_ack_t wpa;
	                        mavlink_msg_mission_ack_decode(&msg, &wpa);
                            wpa.target_system=3;
                            mavlink_msg_mission_ack_encode(source_system_id,source_component_id,&msg,&wpa);
		                    break;

                            case MAVLINK_MSG_ID_MISSION_SET_CURRENT:;      
	                        mavlink_mission_set_current_t wpc;
	                        mavlink_msg_mission_set_current_decode(&msg, &wpc);
		                    wpc.target_system=3;
                            mavlink_msg_mission_set_current_encode(source_system_id,source_component_id,&msg,&wpc);
                            break;

                            case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:;
		                    mavlink_mission_request_list_t wprl;
	                        mavlink_msg_mission_request_list_decode(&msg, &wprl);
                            wprl.target_system=3;
                            mavlink_msg_mission_request_list_encode(source_system_id,source_component_id,&msg,&wprl);
		                    break;

                            case MAVLINK_MSG_ID_MISSION_REQUEST:;
		                    mavlink_mission_request_t wpr;
	                        mavlink_msg_mission_request_decode(&msg, &wpr);
                            wpr.target_system=3;
                            mavlink_msg_mission_request_encode(source_system_id,source_component_id,&msg,&wpr);
		                    break;

                            case MAVLINK_MSG_ID_MISSION_REQUEST_INT:;
		                    mavlink_mission_request_t wpr_1;
	                        mavlink_msg_mission_request_decode(&msg, &wpr_1);
                            wpr_1.target_system=3;
                            mavlink_msg_mission_request_encode(source_system_id,source_component_id,&msg,&wpr_1);
		                    break;

                            case MAVLINK_MSG_ID_MISSION_COUNT:;
                            mavlink_mission_count_t wpc_2;
	                        mavlink_msg_mission_count_decode(&msg, &wpc_2);
                            wpc_2.target_system=3;
                            mavlink_msg_mission_count_encode(source_system_id,source_component_id,&msg,&wpc_2);
                            break;

                            case MAVLINK_MSG_ID_MISSION_ITEM:;
		                    mavlink_mission_item_t wp;
	                        mavlink_msg_mission_item_decode(&msg, &wp);
                            wp.target_system=3;
                            mavlink_msg_mission_item_encode(source_system_id,source_component_id, &msg,&wp);
		                    break;

                            case MAVLINK_MSG_ID_MISSION_ITEM_INT:;
		                    mavlink_mission_item_int_t wp_1;
	                        mavlink_msg_mission_item_int_decode(&msg, &wp_1);
                            wp_1.target_system=3;
                            mavlink_msg_mission_item_int_encode(source_system_id,source_component_id,&msg,&wp_1);
		                    break;


                            case MAVLINK_MSG_ID_MISSION_CLEAR_ALL:;
                            mavlink_mission_clear_all_t wpca;
	                        mavlink_msg_mission_clear_all_decode(&msg, &wpca);
                            wpca.target_system=3;
                            mavlink_msg_mission_clear_all_encode(source_system_id,source_component_id,&msg,&wpca);
                            break;

                            case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:; 
		                	mavlink_param_request_list_t req_list;
		                	mavlink_msg_param_request_list_decode(&msg, &req_list);
                            req_list.target_system = 3;
                            mavlink_msg_param_request_list_encode(source_system_id,source_component_id,&msg,&req_list);
                            break;

                            case MAVLINK_MSG_ID_REQUEST_EVENT:;
                            mavlink_request_event_t request_event;
	                        mavlink_msg_request_event_decode(&msg, &request_event);
                            request_event.target_system = 3;
                            mavlink_msg_request_event_encode(source_system_id,source_component_id,&msg,&request_event);
                            break;

                        

                            default:
                                break;
                            }

                        ssize_t len2;
                        int packetlen = mavlink_msg_to_send_buffer(buff__, &msg);

                        len2 = sendto(fds_thirdpilot[0].fd,buff__,packetlen,0,(struct sockaddr *)&remote_thirdpilot_addr, remote_thirdpilot_addr_len);   
                        
                        if (len2 > 0)
                        {
                            printf("Message forwarded to the third autopilot\n");
                        }                            
                    }
                }
            }
        }
    }
}


int main(int argc, char const *argv[])
{
    //Structs that will describe the old action and the new action
    //associated to the SIGINT signal (Ctrl+c from keyboard).

    struct sigaction new_action;
    
    //Set the handler in the new_action struct
    new_action.sa_handler = sigint_handler;
    //Remove any flag from sa_flag. 
    new_action.sa_flags = 0;
    

    //Set to empty the sa_mask. It means that no signal is blocked
    // while the handler run.
    sigemptyset(&(new_action.sa_mask));
    //Block the SIGTERM signal.
    

    // It means that while the handler run, the SIGTERM signal is ignored
    sigaddset(&(new_action.sa_mask), SIGINT);
   
    
     
    //Replace the signal handler of SIGINT with the one described by new_action
    if (sigaction(SIGINT, &new_action, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    } 


    myflag = 0;
    pthread_t thread1, thread2, thread3, thread4, thread5;
    struct sockaddr_in local_firstpilot_addr, local_secondpilot_addr, local_thirdpilot_addr, localqgc_server_addr;
    __socklen_t local_firstpilot_addr_len, local_secondpilot_addr_len, local_thirdpilot_addr_len, localqgc_server_addr_len; 
    
    typedef uint32_t in_addr_t;
    bool close_connection = false;

    firstpilotselection = true;
    secondpilotselection = false;
    thirdpilotselection = false;
    pilots_number = 3;

    
   int result, socket_reuse;


    //Addressing the local port to bind interface to local port to connect with QGC
    int qgc_local_port = 12500;
    memset((char *)&localqgc_server_addr, 0, sizeof(localqgc_server_addr));
    localqgc_server_addr.sin_family = AF_INET;
    localqgc_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    localqgc_server_addr.sin_port = htons(qgc_local_port);
    localqgc_server_addr_len = sizeof(localqgc_server_addr);

    //Creating socket and binding to the local address

    qgc_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (qgc_socket < 0)
    {
        perror("Creating qgc socket failed\n");
        abort();

    }

    if((bind(qgc_socket, (struct sockaddr*)&localqgc_server_addr,sizeof(struct sockaddr))) == -1)
    {
        perror("Binding socket failed at server to connect to the QGC\n");
        close(qgc_socket);
        exit(EXIT_FAILURE);
    } 

    if ((fcntl(qgc_socket, F_SETFL, O_NONBLOCK)) < 0)
    {
		perror("error setting nonblocking\n");
		close(qgc_socket);
		exit(EXIT_FAILURE);
    }

    //Structure with the Binded socket from QGC 

    memset(fds_qgc,0,sizeof(fds_qgc)); 
    fds_qgc[0].fd = qgc_socket;
    fds_qgc[0].events = POLLIN | POLLOUT;





    //Addressing the QgroundControl to use in sendto() and rcvfrom

    int qgc_portnumber = 14550; 
    
    memset((char *)&qgc_addr, 0, sizeof(qgc_addr));
    qgc_addr.sin_family= AF_INET;
    qgc_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    qgc_addr.sin_port = htons(qgc_portnumber);
    qgc_addr_len = sizeof(qgc_addr);


    

    /*
    result = setsockopt(qgc_socket, SOL_SOCKET, SO_REUSEADDR, &socket_reuse, sizeof(socket_reuse));
    if (result != 0) 
    {
        perror("setsockopt reuse address failed, aborting\n");
        abort();
    }
    */

   //



    /* Addressing the first PX4 and get the socket to poll - the QGC interface program behaves like client */
    
    int firstpilot_portnumber = 15555;

    

    memset((char *)&local_firstpilot_addr,0,sizeof(local_firstpilot_addr));
    local_firstpilot_addr.sin_family = AF_INET;
    local_firstpilot_addr_len = sizeof(local_firstpilot_addr);
    local_firstpilot_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_firstpilot_addr.sin_port = htons(firstpilot_portnumber);

    
    
    
    
    //Creating socket and binding to the local address

    
    firstpilot_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(firstpilot_socket == 0)
    {
        perror("Creating firstpilot socket failed\n");
        abort();
    }

    result = fcntl(firstpilot_socket, F_SETFL, O_NONBLOCK);
    if (result < 0)
    {
        perror("error setting nonblocking on firstpilot\n");
		close(firstpilot_socket);
		exit(EXIT_FAILURE);
    }

    if((bind(firstpilot_socket, (struct sockaddr*)&local_firstpilot_addr,sizeof(struct sockaddr))) == -1)
    {
        perror("Binding socket failed at firstpilot\n");
        close(firstpilot_socket);
        exit(EXIT_FAILURE);
    } 
    
    memset(fds_firstpilot,0,sizeof(fds_firstpilot)); 
    fds_firstpilot[0].fd = firstpilot_socket;
    fds_firstpilot[0].events = POLLIN | POLLOUT;
    
    /*
    result = setsockopt(firstpilot_socket, SOL_SOCKET, SO_REUSEADDR, &socket_reuse, sizeof(socket_reuse));
    if (result != 0) 
    {
        perror("setsockopt reuse address failed, aborting\n");
        abort();
    }
    */


     /* Addressing the second PX4 and get the socket to poll - the QGC interface program behaves like client*/
    
    int secondpilot_portnumber = 15556;

    

    memset((char *)&local_secondpilot_addr,0,sizeof(local_secondpilot_addr));
    local_secondpilot_addr.sin_family = AF_INET;
    local_secondpilot_addr_len = sizeof(local_secondpilot_addr);
    local_secondpilot_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_secondpilot_addr.sin_port = htons(secondpilot_portnumber);

    
    
    
    
    //Creating socket and binding to the local address

    
    secondpilot_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(secondpilot_socket == 0)
    {
        perror("Creating secondpilot socket failed\n");
        abort();
    }

    result = fcntl(secondpilot_socket, F_SETFL, O_NONBLOCK);
    if (result < 0)
    {
        perror("error setting nonblocking\n");
		close(secondpilot_socket);
		exit(EXIT_FAILURE);
    }
    

    

    if((bind(secondpilot_socket, (struct sockaddr*)&local_secondpilot_addr,sizeof(struct sockaddr))) == -1)
    {
        perror("Binding socket failed at the second pilot\n");
        close(secondpilot_socket);
        exit(EXIT_FAILURE);
    } 
    


    memset(fds_secondpilot,0,sizeof(fds_secondpilot)); 
    fds_secondpilot[0].fd = secondpilot_socket;
    fds_secondpilot[0].events = POLLIN | POLLOUT;



     /* Addressing the third PX4 and get the socket to poll - the QGC interface program behaves like client*/
    
    int thirdpilot_portnumber = 15557;

    

    memset((char *)&local_thirdpilot_addr,0,sizeof(local_thirdpilot_addr));
    local_thirdpilot_addr.sin_family = AF_INET;
    local_thirdpilot_addr_len = sizeof(local_thirdpilot_addr);
    local_thirdpilot_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_thirdpilot_addr.sin_port = htons(thirdpilot_portnumber);

    
    
    
    
    //Creating socket and binding to the local address

    
    thirdpilot_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(thirdpilot_socket == 0)
    {
        perror("Creating thirdpilot socket failed\n");
        abort();
    }

    result = fcntl(thirdpilot_socket, F_SETFL, O_NONBLOCK);
    if (result < 0)
    {
        perror("error setting nonblocking\n");
		close(thirdpilot_socket);
		exit(EXIT_FAILURE);
    }
    

    

    if((bind(thirdpilot_socket, (struct sockaddr*)&local_thirdpilot_addr,sizeof(struct sockaddr))) == -1)
    {
        perror("Binding socket failed at the third pilot\n");
        close(thirdpilot_socket);
        exit(EXIT_FAILURE);
    } 
    


    memset(fds_thirdpilot,0,sizeof(fds_thirdpilot)); 
    fds_thirdpilot[0].fd = thirdpilot_socket;
    fds_thirdpilot[0].events = POLLIN | POLLOUT;







    pthread_create(&thread1, NULL, qgc_poll_server , NULL);
    pthread_create(&thread2, NULL, firstpilot_poll_client,NULL);
    pthread_create(&thread3, NULL, secondpilot_poll_client, NULL);
    pthread_create(&thread4, NULL, thirdpilot_poll_client, NULL);
    //Creating selection thread
    pthread_create(&thread5, NULL, selection_communication, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    pthread_join(thread4, NULL);
    pthread_join(thread5, NULL);

    close(qgc_socket);
    close(firstpilot_socket);
    close(secondpilot_socket);
    close(thirdpilot_socket);
    close(firstpilot_selection_socket);
    close(secondpilot_selection_socket);
    close(thirdpilot_selection_socket);
   
}