
#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ip.h> 
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




pthread_cond_t cond0 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;

pthread_mutex_t lock0 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;


bool data_ready = true;



// Flag to end infite cycles from sockets and close them    
volatile __sig_atomic_t myflag;    
    
int simulator_socket, firstpilot_socket, secondpilot_socket, thirdpilot_socket, pilots_number;

pthread_mutex_t simulator_mutex, firstpilot_mutex, secondpilot_mutex, thirdpilot_mutex , selection_mutex;

struct pollfd fds_c[1], fds_s[2], fds_sp[2], fds_tp[2];

bool firstpilotselection, secondpilotselection, thirdpilotselection;

int firstpilot_selection_socket, secondpilot_selection_socket, thirdpilot_selection_socket;

unsigned fd_count_c = 1;


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

    int firstpilot_portnumber = 6510;
    int secondpilot_portnumber = 6511;
    int thirdpilot_portnumber = 6512;

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
        

        int ret = poll(&fds_selection[0],fd_count_selection,5000);
        if (ret < 0)
        {
            perror("poll error on selection socket\n");
            continue;
        }

        if (ret == 0)
        {
            perror("poll timeout on selection socket\n");
            //continue;
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

                                                     //Confirm if the corrrespont ID refers to the autopilot

                                                    if (autopilot_id_origin == 2)
                                                    { 
                                                        voter[1] = redundant_pilot_selection.redundant_px4_selected;
                                                        second_voted = true;
                                                        printf("Received vote from autopilot 2\n");
                                                    }
                                                    break;
                                                case 5:

                                                     //Confirm if the corrrespont ID refers to the autopilot
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
                                                        

                                                        //Testing pilots switch best pilot
                                                        switch (best_pilot)
                                                        {
                                                        case 1:

                                                            firstpilotselection = true;
                                                            secondpilotselection = false;
                                                            thirdpilotselection = false;
                                                            printf("Pilot 1 takes over\n");
                                                            break;
                                                        
                                                        
                                                        case 2:
                                                            firstpilotselection = false;
                                                            secondpilotselection = true;
                                                            thirdpilotselection = false;
                                                            printf("Pilot 2 takes over\n");
                                                            break;
                                                        
                                                        
                                                        case 3:
                                                            firstpilotselection = false;
                                                            secondpilotselection = false;
                                                            thirdpilotselection = true;
                                                            printf("Pilot 3 takes over\n");
                                                            break;

                                                        default:
                                                            break;
                                                        }

                                                        sleep(3);

                                                    }
                                                }
                                                

                                                
                                            
                                            }
                                        
                                        }
                                        
                                    }
                                  
        }

        /*
        
        pthread_mutex_lock(&lock0);

        while (data_ready)
        {
            pthread_cond_wait(&cond0,&lock0);
        }

        data_ready=true;
        pthread_cond_signal(&cond0);
        pthread_mutex_unlock(&lock0);    
        */
    }

}






// Signal handler to activate the flag
void sigint_handler(int signum)    
{
    write(0, "Hello from handler - volatile flag set to 1 \n", 46);
    myflag = 1; 
    
}

// Simulator poll thread

void *simulator_poll_client( void *ptr)
{
   
    struct sockaddr_in remote_simulator_addr;
     __socklen_t remote_simulator_addr_len;
    typedef uint32_t in_addr_t; 
    
    
    // The poll structures are activated after defined socket connections
    
    

    mavlink_status_t mavlink_status;
    mavlink_message_t msg;
    int chan = MAVLINK_COMM_0; 

    memset(&remote_simulator_addr,0,sizeof(remote_simulator_addr));
    remote_simulator_addr.sin_family = AF_INET;
    remote_simulator_addr_len = sizeof(remote_simulator_addr); 
    
    char buffer[MAVLINK_MAX_PACKET_LEN];
    int timeout = 1;
    int z_selection=1;

    //The cycle to poll and send messages

    while (!myflag)
    {   
        
        /*
        pthread_mutex_lock(&lock0);
        while (!data_ready)
        {
            pthread_cond_wait(&cond0,&lock0);
        }

        */
        
        int poll_sim = poll(&fds_c[0], fd_count_c,timeout);
       
        if (poll_sim == 0)
        {
            printf("simulator poll timeout %d \n", poll_sim);
        }
         
        if (poll_sim < 0)
        {
            printf("simulator poll error %d\n",poll_sim);
        }
        
        if (fds_c[0].revents & POLLIN)
        {
            
            int len = recvfrom(simulator_socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &remote_simulator_addr, (__socklen_t *)&remote_simulator_addr_len);
        
            if (len>0)
            {
                for (int i = 0; i < len; i++)
                {
                    
                    if (mavlink_parse_char(chan, buffer[i], &msg, &mavlink_status)) 
                    {

                        //printf("Received message with ID: %d, sequence: %d, from component: %d, from system: %d \n", msg.msgid, msg.seq, msg.compid, msg.sysid);
                        printf("Received message from simulator, forward - first thread 1.1 \n");
                        // forward message to PX4

                        //z_selection = z_selection + 1;


                        

                        //Cycle to forward the message to all PX4 - need to usa a switch to choose instructions to each one of them
                        for(int j=0; j < pilots_number;j++)
                        {
                                switch (j)
                                {
                                case 0:
                                
                                    //Forward to the first pilot socket    
                                    if (fds_s[1].fd > 0)
                                    {
                                        int ret = poll(&fds_s[1],1,timeout);
                        
                        
                                        if (ret < 0)
                                        {
                                            perror("Forward message - Poll error on the first pilot\n"); 
                                            //study the "continue" option because it breaks the forwarding cycle too from the second pilot socket
                                            continue;
                                        }

                                        if (ret == 0 && timeout > 0)
                                        {
                                            printf("Forward message on the first pilot- Poll timeout\n");
                                            continue;
                                        }

                                        if(!fds_s[1].revents & POLLOUT) 
                                        {
                                            printf("Invalid events at the file descriptor POLLOUT on the first PX4 server socket and at first thread loop\n");
                                            continue;
                                        }


                                        int packetlen = mavlink_msg_to_send_buffer(buffer, &msg);

                                        ssize_t len2;
                                        len2 = send(fds_s[1].fd, buffer, packetlen,0);
                                        printf("Message forwarded to the first PX4 from simulator - first thread 1.2 \n");
                                        break;
                                    }
                            
                                case 1:
                                

                                    //Forward to the second pilot socket
                                    if (fds_sp[1].fd > 0)
                                    {
                                        int ret = poll(&fds_sp[1],1,timeout);
                        
                        
                                        if (ret < 0)
                                        {
                                            perror("Forward message - Poll error on the second pilot\n"); 
                                            //study the "continue" option because it breaks the forwarding cycle too from the second pilot socket
                                            continue;
                                        }

                                        if (ret == 0 && timeout > 0)
                                        {
                                            printf("Forward message on the second pilot - Poll timeout\n");
                                            continue;
                                        }

                                        if(!fds_sp[1].revents & POLLOUT) 
                                        {
                                            printf("Invalid events at the file descriptor POLLOUT on the second PX4 server socket and at first thread loop\n");
                                            continue;
                                        }


                                        int packetlen = mavlink_msg_to_send_buffer(buffer, &msg);


                                        ssize_t len3;
                                        //Verify if the second message is accepted because of the mavlink system id
                                        len3 = send(fds_sp[1].fd, buffer, packetlen,0);
                                        printf("Message forwarded to the second PX4 from simulator - first thread 1.3 \n");
                                        break;

                                    }


                                case 2:
                                

                                    //Forward to the third pilot socket
                                    if (fds_tp[1].fd > 0)
                                    {
                                        int ret = poll(&fds_tp[1],1,timeout);
                        
                        
                                        if (ret < 0)
                                        {
                                            perror("Forward message - Poll error on the second pilot\n"); 
                                            //study the "continue" option because it breaks the forwarding cycle too from the second pilot socket
                                            continue;
                                        }

                                        if (ret == 0 && timeout > 0)
                                        {
                                            printf("Forward message on the second pilot - Poll timeout\n");
                                            continue;
                                        }

                                        if(!fds_tp[1].revents & POLLOUT) 
                                        {
                                            printf("Invalid events at the file descriptor POLLOUT on the second PX4 server socket and at first thread loop\n");
                                            continue;
                                        }

                                        int packetlen = mavlink_msg_to_send_buffer(buffer, &msg);

                                        ssize_t len4;
                                        //Verify if the second message is accepted because of the mavlink system id
                                        len4 = send(fds_tp[1].fd, buffer, packetlen,0);
                                        printf("Message forwarded to the third PX4 from simulator - first thread 1.4 \n");
                                        break;

                                    }    

                                default:
                                    break;
                                }
                            
                        
                                
                            
                            

                        }
                        
                    }
                
                }
                
            }
                
        }


    
    /*
    if (z_selection = 50)
    {
        data_ready=false;
        z_selection = 0;
       
    }
    
    pthread_cond_signal(&cond0);
	pthread_mutex_unlock(&lock0);
	
    */


    } 
    
    
    
    
    pthread_exit(NULL);
}


void *firstpilot_poll_server (void *ptr)
{
    /*Pool for client pilots connections and mavlinkmessages */

    struct sockaddr_in remotefirstpilot_addr;
    __socklen_t  remotefirstpilot_addr_len;
    bool close_connection = false;
    char buff__[MAVLINK_MAX_PACKET_LEN];
    int timeout = 1;
    unsigned fd_count_fp = 2;
    
    
    memset((char *)&remotefirstpilot_addr, 0, sizeof(remotefirstpilot_addr));
    remotefirstpilot_addr.sin_family = AF_INET;
    
    while(!myflag)
    {   
        //pthread_mutex_lock(&firstpilot_mutex);
        int time_out= 1;
        int ret = poll(&fds_s[0],fd_count_fp,time_out);
        if (ret < 0)
        {
            perror("poll error\n");
            continue;
        }

        if (ret == 0 && time_out > 0)
        {
            perror("first pilot poll timeout\n");
            
            continue;

        }
        
        for (size_t m = 0; m < fd_count_fp; m++)
        {
            if (fds_s[m].revents == 0)
            {
                continue;
            }
            
            if (!(fds_s[m].revents & POLLIN))
            {
                continue;
            }
            
            /*If event is raised on the listening socket - accept connections*/
            if (m == 0)
            {
                if (fds_s[1].fd > 0) /*Already connected */
                {
                    continue;
                }
                
                int rett = accept(fds_s[0].fd,(struct sockaddr *)&remotefirstpilot_addr,&remotefirstpilot_addr_len);  

                if (rett < 0)
                {
                    if (errno != EWOULDBLOCK)
                    {
                        printf("accept error %s \n", strerror(errno));
                    }

                    continue;
                }

                fds_s[1].fd = rett;
                fds_s[1].events = POLLIN | POLLOUT;

            } else { /* If event is raised on connection socket */
                        

                        int ret = recvfrom(fds_s[m].fd, buff__,sizeof(buff__),0,(struct sockaddr *)&remotefirstpilot_addr,&remotefirstpilot_addr_len);
                        if (ret < 0)
                        {
                            if (errno != EWOULDBLOCK) /* disconnected from client */
                            {
                                printf("recvfrom error : %s \n", strerror(errno));
                            }
                            continue;
                            
                        }

                        if (ret == 0) /* Client PX4 closed the connection orderly */
                        {
                            printf("Connection closed by client\n");
                            close_connection = true;
                            continue;
                        }

                        int len = ret;
                        mavlink_message_t msg;
                        mavlink_status_t status;

                        if(firstpilotselection == true)
                        {

                        
                            for (unsigned k = 0; k < len; ++k)
                            {
                                if (mavlink_parse_char(MAVLINK_COMM_1,buff__[k],&msg, &status))
                                {
                                    //printf("Received message with ID: %d, sequence: %d, from component: %d, from system: %d \n", msg.msgid, msg.seq, msg.compid, msg.sysid);
                                    printf("Received message from the first PX4, forward to simulator - thread 2.1\n");
                                    if (fds_c[0].fd > 0)
                                    {
                                        int ret = poll(&fds_c[0],fd_count_c,timeout);
                        
                        
                                        if (ret < 0)
                                        {
                                            perror("Poll error\n");
                                            continue;
                                        }

                                        if (ret == 0 && timeout > 0)
                                        {
                                            printf("Poll first pilot POLLOUT timeout\n");
                                            continue;
                                        }

                                        if(!fds_c[0].revents & POLLOUT) 
                                        {
                                            printf("Invalid events at the file descriptor POLLOUT on the client socket and second thread loop\n");
                                            continue;
                                        }


                                        int source_component_id = msg.compid;
                                        int source_system_id = msg.sysid;
                                        switch (msg.msgid) 
                                        {
                                            
                                            case MAVLINK_MSG_ID_COMMAND_LONG:;
                                                mavlink_command_long_t cmd_mavlink;
                                                mavlink_msg_command_long_decode(&msg, &cmd_mavlink);
                                                mavlink_msg_command_long_encode(5,source_component_id,&msg, &cmd_mavlink);
                                                break;
                                                
                                           

                                            case MAVLINK_MSG_ID_HIL_ACTUATOR_CONTROLS:;
                                                mavlink_hil_actuator_controls_t controls;
                                                mavlink_msg_hil_actuator_controls_decode(&msg,&controls);
                                                mavlink_msg_hil_actuator_controls_encode(5,source_component_id,&msg,&controls);
                                                break;


                                            case MAVLINK_MSG_ID_HEARTBEAT:;
                                                mavlink_heartbeat_t hb;
                                                mavlink_msg_heartbeat_decode(&msg,&hb);
                                                mavlink_msg_heartbeat_encode(5,source_component_id,&msg,&hb);
                                                break;

                                            default:
                                            break;
                                        }
                        
                                        ssize_t len2;
                                        int packetlen = mavlink_msg_to_send_buffer(buff__, &msg);
                                        len2 = send(fds_c[0].fd, buff__, packetlen,0);
                                        
                                        
                                        if (len2 > 0)
                                        {
                                               printf("Message from the first autopilot forwarded to the simulator - thread 2.2\n");
                                        }
                                        
                                       
                                    }


                                }
                            
                            }
                        }
                        
                    

                        
                       
                        
                    }
            
        }
        
        //pthread_mutex_unlock(&firstpilot_mutex);
    }

    
    
    pthread_exit(NULL);
}



void *secondpilot_poll_server (void *ptr)
{
    /*Pool for client pilots connections and mavlinkmessages */

    struct sockaddr_in remotesecondpilot_addr;
    __socklen_t  remotesecondpilot_addr_len;
    bool close_connection = false;
    char buff__[MAVLINK_MAX_PACKET_LEN];
    int timeout = 1;
    unsigned fd_count_sp = 2;
    
    memset((char *)&remotesecondpilot_addr, 0, sizeof(remotesecondpilot_addr));
    remotesecondpilot_addr.sin_family = AF_INET;

    while(!myflag)
    {   
        //pthread_mutex_lock(&secondpilot_mutex);
        int time_out= 1;
        int ret = poll(&fds_sp[0],fd_count_sp,time_out);
        if (ret < 0)
        {
            perror("poll error\n");
            continue;
        }

        if (ret == 0 && time_out > 0)
        {
            perror("poll second pilot timeout\n");
            continue;
        }
        
        for (size_t m = 0; m < fd_count_sp; m++)
        {
            if (fds_sp[m].revents == 0)
            {
                continue;
            }
            
            if (!(fds_sp[m].revents & POLLIN))
            {
                continue;
            }
            
            /*If event is raised on the listening socket - accept connections*/
            if (m == 0)
            {
                if (fds_sp[1].fd > 0) /*Already connected */
                {
                    continue;
                }
                
                int rett = accept(fds_sp[0].fd,(struct sockaddr *)&remotesecondpilot_addr,&remotesecondpilot_addr_len);  

                if (rett < 0)
                {
                    if (errno != EWOULDBLOCK)
                    {
                        printf("accept error %s \n", strerror(errno));
                    }

                    continue;
                }

                fds_sp[1].fd = rett;
                fds_sp[1].events = POLLIN | POLLOUT;

            } else { /* If event is raised on connection socket */
                        

                        int ret = recvfrom(fds_sp[m].fd, buff__,sizeof(buff__),0,(struct sockaddr *)&remotesecondpilot_addr,&remotesecondpilot_addr_len);
                        if (ret < 0)
                        {
                            if (errno != EWOULDBLOCK) /* disconnected from client */
                            {
                                printf("recvfrom error : %s \n", strerror(errno));
                            }
                            continue;
                            
                        }

                        if (ret == 0) /* Client PX4 closed the connection orderly */
                        {
                            printf("Connection closed by client\n");
                            close_connection = true;
                            continue;
                        }

                        int len = ret;
                        mavlink_message_t msg;
                        mavlink_status_t status;

                        if(secondpilotselection == true)

                        {
                        
                            for (unsigned k = 0; k < len; ++k)
                            {
                                if (mavlink_parse_char(MAVLINK_COMM_2,buff__[k],&msg, &status))
                                {
                                    //printf("Received message with ID: %d, sequence: %d, from component: %d, from system: %d \n", msg.msgid, msg.seq, msg.compid, msg.sysid);
                                    printf("Received message from the second PX4, forward to the simulator - thread 3.1 \n");
                                    if (fds_c[0].fd > 0)
                                    {
                                        int ret = poll(&fds_c[0],fd_count_c,timeout);
                        
                        
                                        if (ret < 0)
                                        {
                                        perror("Poll error\n");
                                        continue;
                                        }

                                        if (ret == 0 && timeout > 0)
                                        {
                                            printf("Poll on the second pilot POLLOUT timeout\n");
                                            continue;
                                        }

                                        if(!fds_c[0].revents & POLLOUT) 
                                        {
                                            printf("Invalid events at the file descriptor POLLOUT on the client socket and second thread loop\n");
                                            continue;
                                        }


                                        int source_component_id = msg.compid;
                                        int source_system_id = msg.sysid;
                                        switch (msg.msgid) 
                                        {
                                            
                                            case MAVLINK_MSG_ID_COMMAND_LONG:;
                                                mavlink_command_long_t cmd_mavlink;
                                                mavlink_msg_command_long_decode(&msg, &cmd_mavlink);
                                                mavlink_msg_command_long_encode(5,source_component_id,&msg, &cmd_mavlink);
                                                break;
                                                
                                           

                                            case MAVLINK_MSG_ID_HIL_ACTUATOR_CONTROLS:;
                                                mavlink_hil_actuator_controls_t controls;
                                                mavlink_msg_hil_actuator_controls_decode(&msg,&controls);
                                                mavlink_msg_hil_actuator_controls_encode(5,source_component_id,&msg,&controls);
                                                break;


                                            case MAVLINK_MSG_ID_HEARTBEAT:;
                                                mavlink_heartbeat_t hb;
                                                mavlink_msg_heartbeat_decode(&msg,&hb);
                                                mavlink_msg_heartbeat_encode(5,source_component_id,&msg,&hb);
                                                break;

                                            default:
                                            break;
                                        }

                                        ssize_t len2;
                                        int packetlen = mavlink_msg_to_send_buffer(buff__, &msg);
                                        len2 = send(fds_c[0].fd, buff__, packetlen,0);
                                        
                                        if (len2 > 0)
                                        {
                                            printf("Message from the second autopilot forwarded to the simulator - thread 3.2\n");
                                        }
                                        
                                      
                                    }
                                }

                            }
                            
                        }
                        
            }
            
        }
        


        //pthread_mutex_unlock(&secondpilot_mutex); 
    }

    
    
    pthread_exit(NULL);
}




void *thirdpilot_poll_server (void *ptr)
{
    /*Pool for client pilots connections and mavlinkmessages */

    struct sockaddr_in remotethirdpilot_addr;
    __socklen_t  remotethirdpilot_addr_len;
    bool close_connection = false;
    char buff__[MAVLINK_MAX_PACKET_LEN];
    int timeout = 1;
    unsigned fd_count_tp = 2;
    
    memset((char *)&remotethirdpilot_addr, 0, sizeof(remotethirdpilot_addr));
    remotethirdpilot_addr.sin_family = AF_INET;

    while(!myflag)
    {   

        //pthread_mutex_lock(&thirdpilot_mutex);
        int time_out= 1;
        int ret = poll(&fds_tp[0],fd_count_tp,time_out);
        if (ret < 0)
        {
            perror("poll error\n");
            continue;
        }

        if (ret == 0 && time_out > 0)
        {
            perror("poll on third pilot timeout\n");
            continue;
        }
        
        for (size_t m = 0; m < fd_count_tp; m++)
        {
            if (fds_tp[m].revents == 0)
            {
                continue;
            }
            
            if (!(fds_tp[m].revents & POLLIN))
            {
                continue;
            }
            
            /*If event is raised on the listening socket - accept connections*/
            if (m == 0)
            {
                if (fds_tp[1].fd > 0) /*Already connected */
                {
                    continue;
                }
                
                int rett = accept(fds_tp[0].fd,(struct sockaddr *)&remotethirdpilot_addr,&remotethirdpilot_addr_len);  

                if (rett < 0)
                {
                    if (errno != EWOULDBLOCK)
                    {
                        printf("accept error %s \n", strerror(errno));
                    }

                    continue;
                }

                fds_tp[1].fd = rett;
                fds_tp[1].events = POLLIN | POLLOUT;

            } else { /* If event is raised on connection socket */

                        

                        int ret = recvfrom(fds_tp[m].fd, buff__,sizeof(buff__),0,(struct sockaddr *)&remotethirdpilot_addr,&remotethirdpilot_addr_len);
                        if (ret < 0)
                        {
                            if (errno != EWOULDBLOCK) /* disconnected from client */
                            {
                                printf("recvfrom error : %s \n", strerror(errno));
                            }
                            continue;
                            
                        }

                        if (ret == 0) /* Client PX4 closed the connection orderly */
                        {
                            printf("Connection closed by client\n");
                            close_connection = true;
                            continue;
                        }

                        int len = ret;
                        mavlink_message_t msg;
                        mavlink_status_t status;

                        if(thirdpilotselection == true)

                        {
                        
                            for (unsigned k = 0; k < len; ++k)
                            {
                                if (mavlink_parse_char(MAVLINK_COMM_3,buff__[k],&msg, &status))
                                {
                                    //printf("Received message with ID: %d, sequence: %d, from component: %d, from system: %d \n", msg.msgid, msg.seq, msg.compid, msg.sysid);
                                    printf("Received message from the third PX4, forward to the simulator - thread 4.1 \n");
                                    if (fds_c[0].fd > 0)
                                    {
                                        int ret = poll(&fds_c[0],fd_count_c,timeout);
                        
                        
                                        if (ret < 0)
                                        {
                                        perror("Poll error\n");
                                        continue;
                                        }

                                        if (ret == 0 && timeout > 0)
                                        {
                                            printf("Poll timeout on the third pilot POLLOUT\n");
                                            continue;
                                        }

                                        if(!fds_c[0].revents & POLLOUT) 
                                        {
                                            printf("Invalid events at the file descriptor POLLOUT on the client socket and second thread loop\n");
                                            continue;
                                        }



                                        int source_component_id = msg.compid;
                                        int source_system_id = msg.sysid;
                                        switch (msg.msgid) 
                                        {
                                            
                                            case MAVLINK_MSG_ID_COMMAND_LONG:;
                                                mavlink_command_long_t cmd_mavlink;
                                                mavlink_msg_command_long_decode(&msg, &cmd_mavlink);
                                                mavlink_msg_command_long_encode(5,source_component_id,&msg, &cmd_mavlink);
                                                break;
                                                
                                           

                                            case MAVLINK_MSG_ID_HIL_ACTUATOR_CONTROLS:;
                                                mavlink_hil_actuator_controls_t controls;
                                                mavlink_msg_hil_actuator_controls_decode(&msg,&controls);
                                                mavlink_msg_hil_actuator_controls_encode(5,source_component_id,&msg,&controls);
                                                break;


                                            case MAVLINK_MSG_ID_HEARTBEAT:;
                                                mavlink_heartbeat_t hb;
                                                mavlink_msg_heartbeat_decode(&msg,&hb);
                                                mavlink_msg_heartbeat_encode(5,source_component_id,&msg,&hb);
                                                break;

                                            default:
                                            break;
                                        }

                                        ssize_t len2;
                                        int packetlen = mavlink_msg_to_send_buffer(buff__, &msg);
                                        len2 = send(fds_c[0].fd, buff__, packetlen,0);

                                                                              
                                        if (len2 > 0)
                                        {
                                              printf("Message from the third autopilot forwarded to the simulator - thread 4.2\n");
                                        }
                                        
                              
                                    }
                                }

                            }
                            
                        }                
                        
            }
            
        }
        
        
        //pthread_mutex_unlock(&thirdpilot_mutex);
    }

    
    
    pthread_exit(NULL);
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
    struct sockaddr_in localfirstpilot_addr, simulator_server_addr, secondpilot_addr, thirdpilot_addr;
    __socklen_t simulator_server_addr_len, localfirstpilot_addr_len, secondpilot_addr_len, thirdpilot_addr_len; 
    
    typedef uint32_t in_addr_t;
    bool close_connection = false;

    firstpilotselection = true;
    secondpilotselection = false;
    thirdpilotselection = false;
    
    pilots_number = 3;


    /* Addressing simulator port - the selection program behaves like client*/

    int simulator_portnumber = 4560; 
    
    memset((char *)&simulator_server_addr, 0, sizeof(simulator_server_addr));
    simulator_server_addr.sin_family= AF_INET;
    simulator_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //inet_addr("192.168.1.2");
    simulator_server_addr.sin_port = htons(simulator_portnumber);
    simulator_server_addr_len = sizeof(simulator_server_addr);
   
    
    printf("Connecting to the server simulator! \n");
   

    while (!myflag)
    {
          
    
        if ((simulator_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("ERROR creating TCP socket\n");
            return 0;
        }

        int yes = 1;
			int ret = setsockopt(simulator_socket, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(int));
            if (ret != 0)
            {
                perror("setsockopt_1 failed\n");
            }
            
            ret = connect(simulator_socket, (struct sockaddr *)&simulator_server_addr,simulator_server_addr_len);
            
            if (ret==0) 
            {
                printf("Simulator connected to the Selection Program on TCP port %d \n", simulator_portnumber);
                break;

            }   else    {
                            close(simulator_socket);  
                            sleep(3);  
                        }
            
    
    }

    printf("Starting the first thread to poll messages from the simulator\n");

    fds_c[0].fd = simulator_socket;
    fds_c[0].events = POLLIN | POLLOUT;
    

  


    printf("Waiting for PX4 connection on the listening sockets \n");

    

    /* Addressing the first PX4 and get the socket to poll - the selection program behaves like server*/

    int firstpilot_portnumber = 4561;

    

    memset((char *)&localfirstpilot_addr,0,sizeof(localfirstpilot_addr));
    localfirstpilot_addr.sin_family = AF_INET;
    localfirstpilot_addr_len = sizeof(localfirstpilot_addr);
    localfirstpilot_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    localfirstpilot_addr.sin_port = htons(firstpilot_portnumber);
    
  
  

    if ((firstpilot_socket = socket(AF_INET, SOCK_STREAM,0)) < 0)
    {
        perror("Creating TCP socket failed, aborting\n");
        abort();    
    }
    
    int yes;
    int result = setsockopt(firstpilot_socket, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
    if (result != 0)
    {
        perror("setsockpt failed, aborting\n");
        abort();
    }

    struct linger nolinger;
    nolinger.l_onoff = 1;
    nolinger.l_linger = 0;

    result = setsockopt(firstpilot_socket, SOL_SOCKET, SO_LINGER, &nolinger, sizeof(nolinger));
    if (result != 0)
    {
        perror("setsockpt linger failed, aborting\n");
        abort();
    }

    int socket_reuse = 1;

    result = setsockopt(firstpilot_socket, SOL_SOCKET, SO_REUSEADDR, &socket_reuse, sizeof(socket_reuse));
    if (result != 0) 
    {
        perror("setsockopt reuse address failed, aborting\n");
        abort();
    }

    /*Not defined on this kernel - if needed, the patch should be updated
    
    result = setsockopt(firstpilot_socket, SOL_SOCKET, SO_REUSEPORT, &socket_reuse,sizeof(socket_reuse));
    if (result != 0)
    {
        perror("setsockopt reuse port failed, aborting");
        abort();
    }*/

    result = fcntl(firstpilot_socket, F_SETFL, O_NONBLOCK);

    if (result == -1)
    {
        perror("setting socket to non-blocking failed, aborting\n");
        abort();
    }
    
    if (bind(firstpilot_socket,(struct sockaddr *)&localfirstpilot_addr,localfirstpilot_addr_len) < 0)  
    {
        perror("bind failed, aborting\n");
        abort();
    }
    
    if (listen(firstpilot_socket,0) < 0)
    {
        perror("listen failed, aborting\n");
        abort();
    }
    
    
    //Making the poll with the first pilot socket

    memset(fds_s,0,sizeof(fds_s)); 
    fds_s[0].fd = firstpilot_socket;
    fds_s[0].events = POLLIN;

   
    
    

    /* Addressing the second PX4 and get the socket to poll*/

    int secondpilot_portnumber = 4562;


    memset((char *)&secondpilot_addr,0,sizeof(secondpilot_addr));
    secondpilot_addr.sin_family = AF_INET;
    secondpilot_addr_len = sizeof(secondpilot_addr);
    secondpilot_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    secondpilot_addr.sin_port = htons(secondpilot_portnumber);

  

    if ((secondpilot_socket = socket(AF_INET, SOCK_STREAM,0)) < 0)
    {
        perror("Creating TCP socket failed on second pilot, aborting\n");
        abort();    
    }
    
    int yes2;
    
    int result2 = setsockopt(secondpilot_socket, IPPROTO_TCP, TCP_NODELAY, &yes2, sizeof(yes2));
    if (result2 != 0)
    {
        perror("setsockpt failed on second pilot, aborting\n");
        abort();
    }

    result2 = setsockopt(secondpilot_socket, SOL_SOCKET, SO_LINGER, &nolinger, sizeof(nolinger));
    if (result2 != 0)
    {
        perror("setsockpt linger failed on second pilot, aborting\n");
        abort();
    }


    result2 = setsockopt(secondpilot_socket, SOL_SOCKET, SO_REUSEADDR, &socket_reuse, sizeof(socket_reuse));
    if (result2 != 0) {
        perror("setsockopt reuse address on second pilot failed, aborting\n");
        abort();
      }

    /*Not defined on this kernel - if needed, the patch should be updated
    
    result2 = setsockopt(firstpilot_socket, SOL_SOCKET, SO_REUSEPORT, &socket_reuse,sizeof(socket_reuse));
    if (result2 != 0)
    {
        perror("setsockopt reuse port failed, aborting");
        abort();
    }*/

    result2 = fcntl(secondpilot_socket, F_SETFL, O_NONBLOCK);

    if (result2 == -1)
    {
        perror("setting socket to non-blocking failed, aborting\n");
        abort();
    }
    
    if (bind(secondpilot_socket,(struct sockaddr *)&secondpilot_addr,secondpilot_addr_len) < 0)  
    {
        perror("bind failed, aborting\n");
        abort();
    }
    
    if (listen(secondpilot_socket,0) < 0)
    {
        perror("listen failed, aborting\n");
        abort();
    }


    //Making the poll with the second pilot socket

    memset(fds_sp,0,sizeof(fds_sp)); 
    fds_sp[0].fd = secondpilot_socket;
    fds_sp[0].events = POLLIN;


    

    
    
    
    /* Addressing the third PX4 and get the socket to poll*/

    int thirdpilot_portnumber = 4563;


    memset((char *)&thirdpilot_addr,0,sizeof(thirdpilot_addr));
    thirdpilot_addr.sin_family = AF_INET;
    thirdpilot_addr_len = sizeof(thirdpilot_addr);
    thirdpilot_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    thirdpilot_addr.sin_port = htons(thirdpilot_portnumber);

  

    if ((thirdpilot_socket = socket(AF_INET, SOCK_STREAM,0)) < 0)
    {
        perror("Creating TCP socket failed on second pilot, aborting\n");
        abort();    
    }
    
    int yes3;
    
    int result3 = setsockopt(thirdpilot_socket, IPPROTO_TCP, TCP_NODELAY, &yes3, sizeof(yes3));
    if (result3 != 0)
    {
        perror("setsockpt failed on second pilot, aborting\n");
        abort();
    }

    result3 = setsockopt(thirdpilot_socket, SOL_SOCKET, SO_LINGER, &nolinger, sizeof(nolinger));
    if (result3 != 0)
    {
        perror("setsockpt linger failed on second pilot, aborting\n");
        abort();
    }


    result3 = setsockopt(thirdpilot_socket, SOL_SOCKET, SO_REUSEADDR, &socket_reuse, sizeof(socket_reuse));
    if (result3 != 0) {
        perror("setsockopt reuse address on second pilot failed, aborting\n");
        abort();
      }


    result3 = fcntl(thirdpilot_socket, F_SETFL, O_NONBLOCK);

    if (result3 == -1)
    {
        perror("setting socket to non-blocking failed, aborting\n");
        abort();
    }
    
    if (bind(thirdpilot_socket,(struct sockaddr *)&thirdpilot_addr,thirdpilot_addr_len) < 0)  
    {
        perror("bind failed, aborting\n");
        abort();
    }
    
    if (listen(thirdpilot_socket,0) < 0)
    {
        perror("listen failed, aborting\n");
        abort();
    }


    //Making the poll with the third pilot socket

    memset(fds_tp,0,sizeof(fds_tp)); 
    fds_tp[0].fd = thirdpilot_socket;
    fds_tp[0].events = POLLIN;

    pthread_mutex_init(&simulator_mutex,NULL);
    pthread_mutex_init(&firstpilot_mutex,NULL);
    pthread_mutex_init(&secondpilot_mutex,NULL);
    pthread_mutex_init(&thirdpilot_mutex,NULL);

    
    
    // Initiate the thread which poll messages from simulator
    pthread_create(&thread1, NULL, simulator_poll_client, NULL);

    //Creating the first pilot thread    
    pthread_create(&thread2, NULL, firstpilot_poll_server, NULL);

    //Creating the second pilot thread
    pthread_create(&thread3, NULL, secondpilot_poll_server, NULL);

    
    //Creating the third pilot thread
    pthread_create(&thread4, NULL, thirdpilot_poll_server, NULL);

    //Creating selection thread
    pthread_create(&thread5, NULL, selection_communication, NULL);


    
    pthread_join(thread4,NULL);
    pthread_join(thread3,NULL);
    pthread_join(thread1,NULL);
    pthread_join(thread2,NULL);
    pthread_join(thread5,NULL);



    pthread_mutex_destroy(&simulator_mutex);
    pthread_mutex_destroy(&firstpilot_mutex);
    pthread_mutex_destroy(&secondpilot_mutex);
    pthread_mutex_destroy(&thirdpilot_mutex);
    pthread_mutex_destroy(&selection_mutex);



    printf("Closing sockets\n");
    
    close(firstpilot_socket);
    close(simulator_socket);
    close(secondpilot_socket);
    close(thirdpilot_socket);
    close(firstpilot_selection_socket);
    close(secondpilot_selection_socket);
    close(thirdpilot_selection_socket);
   
    
        
    
    return 0;
}

