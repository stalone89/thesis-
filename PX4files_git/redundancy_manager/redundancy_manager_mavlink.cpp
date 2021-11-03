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

#include "redundancy_manager.h"



#include <px4_platform_common/getopt.h>
#include <px4_platform_common/log.h>
#include <px4_platform_common/posix.h>
#include <px4_platform_common/tasks.h>



//redundancy_manager *redundancy_manager::instance = nullptr;



pthread_cond_t cond0 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;

pthread_mutex_t lock0 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;


static bool data_ready=0;

static int _fd_first, _fd_second;

//redundancy_manager *redundancy_manager::instance = nullptr;

const unsigned mode_flag_armed = 128;
const unsigned mode_flag_custom = 1;


//Define the struct poll for the two autopilots
static struct pollfd fds[4];

using namespace time_literals;

static struct sockaddr_in _myaddr_first {};
static __socklen_t _myaddr_first_len;

struct sockaddr_in _myaddr_second {};
static __socklen_t _myaddr_second_len;



void redundancy_manager::run()
{
	pthread_setname_np(pthread_self(), "redundancy_recv");


	switch (px4_instance)
	{
	case 0:
		_port_1 = 6500;
		_port_2 = 6501;
		_port_3 = 6510;
		_port_4 = 6515;

		break;

	case 1:

		_port_1 = 6500;
		_port_2 = 6502;
		_port_3 = 6511;
		_port_4 = 6516;
		break;

	case 2:

		_port_1 = 6501;
		_port_2 = 6502;
		_port_3 = 6512;
		_port_4 = 6517;
		break;

	default:
		break;
	}




	//Addressing the selection port connection to the simulator
	addr_selection.sin_family = AF_INET;
	addr_selection.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_selection.sin_port = htons(_port_3);
	addr_selection_len = sizeof(addr_selection);


	//Addressing the selection port connection to the QGC
	addr_qgc_selection.sin_family = AF_INET;
	addr_qgc_selection.sin_addr.s_addr= htonl(INADDR_ANY);
	addr_qgc_selection.sin_port = htons(_port_4);
	addr_qgc_selection_len = sizeof(addr_qgc_selection);



	//Addressing the first pilot connection
	_myaddr_first.sin_family = AF_INET;
	_myaddr_first.sin_addr.s_addr = htonl(INADDR_ANY);
	_myaddr_first.sin_port = htons(_port_1);
	_myaddr_first_len = sizeof(_myaddr_first);



	//Addressing the second pilot connection
	_myaddr_second.sin_family = AF_INET;
	_myaddr_second.sin_addr.s_addr = htonl(INADDR_ANY);
	_myaddr_second.sin_port = htons(_port_2);
	_myaddr_second_len = sizeof(_myaddr_second);






    	struct sockaddr_in remote_secondpilot_addr;
	__socklen_t remote_secondpilot_addr_len;
	remote_secondpilot_addr_len = sizeof(remote_secondpilot_addr);


	struct  sockaddr_in remote_firstpilot_addr;
	__socklen_t remote_firstpilot_addr_len;
	remote_firstpilot_addr_len = sizeof(remote_firstpilot_addr);


	int fd_count;
	unsigned char _buf[MAVLINK_MAX_PACKET_LEN];

   	struct linger nolinger;
    	nolinger.l_onoff = 1;
   	nolinger.l_linger = 0;

	// initialize parameters
	parameters_update(true);


	//Initializing the fds structure
	memset(fds,0,sizeof(fds));


	//Instance 0 is the server, instance 1 is half server/half client, instance 2 is the client

	//Instance 0 behaves like a server with the two sockets

	if (px4_instance == 0)
	{

		int result;
		//First socket definition
		if ((_fd_first = socket(AF_INET, SOCK_STREAM, 0)) < 0){

		return;
		}

		int yes = 1;
		int ret = setsockopt(_fd_first, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(int));

		if (ret != 0)
		{
			PX4_ERR("setsockopt failed: %s", strerror(errno));
		}


		int socket_reuse_2 = 1;

		ret = setsockopt(_fd_first, SOL_SOCKET, SO_REUSEADDR, &socket_reuse_2, sizeof(socket_reuse_2));
		if (ret != 0)
		{
			PX4_ERR("setsockopt reuse address failed, aborting\n");

		}



		result = setsockopt(_fd_first, SOL_SOCKET, SO_LINGER, &nolinger, sizeof(nolinger));
		if (result != 0)
		{
			PX4_ERR("setsockpt linger failed %s\n",strerror(errno));

		}


		result = fcntl(_fd_first, F_SETFL, O_NONBLOCK);

		if (result == -1)
		{
			PX4_ERR("setting socket to non-blocking failed, aborting\n");

		}



		// Second socket definition

		if ((_fd_second = socket(AF_INET, SOCK_STREAM,0)) < 0)
		{
			PX4_ERR("Creating TCP socket failed, aborting\n");

		}

		int yes_2;

		result = setsockopt(_fd_second, IPPROTO_TCP, TCP_NODELAY, &yes_2, sizeof(yes_2));
		if (result != 0)
		{
			PX4_ERR("setsockpt failed, aborting\n");

		}

		result = setsockopt(_fd_second, SOL_SOCKET, SO_LINGER, &nolinger, sizeof(nolinger));
		if (result != 0)
		{
			PX4_ERR("setsockpt linger failed, aborting\n");

		}

		int socket_reuse = 1;

		result = setsockopt(_fd_second, SOL_SOCKET, SO_REUSEADDR, &socket_reuse, sizeof(socket_reuse));
		if (result != 0)
		{
			PX4_ERR("setsockopt reuse address failed, aborting\n");

		}



		result = fcntl(_fd_second, F_SETFL, O_NONBLOCK);

		if (result == -1)
		{
			PX4_ERR("setting socket to non-blocking failed, aborting\n");
			abort();
		}



		if (bind(_fd_first,(struct sockaddr *)&_myaddr_first,_myaddr_first_len) < 0)
    		{
        		PX4_ERR("bind failed, aborting\n");
			abort();

		}

    		if (listen(_fd_first,0) < 0)
    		{
        		PX4_ERR("listen failed, aborting\n");
			abort();

    		}

		if (bind(_fd_second,(struct sockaddr *)&_myaddr_second,_myaddr_second_len) < 0)
    		{
        		PX4_ERR("bind failed, aborting\n");
			abort();
    		}

    		if (listen(_fd_second,0) < 0)
    		{
        		PX4_ERR("listen failed, aborting\n");
			abort();

    		}

		fd_count = 4;
		fds[0].fd = _fd_second;
    		fds[0].events = POLLIN;
		fds[2].fd = _fd_first;
		fds[2].events = POLLIN;


		PX4_INFO("This is the instance %d", px4_instance);
	}







	//Instance 1 behaves like a server to 5502 port and like client to 5500 port


	if (px4_instance == 1)
	{
		while (true)
		{
			//First socket definition
			if ((_fd_first = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				PX4_ERR("Creating TCP socket failed: %s", strerror(errno));
				return;
			}

			int yes = 1;
			int ret = setsockopt(_fd_first, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(int));

			if (ret != 0)
			{
				PX4_ERR("setsockopt failed: %s", strerror(errno));
				return;
			}


			PX4_INFO("This is the instance %d", px4_instance);
			//Conecting to the autopilot
			PX4_INFO("Waiting for the autopilot to accept connection on TCP port %u", _port_1);
			int connection = connect(_fd_first, (struct sockaddr *)&_myaddr_first, _myaddr_first_len);

			if (connection == 0)
			{

				break;

			} else {
			::close(_fd_first);
			system_usleep(500);
			}
		}


		// Second socket definition
		if ((_fd_second = socket(AF_INET, SOCK_STREAM,0)) < 0)
		{
			PX4_ERR("Creating TCP socket failed, aborting\n");
			abort();

		}

		int yes_2;

		int result = setsockopt(_fd_second, IPPROTO_TCP, TCP_NODELAY, &yes_2, sizeof(yes_2));
		if (result != 0)
		{
			PX4_ERR("setsockpt failed, aborting\n");
			abort();

		}

		result = setsockopt(_fd_second, SOL_SOCKET, SO_LINGER, &nolinger, sizeof(nolinger));
		if (result != 0)
		{
			PX4_ERR("setsockpt linger failed, aborting\n");
			abort();

		}

		int socket_reuse = 1;



		result = setsockopt(_fd_second, SOL_SOCKET, SO_REUSEADDR, &socket_reuse, sizeof(socket_reuse));
		if (result != 0)
		{
			PX4_ERR("setsockopt reuse address failed, aborting\n");
			abort();

		}



		result = fcntl(_fd_second, F_SETFL, O_NONBLOCK);

		if (result == -1)
		{
			perror("setting socket to non-blocking failed, aborting\n");
			abort();
		}


		PX4_INFO("Autopilot connected on TCP port %u. Out of the cycle", _port_1);

		if (bind(_fd_second,(struct sockaddr *)&_myaddr_second,_myaddr_second_len) < 0)
    		{
        		PX4_ERR("bind failed, aborting\n");
			abort();
    		}

    		if (listen(_fd_second,0) < 0)
    		{
        		PX4_ERR("listen failed, aborting\n");
			abort();
    		}

		fd_count = 3;
		fds[0].fd = _fd_second;
    		fds[0].events = POLLIN;
		fds[2].fd = _fd_first;
		fds[2].events = POLLIN | POLLOUT;


	}







	//Instance 2 behaves like client to both ports 5501 and 5502
	if (px4_instance == 2)
	{
		while (true)
		{
			//First socket definition
			if ((_fd_first = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
			PX4_ERR("Creating TCP socket failed: %s", strerror(errno));
			return;
			}

			int yes = 1;
			int ret = setsockopt(_fd_first, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(int));

			if (ret != 0)
			{
				PX4_ERR("setsockopt failed: %s", strerror(errno));
			}


			PX4_INFO("This is the instance %d", px4_instance);

			//Conecting to the autopilot
			PX4_INFO("Waiting for the autopilot to accept connection on TCP port %u", _port_1);
			ret = connect(_fd_first, (struct sockaddr *)&_myaddr_first, _myaddr_first_len);

			if (ret == 0)
			{
				PX4_INFO("Autopilot connected on TCP port %u.", _port_1);
				break;

			} else {
			::close(_fd_first);
			system_usleep(500);
			}
		}

		while (true)
		{
			// Second socket definition
			if ((_fd_second = socket(AF_INET, SOCK_STREAM,0)) < 0)
    			{
        		PX4_ERR("Creating TCP socket failed, aborting\n");
			abort();
  	  		}

    			int yes_2;

    			int result = setsockopt(_fd_second, IPPROTO_TCP, TCP_NODELAY, &yes_2, sizeof(yes_2));
    			if (result != 0)
    			{
        		PX4_ERR("setsockpt failed, aborting\n");
			abort();

    			}
			//Conecting to the autopilot
			PX4_INFO("Waiting for the autopilot to accept connection on TCP port %u", _port_2);
			int ret = connect(_fd_second, (struct sockaddr *)&_myaddr_second, _myaddr_second_len);

			if (ret == 0)
			{
				PX4_INFO("Autopilot connected on TCP port %u.", _port_2);
				break;

			} else {
			::close(_fd_second);
			system_usleep(500);
			}
		}


		fd_count = 2;
		fds[0].fd = _fd_second;
    		fds[0].events = POLLIN | POLLOUT;
		fds[1].fd = _fd_first;
		fds[1].events = POLLIN | POLLOUT;

		PX4_INFO("This is the instance %d", px4_instance);

	}










	//Conecting to the Simulator selection port


	while (true)
	{

		//First socket definition
		if ((selection_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
		PX4_ERR("Creating TCP simulator selection socket failed: %s", strerror(errno));
		return;
		}

		int yes = 1;
		int ret = setsockopt(selection_socket, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(int));

		if (ret != 0)
		{
			PX4_ERR("setsockopt on simulator selection socket failed: %s", strerror(errno));
		}


		//Conecting to the simulator selection port
		PX4_INFO("Waiting for the Simulator selection program to accept connection on TCP port %u", _port_3);
		ret = connect(selection_socket, (struct sockaddr *)&addr_selection, addr_selection_len);

		if (ret == 0)
		{
			PX4_INFO("Simulator selection program connected on TCP port %u.", _port_3);
			break;

		} else {
		::close(selection_socket);
		sleep(1);
		}
	}






	//Connecting to the QGC selection port
	while (true)
	{
		//First socket definition
		if ((qgc_selection_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
		PX4_ERR("Creating TCP QGC selection socket failed: %s", strerror(errno));
		return;
		}

		int yes = 1;
		int ret = setsockopt(qgc_selection_socket, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(int));

		if (ret != 0)
		{
			PX4_ERR("setsockopt on QGC selection socket failed: %s", strerror(errno));
		}


		//Conecting to the QGC selection port
		PX4_INFO("Waiting for the QGC selection program to accept connection on TCP port %u", _port_4);
		ret = connect(qgc_selection_socket, (struct sockaddr *)&addr_qgc_selection, addr_qgc_selection_len);

		if (ret == 0)
		{
			PX4_INFO("QGC selection program connected on TCP port %u.", _port_4);
			break;

		} else {
		::close(qgc_selection_socket);
		sleep(1);
		}
	}

	for (int i = 0; i < error_flags_number; i++)
	{
		mypilotfaults_1[i] = false;
		firstpilotfaults_1[i] = false;
		secondpilotfaults_1[i] = false;
	}



	// Create a thread to send data to the Autopilot.
	pthread_t sender_thread_first_pilot;
	pthread_attr_t sender_thread_firstpilot_attr;
	pthread_attr_init(&sender_thread_firstpilot_attr);
	pthread_attr_setstacksize(&sender_thread_firstpilot_attr, PX4_STACK_ADJUSTED(8000));

	struct sched_param param;
	(void)pthread_attr_getschedparam(&sender_thread_firstpilot_attr, &param);
	param.sched_priority = SCHED_PRIORITY_NAVIGATION;
	(void)pthread_attr_setschedparam(&sender_thread_firstpilot_attr, &param);

	pthread_create(&sender_thread_first_pilot, &sender_thread_firstpilot_attr, redundancy_manager::sending_trampoline, nullptr);
	pthread_attr_destroy(&sender_thread_firstpilot_attr);




	mavlink_status_t mavlink_status = {};



	//Cycle to poll the messages for instance 0


	if(px4_instance == 0)
	{


		while (true)
		{


			pthread_mutex_lock(&lock0);
			while (!data_ready)
			{
				pthread_cond_wait(&cond0,&lock0);
			}




			int pret = ::poll(&fds[0], fd_count, 2000);


			if (pret == 0) {
				// Timed out.
				PX4_ERR("Receiving poll timeout %d, %d", pret, errno);
				continue;
			}

			if (pret < 0) {
				PX4_ERR("poll error %d, %d", pret, errno);
				continue;
			}


			for (int m = 0; m < fd_count; m++)
			{
				if (fds[m].revents == 0)
				{
					continue;
				}

				if (!(fds[m].revents & POLLIN))
				{
					continue;
				}

				/*If event is raised on the listening socket - accept connections*/
				if (m == 0)
				{

					if (fds[1].fd > 0) /*Already connected */
					{
						PX4_INFO("First socket Connected");
						continue;
					}

					int rett = accept(fds[0].fd,(struct sockaddr *)&remote_secondpilot_addr,&remote_secondpilot_addr_len);


					if (rett < 0)
					{
						if (errno != EWOULDBLOCK)
						{
							PX4_ERR("accept error %s \n", strerror(errno));
						}
						continue;
					}

					fds[1].fd = rett;
					fds[1].events = POLLIN | POLLOUT;



				} else 	{


						if (m == 2)
						{

							if (fds[3].fd > 0) /*Already connected */
							{

								continue;
							}

							int rett = accept(fds[2].fd,(struct sockaddr *)&remote_firstpilot_addr,&remote_firstpilot_addr_len);


							PX4_INFO("Keeps trying to connect on the first socket");

							if (rett < 0)
							{
								if (errno != EWOULDBLOCK)
								{
								printf("accept error %s \n", strerror(errno));
								}
								continue;
							}

							fds[3].fd = rett;
							fds[3].events = POLLIN | POLLOUT;



						} else {


							if (fds[m].revents & POLLIN)
							{
								if(m==1)
								{
									int len = ::recvfrom(fds[1].fd, _buf, sizeof(_buf), 0, (struct sockaddr *)&remote_secondpilot_addr, (socklen_t *)&remote_secondpilot_addr_len);
									if (len > 0)
									{
										//PX4_INFO("Receiving messages");
										mavlink_message_t msg;

										for (int i = 0; i < len; i++)
										{
											if (mavlink_parse_char(MAVLINK_COMM_1, _buf[i], &msg, &mavlink_status))
											{

												handle_message(&msg, false);


											}
										}
									}
									if (len <= 0)
									{
										PX4_INFO("Error sending socket on second port");
									}

								} else {
									if (m==3)
									{


										int len = ::recvfrom(fds[3].fd, _buf, sizeof(_buf), 0, (struct sockaddr *)&remote_firstpilot_addr, (socklen_t *)&remote_firstpilot_addr_len);
										if (len > 0)
										{
											//PX4_INFO("Receiving messages");
											mavlink_message_t msg;
											for (int i = 0; i < len; i++)
											{
												if (mavlink_parse_char(MAVLINK_COMM_2, _buf[i], &msg, &mavlink_status))
												{

													handle_message(&msg, true);


												}

											}
										}
										if (len <= 0)
										{
											PX4_INFO("Error sending socket on first port");
										}
									}
								}

							}


						}
					}

			}


		check_pilots();
		data_ready=0;

		pthread_mutex_unlock(&lock0);
		pthread_cond_signal(&cond0);


		}
	}






	//Cycle to poll the messages for the instance 1



	if(px4_instance == 1)
	{
		while (true)
		{


			pthread_mutex_lock(&lock0);
			while (!data_ready)
			{
				pthread_cond_wait(&cond0,&lock0);
			}



			int pret = ::poll(&fds[0], fd_count, 2000);


			if (pret == 0) {
				// Timed out.
				PX4_ERR("Receiving poll timeout %d, %d", pret, errno);
				continue;
			}

			if (pret < 0) {
				PX4_ERR("poll error %d, %d", pret, errno);
				continue;
			}


			for (int m = 0; m < fd_count; m++)
			{
				if (fds[m].revents == 0)
				{
					continue;
				}

				if (!(fds[m].revents & POLLIN))
				{
					continue;
				}

				/*If event is raised on the listening socket - accept connections*/
				if (m == 0)
				{

					if (fds[1].fd > 0) /*Already connected */
					{
						continue;
					}

					int rett = accept(fds[0].fd,(struct sockaddr *)&remote_secondpilot_addr,&remote_secondpilot_addr_len);

					PX4_INFO("Accepted connection on port %u", _port_2);


					if (rett < 0)
					{
						if (errno != EWOULDBLOCK)
						{
							printf("accept error %s \n", strerror(errno));
						}
						continue;
					}

					fds[1].fd = rett;
					fds[1].events = POLLIN | POLLOUT;



				} else 	{

						if (fds[m].revents & POLLIN)
						{
							if(m==1)
							{
								int len = ::recvfrom(fds[1].fd, _buf, sizeof(_buf), 0, (struct sockaddr *)&remote_secondpilot_addr, (socklen_t *)&remote_secondpilot_addr_len);
								if (len > 0)
								{

									mavlink_message_t msg;

									for (int i = 0; i < len; i++)
									{
										if (mavlink_parse_char(MAVLINK_COMM_1, _buf[i], &msg, &mavlink_status))
										{
											handle_message(&msg, false);

										}
									}
								}
								if (len <= 0)
									{
										PX4_INFO("Error sending socket on second port");
									}

							} else {
								if (m==2)
								{
									int len = ::recvfrom(fds[2].fd, _buf, sizeof(_buf), 0, (struct sockaddr *)&remote_firstpilot_addr, (socklen_t *)&remote_firstpilot_addr_len);
									if (len > 0)
									{

										mavlink_message_t msg;
										for (int i = 0; i < len; i++)
										{
											if (mavlink_parse_char(MAVLINK_COMM_2, _buf[i], &msg, &mavlink_status))
											{
												handle_message(&msg, true);

											}
										}
									}

									if (len <= 0)
									{
										PX4_INFO("Error sending socket on first port");
									}

								}
							}

						}


					}


			}


		check_pilots();
		data_ready=0;

		pthread_mutex_unlock(&lock0);
		pthread_cond_signal(&cond0);

		}
	}






	//Cycle to poll the messages for instance 2


	if(px4_instance == 2)
	{
		while (true)
		{
			pthread_mutex_lock(&lock0);
			while (!data_ready)
			{
				pthread_cond_wait(&cond0,&lock0);
			}

			int pret = ::poll(&fds[0], fd_count, 2000);


			if (pret == 0) {
				// Timed out.
				PX4_ERR("Receiving poll timeout %d, %d", pret, errno);
				continue;
			}

			if (pret < 0) {
				PX4_ERR("poll error %d, %d", pret, errno);
				continue;
			}


			for (int m = 0; m < fd_count; m++)
			{
				if (fds[m].revents == 0)
				{
					continue;
				}

				if (!(fds[m].revents & POLLIN))
				{
					continue;
				}

				if (fds[m].revents & POLLIN)
				{
					if(m==0)
					{
						int len = ::recvfrom(fds[0].fd, _buf, sizeof(_buf), 0, (struct sockaddr *)&remote_secondpilot_addr, (socklen_t *)&remote_secondpilot_addr_len);
						if (len > 0)
						{
							//PX4_INFO("Receiving messages");
							mavlink_message_t msg;

							for (int i = 0; i < len; i++)
							{
								if (mavlink_parse_char(MAVLINK_COMM_1, _buf[i], &msg, &mavlink_status))
								{
									handle_message(&msg, false);

								}
							}
						}
						if (len <= 0)
							{
								PX4_INFO("Error sending socket on second port");
							}

					} else {
						if (m==1)
						{
							int len = ::recvfrom(fds[1].fd, _buf, sizeof(_buf), 0, (struct sockaddr *)&remote_firstpilot_addr, (socklen_t *)&remote_firstpilot_addr_len);
							if (len > 0)
							{
								//PX4_INFO("Receiving messages");
								mavlink_message_t msg;
								for (int i = 0; i < len; i++)
								{
									if (mavlink_parse_char(MAVLINK_COMM_2, _buf[i], &msg, &mavlink_status))
									{
										handle_message(&msg, true);

									}
								}
							}
							if (len <= 0)
							{
								PX4_INFO("Error sending socket on first port");
							}

						}
					}

				}





			}


			check_pilots();
			data_ready=0;

			pthread_mutex_unlock(&lock0);
			pthread_cond_signal(&cond0);
		}
	}


}



void redundancy_manager::handle_message(const mavlink_message_t *msg, bool firstpilotreceiving)
{
	switch (msg->msgid)
	{
	case MAVLINK_MSG_ID_REDUNDANT_VEHICLE_STATUS:
		handle_message_redundant(msg, firstpilotreceiving);
		break;
	case MAVLINK_MSG_ID_SYS_STATUS:
		handle_message_sensors(msg, firstpilotreceiving);
		break;
	case MAVLINK_MSG_ID_HEARTBEAT:
		handle_message_heartbeat(msg, firstpilotreceiving);
		break;
	}
}



void redundancy_manager::handle_message_redundant(const mavlink_message_t *msg, bool firstpilotreceiving)
{

	mavlink_redundant_vehicle_status_t redundant_message;
	mavlink_msg_redundant_vehicle_status_decode(msg, &redundant_message);


	if (firstpilotreceiving == true)
	{
		redundant_firstpilotuorb.timestamp = hrt_absolute_time();
		redundant_firstpilotuorb.timestamp_pilot = redundant_message.time_usec;


		//Vehicle status flags


		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_SYSTEM_SENSORS_INITIALIZED) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_SYSTEM_SENSORS_INITIALIZED)
		{
			redundant_firstpilotuorb.condition_system_sensors_initialized = true;

		}else{
			redundant_firstpilotuorb.condition_system_sensors_initialized = false;
		}


		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ANGULAR_VELOCITY_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ANGULAR_VELOCITY_VALID)
		{
			redundant_firstpilotuorb.condition_angular_velocity_valid = true;
		}else{
			redundant_firstpilotuorb.condition_angular_velocity_valid= false;
		}


		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ATTITUDE_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ATTITUDE_VALID)
		{
			redundant_firstpilotuorb.condition_attitude_valid = true;
		} else {
			redundant_firstpilotuorb.condition_attitude_valid= false;
		}


		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_LOCAL_ALTITUDE_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_LOCAL_ALTITUDE_VALID)
		{
			redundant_firstpilotuorb.condition_local_altitude_valid = true;

		} else {
			redundant_firstpilotuorb.condition_local_altitude_valid= false;
		}


		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_LOCAL_POSITION_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_LOCAL_POSITION_VALID)
		{
			redundant_firstpilotuorb.condition_local_position_valid = true;

		} else {
			redundant_firstpilotuorb.condition_local_position_valid= false;
		}


		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_lOCAL_VELOCITY_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_lOCAL_VELOCITY_VALID)
		{
			redundant_firstpilotuorb.condition_local_velocity_valid = true;

		}else{
			redundant_firstpilotuorb.condition_local_velocity_valid= false;
		}




		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_HOME_POSITION_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_HOME_POSITION_VALID)
		{
			redundant_firstpilotuorb.condition_home_position_valid = true;

		}else{
			redundant_firstpilotuorb.condition_home_position_valid= false;
		}




		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_POWER_INPUT_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_POWER_INPUT_VALID)
		{
			redundant_firstpilotuorb.condition_power_input_valid = true;

		}else{
			redundant_firstpilotuorb.condition_power_input_valid= false;
		}




		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_BATTERY_HEALTHY_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_BATTERY_HEALTHY_VALID)
		{
			redundant_firstpilotuorb.condition_battery_healthy = true;

		}else{
			redundant_firstpilotuorb.condition_battery_healthy= false;
		}



		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ESCS_ERROR) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ESCS_ERROR)
		{
			redundant_firstpilotuorb.condition_escs_error = true;

		}else{
			redundant_firstpilotuorb.condition_escs_error= false;
		}


		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ESCS_FAILURE) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ESCS_FAILURE)
		{
			redundant_firstpilotuorb.condition_escs_failure = true;

		}else{
			redundant_firstpilotuorb.condition_escs_failure= false;
		}



		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_GLOBAL_POSITION_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_GLOBAL_POSITION_VALID)
		{
			redundant_firstpilotuorb.condition_global_position_valid = true;

		}else{
			redundant_firstpilotuorb.condition_global_position_valid= false;
		}

		//Vehicle_status_main

		if ((redundant_message.redundant_vehicle_status_main & REDUNDANT_VEHICLE_STATUS_MAIN::DATA_LINK_LOST) == REDUNDANT_VEHICLE_STATUS_MAIN::DATA_LINK_LOST)
		{
			redundant_firstpilotuorb.data_link_lost = true;

		}else{
			redundant_firstpilotuorb.data_link_lost= false;
		}




		if ((redundant_message.redundant_vehicle_status_main & REDUNDANT_VEHICLE_STATUS_MAIN::ENGINE_FAILURE) == REDUNDANT_VEHICLE_STATUS_MAIN::ENGINE_FAILURE)
		{
			redundant_firstpilotuorb.engine_failure = true;

		}else{
			redundant_firstpilotuorb.engine_failure= false;
		}



		if ((redundant_message.redundant_vehicle_status_main & REDUNDANT_VEHICLE_STATUS_MAIN::FAILSAFE) == REDUNDANT_VEHICLE_STATUS_MAIN::FAILSAFE)
		{
			redundant_firstpilotuorb.failsafe = true;

		}else{
			redundant_firstpilotuorb.failsafe= false;
		}



		if ((redundant_message.redundant_vehicle_status_main & REDUNDANT_VEHICLE_STATUS_MAIN::MISSION_FAILURE) == REDUNDANT_VEHICLE_STATUS_MAIN::MISSION_FAILURE)
		{
			redundant_firstpilotuorb.mission_failure = true;

		}else{
			redundant_firstpilotuorb.mission_failure= false;
		}


		redundant_firstpilotuorb.data_link_lost_counter = redundant_message.data_link_lost_counter;
		redundant_firstpilotuorb.failsafe_timestamp = redundant_message.failsafe_timestamp;
		redundant_firstpilotuorb.takeoff_time = redundant_message.takeoff_time;

		redundant_firstpilotuorb.failure_detector_status = redundant_message.redundant_vehicle_status_failuredetector;

		_redundancy_firstpilot_pub.publish(redundant_firstpilotuorb);
		PX4_INFO("Published first pilot");


	} else {

		redundant_secondpilotuorb.timestamp = hrt_absolute_time();
		redundant_secondpilotuorb.timestamp_pilot = redundant_message.time_usec;


		//Vehicle status flags

		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_SYSTEM_SENSORS_INITIALIZED) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_SYSTEM_SENSORS_INITIALIZED)
		{
			redundant_secondpilotuorb.condition_system_sensors_initialized = true;
		}else{

			redundant_secondpilotuorb.condition_system_sensors_initialized = false;

		}
		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ANGULAR_VELOCITY_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ANGULAR_VELOCITY_VALID)
		{
			redundant_secondpilotuorb.condition_angular_velocity_valid = true;
		}else{

			redundant_secondpilotuorb.condition_angular_velocity_valid = false;

		}


		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ATTITUDE_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ATTITUDE_VALID)
		{
			redundant_secondpilotuorb.condition_attitude_valid = true;
		}else{

			redundant_secondpilotuorb.condition_attitude_valid = false;

		}


		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_LOCAL_ALTITUDE_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_LOCAL_ALTITUDE_VALID)
		{
			redundant_secondpilotuorb.condition_local_altitude_valid = true;

		}else{

			redundant_secondpilotuorb.condition_local_altitude_valid = false;

		}




		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_LOCAL_POSITION_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_LOCAL_POSITION_VALID)
		{
			redundant_secondpilotuorb.condition_local_position_valid = true;

		}else{

			redundant_secondpilotuorb.condition_local_position_valid = false;

		}



		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_lOCAL_VELOCITY_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_lOCAL_VELOCITY_VALID)
		{
			redundant_secondpilotuorb.condition_local_velocity_valid = true;

		}else{

			redundant_secondpilotuorb.condition_local_velocity_valid = false;

		}




		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_HOME_POSITION_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_HOME_POSITION_VALID)
		{
			redundant_secondpilotuorb.condition_home_position_valid = true;

		}else{

			redundant_secondpilotuorb.condition_home_position_valid = false;

		}




		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_POWER_INPUT_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_POWER_INPUT_VALID)
		{
			redundant_secondpilotuorb.condition_power_input_valid = true;

		}else{

			redundant_secondpilotuorb.condition_power_input_valid = false;

		}


		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_BATTERY_HEALTHY_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_BATTERY_HEALTHY_VALID)
		{
			redundant_secondpilotuorb.condition_battery_healthy = true;
		}else{

			redundant_secondpilotuorb.condition_battery_healthy = false;

		}



		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ESCS_ERROR) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ESCS_ERROR)
		{
			redundant_secondpilotuorb.condition_escs_error = true;

		}else{

			redundant_secondpilotuorb.condition_escs_error = false;

		}



		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ESCS_FAILURE) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_ESCS_FAILURE)
		{
			redundant_secondpilotuorb.condition_escs_failure = true;

		}else{

			redundant_secondpilotuorb.condition_escs_failure = false;

		}


		if ((redundant_message.redundant_vehicle_status_flags & REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_GLOBAL_POSITION_VALID) == REDUNDANT_VEHICLE_STATUS_FLAGS::CONDITION_GLOBAL_POSITION_VALID)
		{
			redundant_secondpilotuorb.condition_global_position_valid = true;

		}else{

			redundant_secondpilotuorb.condition_global_position_valid = false;

		}

		//Vehicle_status_main

		if ((redundant_message.redundant_vehicle_status_main & REDUNDANT_VEHICLE_STATUS_MAIN::DATA_LINK_LOST) == REDUNDANT_VEHICLE_STATUS_MAIN::DATA_LINK_LOST)
		{
			redundant_secondpilotuorb.data_link_lost = true;

		}else{

			redundant_secondpilotuorb.data_link_lost = false;

		}

		if ((redundant_message.redundant_vehicle_status_main & REDUNDANT_VEHICLE_STATUS_MAIN::ENGINE_FAILURE) == REDUNDANT_VEHICLE_STATUS_MAIN::ENGINE_FAILURE)
		{
			redundant_secondpilotuorb.engine_failure = true;

		}else{

			redundant_secondpilotuorb.engine_failure = false;

		}



		if ((redundant_message.redundant_vehicle_status_main & REDUNDANT_VEHICLE_STATUS_MAIN::FAILSAFE) == REDUNDANT_VEHICLE_STATUS_MAIN::FAILSAFE)
		{
			redundant_secondpilotuorb.failsafe = true;

		}else{

			redundant_secondpilotuorb.failsafe = false;

		}



		if ((redundant_message.redundant_vehicle_status_main & REDUNDANT_VEHICLE_STATUS_MAIN::MISSION_FAILURE) == REDUNDANT_VEHICLE_STATUS_MAIN::MISSION_FAILURE)
		{
			redundant_secondpilotuorb.mission_failure = true;

		}else{

			redundant_secondpilotuorb.mission_failure = false;

		}


		redundant_secondpilotuorb.data_link_lost_counter = redundant_message.data_link_lost_counter;
		redundant_secondpilotuorb.failsafe_timestamp = redundant_message.failsafe_timestamp;
		redundant_secondpilotuorb.takeoff_time = redundant_message.takeoff_time;

		redundant_secondpilotuorb.failure_detector_status = redundant_message.redundant_vehicle_status_failuredetector;

		_redundancy_secondpilot_pub.publish(redundant_secondpilotuorb);
		PX4_INFO("Published second pilot");



	}

}



void redundancy_manager::handle_message_sensors(const mavlink_message_t *msg , bool firstpilotreceiving)
{
	mavlink_sys_status_t sensors_message;
	mavlink_msg_sys_status_decode(msg, &sensors_message);
	redundancy_firstpilot_sensors_s redundant_firstpilot_sensors_uorb;
	redundancy_secondpilot_sensors_s redundant_secondpilot_sensors_uorb;
	switch (px4_instance)
	{
	case 0:
		if (firstpilotreceiving == true && msg->sysid==2)
		{

			redundant_firstpilot_sensors_uorb.timestamp = hrt_absolute_time();
			redundant_firstpilot_sensors_uorb.onboard_control_sensors_present = sensors_message.onboard_control_sensors_present;
			redundant_firstpilot_sensors_uorb.onboard_control_sensors_enabled = sensors_message.onboard_control_sensors_enabled;
			redundant_firstpilot_sensors_uorb.onboard_control_sensors_health = sensors_message.onboard_control_sensors_health;
			_redundancy_firstpilot_sensors_pub.publish(redundant_firstpilot_sensors_uorb);

		} else {

			if (firstpilotreceiving == false && msg->sysid==3)
			{
				redundant_secondpilot_sensors_uorb.timestamp = hrt_absolute_time();
				redundant_secondpilot_sensors_uorb.onboard_control_sensors_present = sensors_message.onboard_control_sensors_present;
				redundant_secondpilot_sensors_uorb.onboard_control_sensors_enabled = sensors_message.onboard_control_sensors_enabled;
				redundant_secondpilot_sensors_uorb.onboard_control_sensors_health = sensors_message.onboard_control_sensors_health;

				_redundancy_secondpilot_sensors_pub.publish(redundant_secondpilot_sensors_uorb);
			}
		}
		break;
	case 1:
		if (firstpilotreceiving == true && msg->sysid==1)
		{

			redundant_firstpilot_sensors_uorb.timestamp = hrt_absolute_time();
			redundant_firstpilot_sensors_uorb.onboard_control_sensors_present = sensors_message.onboard_control_sensors_present;
			redundant_firstpilot_sensors_uorb.onboard_control_sensors_enabled = sensors_message.onboard_control_sensors_enabled;
			redundant_firstpilot_sensors_uorb.onboard_control_sensors_health = sensors_message.onboard_control_sensors_health;
			_redundancy_firstpilot_sensors_pub.publish(redundant_firstpilot_sensors_uorb);

		} else {

			if (firstpilotreceiving == false && msg->sysid==3)
			{
				redundant_secondpilot_sensors_uorb.timestamp = hrt_absolute_time();
				redundant_secondpilot_sensors_uorb.onboard_control_sensors_present = sensors_message.onboard_control_sensors_present;
				redundant_secondpilot_sensors_uorb.onboard_control_sensors_enabled = sensors_message.onboard_control_sensors_enabled;
				redundant_secondpilot_sensors_uorb.onboard_control_sensors_health = sensors_message.onboard_control_sensors_health;

				_redundancy_secondpilot_sensors_pub.publish(redundant_secondpilot_sensors_uorb);
			}
		}
		break;

	case 2:
		if (firstpilotreceiving == true && msg->sysid==1)
		{

			redundant_firstpilot_sensors_uorb.timestamp = hrt_absolute_time();
			redundant_firstpilot_sensors_uorb.onboard_control_sensors_present = sensors_message.onboard_control_sensors_present;
			redundant_firstpilot_sensors_uorb.onboard_control_sensors_enabled = sensors_message.onboard_control_sensors_enabled;
			redundant_firstpilot_sensors_uorb.onboard_control_sensors_health = sensors_message.onboard_control_sensors_health;
			_redundancy_firstpilot_sensors_pub.publish(redundant_firstpilot_sensors_uorb);

		} else {

			if (firstpilotreceiving == false && msg->sysid==2)
			{
				redundant_secondpilot_sensors_uorb.timestamp = hrt_absolute_time();
				redundant_secondpilot_sensors_uorb.onboard_control_sensors_present = sensors_message.onboard_control_sensors_present;
				redundant_secondpilot_sensors_uorb.onboard_control_sensors_enabled = sensors_message.onboard_control_sensors_enabled;
				redundant_secondpilot_sensors_uorb.onboard_control_sensors_health = sensors_message.onboard_control_sensors_health;

				_redundancy_secondpilot_sensors_pub.publish(redundant_secondpilot_sensors_uorb);
			}
		}

		break;
	default:
		break;
	}


}



void redundancy_manager::handle_message_heartbeat(const mavlink_message_t *msg , bool firstpilotreceiving)
{
	mavlink_heartbeat_t heartbeat_message;
	mavlink_msg_heartbeat_decode(msg, &heartbeat_message);
	redundancy_firstpilot_heartbeat_s redundancy_firstpilot_heartbeat_uorb;
	redundancy_secondpilot_heartbeat_s redundancy_secondpilot_heartbeat_uorb;


	switch (px4_instance)
	{
	case 0:

		if (firstpilotreceiving == true && msg->sysid==2)
		{
			redundancy_firstpilot_heartbeat_uorb.autopilot = heartbeat_message.autopilot;
			redundancy_firstpilot_heartbeat_uorb.timestamp = hrt_absolute_time();

			_redundancy_firstpilot_hearbeat_pub.publish(redundancy_firstpilot_heartbeat_uorb);

		} else {

			if (firstpilotreceiving == false && msg->sysid==3)
			{
				redundancy_secondpilot_heartbeat_uorb.autopilot = heartbeat_message.autopilot;
				redundancy_secondpilot_heartbeat_uorb.timestamp = hrt_absolute_time();

				_redundancy_secondpilot_hearbeat_pub.publish(redundancy_secondpilot_heartbeat_uorb);
			}
		}
		break;


	case 1:

		if (firstpilotreceiving == true && msg->sysid==1)
		{
			redundancy_firstpilot_heartbeat_uorb.autopilot = heartbeat_message.autopilot;
			redundancy_firstpilot_heartbeat_uorb.timestamp = hrt_absolute_time();

			_redundancy_firstpilot_hearbeat_pub.publish(redundancy_firstpilot_heartbeat_uorb);

		} else {

			if (firstpilotreceiving == false && msg->sysid==3)
			{
				redundancy_secondpilot_heartbeat_uorb.autopilot = heartbeat_message.autopilot;
				redundancy_secondpilot_heartbeat_uorb.timestamp = hrt_absolute_time();

				_redundancy_secondpilot_hearbeat_pub.publish(redundancy_secondpilot_heartbeat_uorb);
			}
		}
		break;

	case 2:

		if (firstpilotreceiving == true && msg->sysid==1)
		{
			redundancy_firstpilot_heartbeat_uorb.autopilot = heartbeat_message.autopilot;
			redundancy_firstpilot_heartbeat_uorb.timestamp = hrt_absolute_time();

			_redundancy_firstpilot_hearbeat_pub.publish(redundancy_firstpilot_heartbeat_uorb);

		} else {

			if (firstpilotreceiving == false && msg->sysid==2)
			{
				redundancy_secondpilot_heartbeat_uorb.autopilot = heartbeat_message.autopilot;
				redundancy_secondpilot_heartbeat_uorb.timestamp = hrt_absolute_time();

				_redundancy_secondpilot_hearbeat_pub.publish(redundancy_secondpilot_heartbeat_uorb);
			}
		}

	default:
		break;
	}




	if (firstpilotreceiving == true)
	{

		redundancy_firstpilot_heartbeat_uorb.autopilot = heartbeat_message.autopilot;
		redundancy_firstpilot_heartbeat_uorb.timestamp = hrt_absolute_time();

		_redundancy_firstpilot_hearbeat_pub.publish(redundancy_firstpilot_heartbeat_uorb);
	} else {

		redundancy_secondpilot_heartbeat_uorb.autopilot = heartbeat_message.autopilot;
		redundancy_secondpilot_heartbeat_uorb.timestamp = hrt_absolute_time();

		_redundancy_secondpilot_hearbeat_pub.publish(redundancy_secondpilot_heartbeat_uorb);

	}

}



//Comparison between autopilots data
void redundancy_manager::check_pilots()
{



	if (_estimator_selector_status_sub.updated())
	{


		if (_estimator_selector_status_sub.copy(&_estimator_selector_status))
		{
			if(hrt_elapsed_time(&_estimator_selector_status.timestamp) > 1_s)
			{
				estimator_selector_failing = true;
			}

			if (_estimator_selector_status.primary_instance != _estimator_status_sub.get_instance())
			{
				_estimator_status_sub.ChangeInstance(_estimator_selector_status.primary_instance);
				_estimator_status_flags_sub.ChangeInstance(_estimator_selector_status.primary_instance);
				_estimator_event_flags_sub.ChangeInstance(_estimator_selector_status.primary_instance);
			}

		}
	}


	if (_estimator_status_sub.update())
	{
		if(_estimator_status_sub.copy(&_estimator_status))
		{
			if(hrt_elapsed_time(&_estimator_status.timestamp) > 1_s)
			{
				estimator_failing = true;
			}

		}

	}

	if (_estimator_status_flags_sub.update())
	{
		if(_estimator_status_flags_sub.copy(&_estimator_status_flags))
		{
			if(hrt_elapsed_time(&_estimator_status_flags.timestamp) > 1_s)
			{
				estimator_failing = true;
			}


		}

	}

	if (_estimator_event_flags_sub.update())
	{
		if(_estimator_event_flags_sub.copy(&_estimator_event_flags))
		{
			if(hrt_elapsed_time(&_estimator_event_flags.timestamp) > 1_s)
			{
				estimator_failing = true;
			}

		}

	}

	if (_vehicle_status_sub.updated())
	{
		if(_vehicle_status_sub.copy(&_vehicle_status))
		{
			if(hrt_elapsed_time(&_vehicle_status.timestamp) > 1_s)
			{
				vehicle_failing = true;
			}
		}

	}

	if (_vehicle_status_flags_sub.updated())
	{
		if(_vehicle_status_flags_sub.copy(&_vehicle_status_flags))
		{
			if(hrt_elapsed_time(&_vehicle_status_flags.timestamp) > 1_s)
			{
				vehicle_failing = true;
			}
		}

	}


	if (hrt_elapsed_time(&_vehicle_status.takeoff_time) > 10_s)
	{

		//test_seconds = hrt_elapsed_time(&_vehicle_status.takeoff_time);
		//PX4_INFO("Checking test_seconds after the takeoff %ld", test_seconds);

		check_failing_flags();
		define_pilots_condition();
		voting_best_pilot();

		px4_sleep(1);


	}


	send_selection();

}



void redundancy_manager::define_pilots_condition()
{

	//Defining First Pilot condition
	if (!firstpilotfailing)
	{
		time_last_firstpilot_pass = hrt_absolute_time();
		if (hrt_elapsed_time(&time_last_firstpilot_fail) > 150_s )
		{
			firstpilotfailed = false;
		}


	} else {

		time_last_firstpilot_fail = hrt_absolute_time();
		if (hrt_elapsed_time(&time_last_firstpilot_pass) > 2_s)
		{
			firstpilotfailed = true;
		}

	}


	//Defining second pilot condition
	if (!secondpilotfailing)
	{
		time_last_secondpilot_pass = hrt_absolute_time();
		if (hrt_elapsed_time(&time_last_secondpilot_fail) > 150_s )
		{
			secondpilotfailed = false;
		}


	} else {
		time_last_secondpilot_fail = hrt_absolute_time();
		if (hrt_elapsed_time(&time_last_secondpilot_pass) > 3_s)
		{
			secondpilotfailed = true;
		}

	}

	//Defining my pilot condition

	if (!mypilotfailing)
	{
		time_last_mypilot_pass = hrt_absolute_time();
		if (hrt_elapsed_time(&time_last_mypilot_fail) > 150_s)
		{
			mypilotfailed = false;
		}


	} else {
		time_last_mypilot_fail = hrt_absolute_time();
		if (hrt_elapsed_time(&time_last_mypilot_pass) > 3_s)
		{
			mypilotfailed = true;
		}

	}

}


void redundancy_manager::voting_best_pilot()
{

	switch (px4_instance)
	{
	case 0:
		pilot_healthy[0] = (!mypilotfailed);
		pilot_healthy[1] = (!firstpilotfailed);
		pilot_healthy[2] = (!secondpilotfailed);
		pilot_priority[0] = pilot_priority_1[0];
		pilot_priority[1] = pilot_priority_1[1];
		pilot_priority[2] = pilot_priority_1[2];
		pilot_error_count[0] = counter_pilotfailing[0];
		pilot_error_count[1] = counter_pilotfailing[1];
		pilot_error_count[2] = counter_pilotfailing[2];

		break;
	case 1:
		pilot_healthy[0] = (!firstpilotfailed);
		pilot_healthy[1] = (!mypilotfailed);
		pilot_healthy[2] = (!secondpilotfailed);
		pilot_priority[0] = pilot_priority_1[1];
		pilot_priority[1] = pilot_priority_1[0];
		pilot_priority[2] = pilot_priority_1[2];
		pilot_error_count[0] = counter_pilotfailing[1];
		pilot_error_count[1] = counter_pilotfailing[0];
		pilot_error_count[2] = counter_pilotfailing[2];
		break;
	case 2:
		pilot_healthy[0] = (!firstpilotfailed);
		pilot_healthy[1] = (!secondpilotfailed);
		pilot_healthy[2] = (!mypilotfailed);
		pilot_priority[0] = pilot_priority_1[1];
		pilot_priority[1] = pilot_priority_1[2];
		pilot_priority[2] = pilot_priority_1[0];
		pilot_error_count[0] = counter_pilotfailing[1];
		pilot_error_count[1] = counter_pilotfailing[2];
		pilot_error_count[2] = counter_pilotfailing[0];
		break;
	default:
		break;
	}

	switch (px4_instance)
	{
	case 0:

		//Pilot_1
		redundancy_selection_uorb.condition_system_sensors_initialized_1 = mypilotfaults_1[0];
		redundancy_selection_uorb.condition_angular_velocity_valid_1 = mypilotfaults_1[1];
		redundancy_selection_uorb.condition_local_altitude_valid_1 = mypilotfaults_1[3];
		redundancy_selection_uorb.condition_local_position_valid_1 = mypilotfaults_1[4];
		redundancy_selection_uorb.condition_local_velocity_valid_1 = mypilotfaults_1[5];
		redundancy_selection_uorb.condition_global_position_valid_1 = mypilotfaults_1[6];
		redundancy_selection_uorb.condition_home_position_valid_1 = mypilotfaults_1[7];
		redundancy_selection_uorb.condition_battery_healthy_1 = mypilotfaults_1[8];
		redundancy_selection_uorb.failsafe_1 = mypilotfaults_1[9];
		redundancy_selection_uorb.data_link_lost_1 = mypilotfaults_1[10];
		redundancy_selection_uorb.mission_failure_1 = mypilotfaults_1[11];

		//Pilot_2
		redundancy_selection_uorb.condition_system_sensors_initialized_2 = firstpilotfaults_1[0];
		redundancy_selection_uorb.condition_angular_velocity_valid_2 = firstpilotfaults_1[1];
		redundancy_selection_uorb.condition_local_altitude_valid_2 = firstpilotfaults_1[3];
		redundancy_selection_uorb.condition_local_position_valid_2 = firstpilotfaults_1[4];
		redundancy_selection_uorb.condition_local_velocity_valid_2 = firstpilotfaults_1[5];
		redundancy_selection_uorb.condition_global_position_valid_2 = firstpilotfaults_1[6];
		redundancy_selection_uorb.condition_home_position_valid_2 = firstpilotfaults_1[7];
		redundancy_selection_uorb.condition_battery_healthy_2 = firstpilotfaults_1[8];
		redundancy_selection_uorb.failsafe_2 = firstpilotfaults_1[9];
		redundancy_selection_uorb.data_link_lost_2 = firstpilotfaults_1[10];
		redundancy_selection_uorb.mission_failure_2 = firstpilotfaults_1[11];


		//Pilot_3
		redundancy_selection_uorb.condition_system_sensors_initialized_3 = secondpilotfaults_1[0];
		redundancy_selection_uorb.condition_angular_velocity_valid_3 = secondpilotfaults_1[1];
		redundancy_selection_uorb.condition_local_altitude_valid_3 = secondpilotfaults_1[3];
		redundancy_selection_uorb.condition_local_position_valid_3 = secondpilotfaults_1[4];
		redundancy_selection_uorb.condition_local_velocity_valid_3 = secondpilotfaults_1[5];
		redundancy_selection_uorb.condition_global_position_valid_3 = secondpilotfaults_1[6];
		redundancy_selection_uorb.condition_home_position_valid_3 = secondpilotfaults_1[7];
		redundancy_selection_uorb.condition_battery_healthy_3 = secondpilotfaults_1[8];
		redundancy_selection_uorb.failsafe_3 = secondpilotfaults_1[9];
		redundancy_selection_uorb.data_link_lost_3 = secondpilotfaults_1[10];
		redundancy_selection_uorb.mission_failure_3 = secondpilotfaults_1[11];


		break;
	case 1:
		//Pilot 1
		redundancy_selection_uorb.condition_system_sensors_initialized_1 = firstpilotfaults_1[0];
		redundancy_selection_uorb.condition_angular_velocity_valid_1 = firstpilotfaults_1[1];
		redundancy_selection_uorb.condition_local_altitude_valid_1 = firstpilotfaults_1[3];
		redundancy_selection_uorb.condition_local_position_valid_1 = firstpilotfaults_1[4];
		redundancy_selection_uorb.condition_local_velocity_valid_1 = firstpilotfaults_1[5];
		redundancy_selection_uorb.condition_global_position_valid_1 = firstpilotfaults_1[6];
		redundancy_selection_uorb.condition_home_position_valid_1 = firstpilotfaults_1[7];
		redundancy_selection_uorb.condition_battery_healthy_1 = firstpilotfaults_1[8];
		redundancy_selection_uorb.failsafe_1 = firstpilotfaults_1[9];
		redundancy_selection_uorb.data_link_lost_1 = firstpilotfaults_1[10];
		redundancy_selection_uorb.mission_failure_1 = firstpilotfaults_1[11];

		//Pilot 2
		redundancy_selection_uorb.condition_system_sensors_initialized_2 = mypilotfaults_1[0];
		redundancy_selection_uorb.condition_angular_velocity_valid_2 = mypilotfaults_1[1];
		redundancy_selection_uorb.condition_local_altitude_valid_2 = mypilotfaults_1[3];
		redundancy_selection_uorb.condition_local_position_valid_2 = mypilotfaults_1[4];
		redundancy_selection_uorb.condition_local_velocity_valid_2 = mypilotfaults_1[5];
		redundancy_selection_uorb.condition_global_position_valid_2 = mypilotfaults_1[6];
		redundancy_selection_uorb.condition_home_position_valid_2 = mypilotfaults_1[7];
		redundancy_selection_uorb.condition_battery_healthy_2 = mypilotfaults_1[8];
		redundancy_selection_uorb.failsafe_2 = mypilotfaults_1[9];
		redundancy_selection_uorb.data_link_lost_2 = mypilotfaults_1[10];
		redundancy_selection_uorb.mission_failure_2 = mypilotfaults_1[11];

		//Pilot 3

		redundancy_selection_uorb.condition_system_sensors_initialized_3 = secondpilotfaults_1[0];
		redundancy_selection_uorb.condition_angular_velocity_valid_3 = secondpilotfaults_1[1];
		redundancy_selection_uorb.condition_local_altitude_valid_3 = secondpilotfaults_1[3];
		redundancy_selection_uorb.condition_local_position_valid_3 = secondpilotfaults_1[4];
		redundancy_selection_uorb.condition_local_velocity_valid_3 = secondpilotfaults_1[5];
		redundancy_selection_uorb.condition_global_position_valid_3 = secondpilotfaults_1[6];
		redundancy_selection_uorb.condition_home_position_valid_3 = secondpilotfaults_1[7];
		redundancy_selection_uorb.condition_battery_healthy_3 = secondpilotfaults_1[8];
		redundancy_selection_uorb.failsafe_3 = secondpilotfaults_1[9];
		redundancy_selection_uorb.data_link_lost_3 = secondpilotfaults_1[10];
		redundancy_selection_uorb.mission_failure_3 = secondpilotfaults_1[11];
		break;

	case 2:

		//Pilot 1
		redundancy_selection_uorb.condition_system_sensors_initialized_1 = firstpilotfaults_1[0];
		redundancy_selection_uorb.condition_angular_velocity_valid_1 = firstpilotfaults_1[1];
		redundancy_selection_uorb.condition_local_altitude_valid_1 = firstpilotfaults_1[3];
		redundancy_selection_uorb.condition_local_position_valid_1 = firstpilotfaults_1[4];
		redundancy_selection_uorb.condition_local_velocity_valid_1 = firstpilotfaults_1[5];
		redundancy_selection_uorb.condition_global_position_valid_1 = firstpilotfaults_1[6];
		redundancy_selection_uorb.condition_home_position_valid_1 = firstpilotfaults_1[7];
		redundancy_selection_uorb.condition_battery_healthy_1 = firstpilotfaults_1[8];
		redundancy_selection_uorb.failsafe_1 = firstpilotfaults_1[9];
		redundancy_selection_uorb.data_link_lost_1 = firstpilotfaults_1[10];
		redundancy_selection_uorb.mission_failure_1 = firstpilotfaults_1[11];

		//Pilot 2
		redundancy_selection_uorb.condition_system_sensors_initialized_2 = secondpilotfaults_1[0];
		redundancy_selection_uorb.condition_angular_velocity_valid_2 = secondpilotfaults_1[1];
		redundancy_selection_uorb.condition_local_altitude_valid_2 = secondpilotfaults_1[3];
		redundancy_selection_uorb.condition_local_position_valid_2 = secondpilotfaults_1[4];
		redundancy_selection_uorb.condition_local_velocity_valid_2 = secondpilotfaults_1[5];
		redundancy_selection_uorb.condition_global_position_valid_2 = secondpilotfaults_1[6];
		redundancy_selection_uorb.condition_home_position_valid_2 = secondpilotfaults_1[7];
		redundancy_selection_uorb.condition_battery_healthy_2 = secondpilotfaults_1[8];
		redundancy_selection_uorb.failsafe_2 = secondpilotfaults_1[9];
		redundancy_selection_uorb.data_link_lost_2 = secondpilotfaults_1[10];
		redundancy_selection_uorb.mission_failure_2 = secondpilotfaults_1[11];

		//Pilot 3
		redundancy_selection_uorb.condition_system_sensors_initialized_3 = mypilotfaults_1[0];
		redundancy_selection_uorb.condition_angular_velocity_valid_3 = mypilotfaults_1[1];
		redundancy_selection_uorb.condition_local_altitude_valid_3 = mypilotfaults_1[3];
		redundancy_selection_uorb.condition_local_position_valid_3 = mypilotfaults_1[4];
		redundancy_selection_uorb.condition_local_velocity_valid_3 = mypilotfaults_1[5];
		redundancy_selection_uorb.condition_global_position_valid_3 = mypilotfaults_1[6];
		redundancy_selection_uorb.condition_home_position_valid_3 = mypilotfaults_1[7];
		redundancy_selection_uorb.condition_battery_healthy_3 = mypilotfaults_1[8];
		redundancy_selection_uorb.failsafe_3 = mypilotfaults_1[9];
		redundancy_selection_uorb.data_link_lost_3 = mypilotfaults_1[10];
		redundancy_selection_uorb.mission_failure_3 = mypilotfaults_1[11];
		break;

	default:
		break;
	}



	PX4_INFO("Testing error counters.counter_pilotfailing[0]: %d, counter_pilotfailing[1]: %d, counter_pilotfailing[2]: %d \n", counter_pilotfailing[0],counter_pilotfailing[1],counter_pilotfailing[2]);

	PX4_INFO("Testing priorities. Pilot_priority_1[0]: %d . Pilot_priority_1[1]: %d. Pilot_priority_1[2]: %d. \n",pilot_priority_1[0],pilot_priority_1[1], pilot_priority_1[2]);

	PX4_INFO("Testing priorities. Pilot_priority[0]: %d . Pilot_priority[1]: %d. Pilot_priority[2]: %d. \n",pilot_priority[0],pilot_priority[1], pilot_priority[2]);


	best_pilot_priority = 3;

	for (size_t i = 0; i < 3; i++)
	{

		if (pilot_priority[i] < best_pilot_priority)
		{
			best_pilot = i + 1;
			best_pilot_priority = pilot_priority[i];
		}

	}




	if (previous_best_pilot != best_pilot)
	{
		PX4_WARN("Changed best pilot - From %d to %d", previous_best_pilot, best_pilot);
	}

	previous_best_pilot = best_pilot;

	//Publish the results on the uorb topic


	redundancy_selection_uorb.pilot1_healthy = pilot_healthy[0];
	redundancy_selection_uorb.pilot1_priority = pilot_priority[0];
	redundancy_selection_uorb.pilot2_healthy = pilot_healthy[1];
	redundancy_selection_uorb.pilot2_priority = pilot_priority[1];
	redundancy_selection_uorb.pilot3_healthy = pilot_healthy[2];
	redundancy_selection_uorb.pilot3_priority = pilot_priority[2];

	redundancy_selection_uorb.pilot_selection = best_pilot;
	redundancy_selection_uorb.best_pilot_priority = best_pilot_priority;
	redundancy_selection_uorb.timestamp = hrt_absolute_time();

	redundancy_selection_uorb.pilot1_error_count = pilot_error_count[0];
	redundancy_selection_uorb.pilot2_error_count = pilot_error_count[1];
	redundancy_selection_uorb.pilot3_error_count = pilot_error_count[2];

	_redundancy_selection_pub.publish(redundancy_selection_uorb);


}



void redundancy_manager::send_selection()
{
	uint8_t  buf[MAVLINK_MAX_PACKET_LEN];
	uint16_t bufLen = 0;
	mavlink_redundant_vehicle_selected_t px4_selected_message = {};
	mavlink_message_t message_3 = {};
	ssize_t len_1;


	ssize_t len_2;



	memset(buf,0,sizeof(buf));

	memset(&px4_selected_message,0,sizeof(mavlink_redundant_vehicle_selected_t));
	px4_selected_message.time_usec = hrt_absolute_time();
	px4_selected_message.redundant_px4_selected = best_pilot;


	mavlink_msg_redundant_vehicle_selected_encode(_param_mav_sys_id.get(), _param_mav_comp_id.get(), &message_3, &px4_selected_message);



	bufLen = mavlink_msg_to_send_buffer(buf, &message_3);



	len_1 = ::sendto(selection_socket, buf, bufLen, 0, (struct sockaddr*)&addr_selection, addr_selection_len);

	if (len_1 <= 0)
	{
		PX4_ERR("Error sending selection message to the simulator interface");
	}


	if (len_1 >0)
	{
		PX4_INFO("PX4 sending selection pilot: %d to the simulator\n",best_pilot);
	}




	len_2 = ::sendto(qgc_selection_socket, buf, bufLen, 0, (struct sockaddr*)&addr_qgc_selection, addr_qgc_selection_len);

	if (len_2 <= 0)
	{
		PX4_ERR("Error sending selection message to the QGC interface");
	}



	if (len_2 > 0)
	{
		PX4_INFO("PX4 sending selection pilot: %d to the QgroundControl\n", best_pilot);
	}



}


void redundancy_manager::check_failing_flags()
{
	//RESET FAILINGS
	mypilotfailing = false;
	firstpilotfailing = false;
	secondpilotfailing = false;


	//RESET COUNTERS

	counter_failing_worst = 1;
	counter_failing_worst_2 = 1;


	for (size_t i = 0; i < 3; i++)
	{
		pilot_priority[i] = 1;

	}

	for (size_t i = 0; i < 3; i++)
	{
		pilot_priority_1[i] = 1;

	}



	for (size_t i = 0; i < 3; i++)
	{
		counter_pilotfailing[i] = 0;

	}


	for (int i = 0; i < error_flags_number; i++)
	{
		mypilotfaults[i] = false;
		firstpilotfaults[i] = false;
		secondpilotfaults[i] = false;

	}

	//Verify communications





	communication_firstpilot = hrt_elapsed_time(&redundant_firstpilotuorb.timestamp_pilot);
	PX4_INFO("Time elapsed from the first pilot message timestamp %ld", communication_firstpilot);


	communication_secondpilot = hrt_elapsed_time(&redundant_secondpilotuorb.timestamp_pilot);
	PX4_INFO("Time elapsed from the second pilot message timestamp %ld", communication_secondpilot);




	//PX4_INFO("First pilot message timestamp %ld", redundant_firstpilotuorb.timestamp_pilot);
	//PX4_INFO("Second pilot message timestamp %ld", redundant_secondpilotuorb.timestamp_pilot);



	if (hrt_elapsed_time(&redundant_firstpilotuorb.timestamp_pilot) > 3_s)
	{
		PX4_INFO("Failed communication on the first pilot. Decrease priority.");
		communication_fail_firstpilot = true;


	} else {

		PX4_INFO("Good communication on the first pilot.");
	}

	if (hrt_elapsed_time(&redundant_secondpilotuorb.timestamp_pilot) > 3_s)
	{
		PX4_INFO("Failed communication on the second pilot. Decrease priority.");
		communication_fail_secondpilot = true;

	} else {

		PX4_INFO("Good communication on the second pilot.");
	}



	//Vehicle_status_flags topic
	if (_vehicle_status_flags.condition_system_sensors_initialized != redundant_firstpilotuorb.condition_system_sensors_initialized)
	{

		if (!redundant_secondpilotuorb.condition_system_sensors_initialized)
		{
			secondpilotfaults[0] = true;
			PX4_INFO("Pilot[2] sensors initialized error");




		}


		if (_vehicle_status_flags.condition_system_sensors_initialized == true)
		{

			firstpilotfaults[0] = true;
			PX4_INFO("Pilot[1] sensors initialized error");


		} else {

			mypilotfaults[0] = true;
			PX4_INFO("Pilot[0] sensors initialized error");


		}

	} else {

		if (redundant_secondpilotuorb.condition_system_sensors_initialized != _vehicle_status_flags.condition_system_sensors_initialized)
		{

			if (redundant_secondpilotuorb.condition_system_sensors_initialized == true)
			{
				mypilotfaults[0] = true;
				PX4_INFO("Pilot[0] sensors initialized error");
				firstpilotfaults[0] = true;
				PX4_INFO("Pilot[1] sensors initialized error");

			} else {

				secondpilotfaults[0] = true;
				PX4_INFO("Pilot[2] sensors initialized error");

			}



		}

	}



	if (_vehicle_status_flags.condition_angular_velocity_valid!= redundant_firstpilotuorb.condition_angular_velocity_valid)
	{

		if (!redundant_secondpilotuorb.condition_angular_velocity_valid)
		{
			secondpilotfaults[1] = true;
			PX4_INFO("Pilot[2] condition_angular_velocity error");

		}


		if (_vehicle_status_flags.condition_angular_velocity_valid)
		{


			firstpilotfaults[1] = true;
			PX4_INFO("Pilot[1] condition_angular_velocity error");



		} else {

			mypilotfaults[1] = true;
			PX4_INFO("Pilot[0] condition_angular_velocity error");



		}

	} else {

		if (redundant_secondpilotuorb.condition_angular_velocity_valid != _vehicle_status_flags.condition_angular_velocity_valid)
		{

			if (redundant_secondpilotuorb.condition_angular_velocity_valid == true)
			{
				mypilotfaults[1] = true;
				PX4_INFO("Pilot[0] condition_angular_velocity error");
				firstpilotfaults[1] = true;
				PX4_INFO("Pilot[1] condition_angular_velocity error");

			} else {

				secondpilotfaults[1] = true;
				PX4_INFO("Pilot[2] condition_angular_velocity error");

			}



		}
	}




	if (_vehicle_status_flags.condition_local_altitude_valid!= redundant_firstpilotuorb.condition_local_altitude_valid)
	{
		if (!redundant_secondpilotuorb.condition_local_altitude_valid)
		{
			secondpilotfaults[3] = true;
			PX4_INFO("Pilot[2] condition_local_altitude error");
		}



		if (_vehicle_status_flags.condition_local_altitude_valid)
		{
			firstpilotfaults[3] = true;
			PX4_INFO("Pilot[1] condition_local_altitude error");

		} else {

			mypilotfaults[3] = true;
			PX4_INFO("Pilot[0] condition_local_altitude error");

		}

	} else {

		if (redundant_secondpilotuorb.condition_local_altitude_valid != _vehicle_status_flags.condition_local_altitude_valid)
		{

			if (redundant_secondpilotuorb.condition_local_altitude_valid == true)
			{
				mypilotfaults[3] = true;
				PX4_INFO("Pilot[0] condition_local_altitude error");
				firstpilotfaults[3] = true;
				PX4_INFO("Pilot[1] condition_local_altitude error");

			} else {

				secondpilotfaults[3] = true;
				PX4_INFO("Pilot[2] condition_local_altitude error");

			}

		}
	}



	if (_vehicle_status_flags.condition_local_position_valid!= redundant_firstpilotuorb.condition_local_position_valid)
	{

		if (!redundant_secondpilotuorb.condition_local_position_valid)
		{
			secondpilotfaults[4] = true;
			PX4_INFO("Pilot[2] condition_local_position error");
		}

		if (_vehicle_status_flags.condition_local_position_valid)
		{

			firstpilotfaults[4] = true;
			PX4_INFO("Pilot[1] condition_local_position error");

		} else {


			mypilotfaults[4] = true;
			PX4_INFO("Pilot[0] condition_local_position error");
		}

	} else {
		if (redundant_secondpilotuorb.condition_local_position_valid !=_vehicle_status_flags.condition_local_position_valid )
		{

			if (redundant_secondpilotuorb.condition_local_position_valid == true)
			{
				mypilotfaults[4] = true;
				PX4_INFO("Pilot[0] condition_local_position error");
				firstpilotfaults[4] = true;
				PX4_INFO("Pilot[1] condition_local_position error");

			} else {

				secondpilotfaults[4] = true;
				PX4_INFO("Pilot[2] condition_local_position error");

			}

		}
	}


	if (_vehicle_status_flags.condition_local_velocity_valid!= redundant_firstpilotuorb.condition_local_velocity_valid)
	{

		if (!redundant_secondpilotuorb.condition_local_velocity_valid)
		{
			secondpilotfaults[5] = true;
			PX4_INFO("Pilot[2] condition_local_velocity error");
		}


		if (_vehicle_status_flags.condition_local_velocity_valid)
		{


			firstpilotfaults[5] = true;
			PX4_INFO("Pilot[1] condition_local_velocity error");

		} else {

			mypilotfaults[5] = true;
			PX4_INFO("Pilot[0] condition_local_velocity error");

		}

	} else {

		if (redundant_secondpilotuorb.condition_local_velocity_valid !=_vehicle_status_flags.condition_local_velocity_valid)
		{

			if (redundant_secondpilotuorb.condition_local_velocity_valid == true)
			{
				mypilotfaults[5] = true;
				PX4_INFO("Pilot[0] condition_local_velocity error");
				firstpilotfaults[5] = true;
				PX4_INFO("Pilot[1] condition_local_velocity error");

			} else {

				secondpilotfaults[5] = true;
				PX4_INFO("Pilot[2] condition_local_velocity error");

			}
		}
	}




	if (_vehicle_status_flags.condition_global_position_valid!= redundant_firstpilotuorb.condition_global_position_valid)
	{

		if (!redundant_secondpilotuorb.condition_global_position_valid)
		{
			secondpilotfaults[6] = true;
			PX4_INFO("Pilot[2] condition_global_position error");
		}



		if (_vehicle_status_flags.condition_global_position_valid)
		{

			firstpilotfaults[6]=true;
			PX4_INFO("Pilot[1] condition_global_position error");

		} else {
			if (redundant_secondpilotuorb.condition_global_position_valid)
			{

				mypilotfaults[6] = true;
				PX4_INFO("Pilot[0] condition_global_position error");
			}

		}

	} else {
		if (redundant_secondpilotuorb.condition_global_position_valid!=_vehicle_status_flags.condition_global_position_valid)
		{
			if (redundant_secondpilotuorb.condition_global_position_valid == true)
			{
				mypilotfaults[6] = true;
				PX4_INFO("Pilot[0] condition_global_position error");
				firstpilotfaults[6] = true;
				PX4_INFO("Pilot[1] condition_global_position error");

			} else {

				secondpilotfaults[6] = true;
				PX4_INFO("Pilot[2] condition_global_position error");

			}

		}
	}




	if (_vehicle_status_flags.condition_home_position_valid != redundant_firstpilotuorb.condition_home_position_valid)
	{

		if (!redundant_secondpilotuorb.condition_home_position_valid)
		{
			secondpilotfaults[7] = true;
			PX4_INFO("Pilot[2] condition_home_position error");
		}



		if (_vehicle_status_flags.condition_home_position_valid)
		{

			firstpilotfaults[7] = true;
			PX4_INFO("Pilot[1] condition_home_position error");

		} else {

			mypilotfaults[7]=true;
			PX4_INFO("Pilot[0] condition_home_position error");

		}

	} else {
		if (redundant_secondpilotuorb.condition_home_position_valid != _vehicle_status_flags.condition_home_position_valid)
		{

			if (redundant_secondpilotuorb.condition_home_position_valid == true)
			{
				mypilotfaults[7] = true;
				PX4_INFO("Pilot[0] condition_home_position error");
				firstpilotfaults[7] = true;
				PX4_INFO("Pilot[1] condition_home_position error");

			} else {

				secondpilotfaults[7] = true;
				PX4_INFO("Pilot[2] condition_home_position error");

			}

		}
	}



	if (_vehicle_status_flags.condition_battery_healthy!= redundant_firstpilotuorb.condition_battery_healthy)
	{

		if (!redundant_secondpilotuorb.condition_battery_healthy)
		{
			secondpilotfaults[8] = true;
			PX4_INFO("Pilot[2] condition_battery error");
		}


		if (_vehicle_status_flags.condition_battery_healthy)
		{
			firstpilotfaults[8] = true;
			PX4_INFO("Pilot[1] condition_battery error");

		} else {

			mypilotfaults[8] = true;
			PX4_INFO("Pilot[0] condition_battery error");

		}

	} else {


		if (redundant_secondpilotuorb.condition_battery_healthy != _vehicle_status_flags.condition_battery_healthy)
		{
			if (redundant_secondpilotuorb.condition_battery_healthy == true)
			{
				mypilotfaults[8] = true;
				PX4_INFO("Pilot[0] condition_battery error");
				firstpilotfaults[8] = true;
				PX4_INFO("Pilot[1] condition_battery error");

			} else {

				secondpilotfaults[8] = true;
				PX4_INFO("Pilot[2] condition_battery error");

			}

		}
	}






	//Vehicle_status topic

	if (_vehicle_status.failsafe != redundant_firstpilotuorb.failsafe)
	{
		if (redundant_secondpilotuorb.failsafe)
		{
			secondpilotfaults[9] = true;
			PX4_INFO("Pilot[2] failsafe error");
		}

		if (!_vehicle_status.failsafe)
		{

			firstpilotfaults[9] = true;
			PX4_INFO("Pilot[1] failsafe error");

		} else {

			mypilotfaults[9] = true;
			PX4_INFO("Pilot[0] failsafe error");

		}

	} else {

		if (redundant_secondpilotuorb.failsafe != _vehicle_status.failsafe)
		{
			if (redundant_secondpilotuorb.failsafe == true)
			{
				secondpilotfaults[9] = true;
				PX4_INFO("Pilot[2] failsafe error");

			} else {

				mypilotfaults[9] = true;
				PX4_INFO("Pilot[0] failsafe error");
				firstpilotfaults[9] = true;
				PX4_INFO("Pilot[1] failsafe error");
			}



		}
	}





	if (_vehicle_status.data_link_lost != redundant_firstpilotuorb.data_link_lost)
	{


		if (redundant_secondpilotuorb.data_link_lost)
		{
			secondpilotfaults[10] = true;
			PX4_INFO("Pilot[2] data_link_lost error");
		}



		if (!_vehicle_status.data_link_lost)
		{
			firstpilotfaults[10] = true;
			PX4_INFO("Pilot[1] data_link_lost error");

		} else {
			mypilotfaults[10] = true;
			PX4_INFO("Pilot[0] data_link_lost error");

		}

	} else {


		if (redundant_secondpilotuorb.data_link_lost!= _vehicle_status.data_link_lost)
		{
			if (redundant_secondpilotuorb.data_link_lost == true)
			{
				secondpilotfaults[10] = true;
				PX4_INFO("Pilot[2] data_link_lost error");

			} else {

				mypilotfaults[10] = true;
				PX4_INFO("Pilot[0] data_link_lost error");
				firstpilotfaults[10] = true;
				PX4_INFO("Pilot[1] data_link_lost error");
			}



		}
	}


	if (_vehicle_status.mission_failure != redundant_firstpilotuorb.mission_failure)
	{
		if (redundant_secondpilotuorb.mission_failure)
		{
			secondpilotfaults[11] = true;
			PX4_INFO("Pilot[2] mission failure error");
		}

		if (_vehicle_status.mission_failure)
		{
			if (redundant_firstpilotuorb.mission_failure)
			{
				firstpilotfaults[11] = true;
				PX4_INFO("Pilot[1] mission failure error");
			}

		} else {
			if (redundant_secondpilotuorb.mission_failure)
			{
				mypilotfaults[11] = true;
				PX4_INFO("Pilot[0] mission failure error");

			}

		}

	} else {



		if (redundant_secondpilotuorb.mission_failure!= _vehicle_status.mission_failure)
		{
			if (redundant_secondpilotuorb.mission_failure == true)
			{
				secondpilotfaults[11] = true;
				PX4_INFO("Pilot[2] mission failure error");

			} else {

				mypilotfaults[11] = true;
				PX4_INFO("Pilot[0] mission failure error");
				firstpilotfaults[11] = true;
				PX4_INFO("Pilot[1] mission failure error");
			}

		}

	}







	/* Frequent errors

	if (_vehicle_status_flags.condition_attitude_valid!= redundant_firstpilotuorb.condition_attitude_valid)
	{

		if (!redundant_secondpilotuorb.condition_attitude_valid)
		{
			secondpilotfaults[2] = true;
			PX4_INFO("Pilot[2] condition_attitude error");

		}


		if (_vehicle_status_flags.condition_attitude_valid)
		{

			firstpilotfaults[2]=true;
			PX4_INFO("Pilot[1] condition_attitude error");


		} else {

			mypilotfaults[2] = true;
			PX4_INFO("Pilot[0] condition_attitude error");


		}

	} else {

		if (redundant_secondpilotuorb.condition_attitude_valid != _vehicle_status_flags.condition_attitude_valid)
		{
			secondpilotfaults[2] = true;
			PX4_INFO("Pilot[2] condition_attitude error");


		}
	}



	//sensors

	Redundant in case of having the same sensors

	if (_vehicle_status.onboard_control_sensors_present != redundant_firstpilotuorb.onboard_control_sensors_present)
	{
		if (_vehicle_status.onboard_control_sensors_present == redundant_secondpilotuorb.onboard_control_sensors_present )
		{
			firstpilotfailing = true;

		} else {
			if (redundant_firstpilotuorb.onboard_control_sensors_present == redundant_secondpilotuorb.onboard_control_sensors_present)
			{
				mypilotfailing = true;
			}

		}

	} else {
		if (_vehicle_status.onboard_control_sensors_present != redundant_secondpilotuorb.onboard_control_sensors_present)
		{
			secondpilotfailing = true;
		}
	}





	if (_vehicle_status.onboard_control_sensors_enabled != redundant_firstpilotuorb.onboard_control_sensors_enabled)
	{
		if (_vehicle_status.onboard_control_sensors_enabled== redundant_secondpilotuorb.onboard_control_sensors_enabled)
		{
			firstpilotfailing = true;

		} else {
			if (redundant_firstpilotuorb.onboard_control_sensors_enabled == redundant_secondpilotuorb.onboard_control_sensors_enabled)
			{
				mypilotfailing = true;
			}

		}

	} else {
		if (_vehicle_status.onboard_control_sensors_enabled != redundant_secondpilotuorb.onboard_control_sensors_enabled)
		{
			secondpilotfailing = true;
		}
	}


	*/


	/* SENSORS CAUSING PROBLEMS

	if (_vehicle_status.onboard_control_sensors_health != redundant_firstpilotuorb.onboard_control_sensors_health)
	{


		if (_vehicle_status.onboard_control_sensors_health == redundant_secondpilotuorb.onboard_control_sensors_health)
		{

			firstpilotfaults[12] = true;
			PX4_WARN("sensor failing");

		} else {
			if (redundant_firstpilotuorb.onboard_control_sensors_health == redundant_secondpilotuorb.onboard_control_sensors_health)
			{

				mypilotfaults[12] = true;
				PX4_WARN("sensor failing");
			}

		}

	} else {
		if (_vehicle_status.onboard_control_sensors_health != redundant_secondpilotuorb.onboard_control_sensors_health)
		{

			secondpilotfaults[12] = true;
			PX4_WARN("sensor failing");
		}
	}


	*/


	/* Just for real flight tests

	// Failure Detector


	if (_vehicle_status.failure_detector_status != redundant_firstpilotuorb.failure_detector_status)
	{
		if (_vehicle_status.failure_detector_status == redundant_secondpilotuorb.failure_detector_status)
		{
			firstpilotfailing = true;

		} else {
			if (redundant_firstpilotuorb.failure_detector_status == redundant_secondpilotuorb.failure_detector_status)
			{
				mypilotfailing = true;
			}

		}

	} else {
		if (_vehicle_status.failure_detector_status != redundant_secondpilotuorb.failure_detector_status)
		{
			secondpilotfailing = true;
		}
	}



	if (_vehicle_status_flags.condition_escs_error!= redundant_firstpilotuorb.condition_escs_error)
	{
		if (_vehicle_status_flags.condition_escs_error)
		{
			if (redundant_firstpilotuorb.condition_escs_error)
			{
				firstpilotfailing = true;
			}

		} else {
			if (redundant_secondpilotuorb.condition_escs_error)
			{
				mypilotfailing = true;
			}

		}

	} else {
		if (!redundant_secondpilotuorb.condition_escs_error)
		{
			if (_vehicle_status_flags.condition_escs_error)
			{
				secondpilotfailing = true;
			}

		}
	}



	if (_vehicle_status_flags.condition_escs_failure!= redundant_firstpilotuorb.condition_escs_failure)
	{
		if (_vehicle_status_flags.condition_escs_failure)
		{
			if (redundant_firstpilotuorb.condition_escs_failure)
			{
				firstpilotfailing = true;
			}

		} else {
			if (redundant_secondpilotuorb.condition_escs_failure)
			{
				mypilotfailing = true;
			}

		}

	} else {
		if (!redundant_secondpilotuorb.condition_escs_failure)
		{
			if (_vehicle_status_flags.condition_escs_failure)
			{
				secondpilotfailing = true;
			}

		}
	}



	if (_vehicle_status_flags.condition_power_input_valid!= redundant_firstpilotuorb.condition_power_input_valid)
	{
		if (_vehicle_status_flags.condition_power_input_valid)
		{
			if (redundant_firstpilotuorb.condition_power_input_valid)
			{
				firstpilotfailing = true;
			}

		} else {
			if (redundant_secondpilotuorb.condition_power_input_valid)
			{
				mypilotfailing = true;
			}

		}

	} else {
		if (!redundant_secondpilotuorb.condition_power_input_valid)
		{
			if (_vehicle_status_flags.condition_power_input_valid)
			{
				secondpilotfailing = true;
			}

		}
	}


	if (_vehicle_status.engine_failure != redundant_firstpilotuorb.engine_failure)
	{
		if (_vehicle_status.engine_failure)
		{
			if (redundant_firstpilotuorb.engine_failure)
			{
				firstpilotfailing = true;
			}

		} else {
			if (redundant_secondpilotuorb.engine_failure)
			{
				mypilotfailing = true;
			}

		}

	} else {
		if (!redundant_secondpilotuorb.engine_failure)
		{
			if (_vehicle_status.engine_failure)
			{
				secondpilotfailing = true;
			}

		}
	}

	*/

	for (int i = 0; i < error_flags_number; i++)
	{
		if (mypilotfaults[i] == true)
		{

			time_last_mypilot_fault_fail[i] = hrt_absolute_time();

			if (hrt_elapsed_time(&time_last_mypilot_fault_pass[i]) > 1_s)
			{
				mypilotfaults_1[i] = true;
			}



		} else {

			time_last_mypilot_fault_pass[i] = hrt_absolute_time();

			if (hrt_elapsed_time(&time_last_mypilot_fault_fail[i]) > 600_s)
			{
				mypilotfaults_1[i] = false;
			}

		}




		if (firstpilotfaults[i] == true)
		{
			time_last_mypilot_fault_fail[i] = hrt_absolute_time();

			if (hrt_elapsed_time(&time_last_firstpilot_fault_pass[i]) > 1_s)
			{
				firstpilotfaults_1[i] = true;
			}



		} else {

			time_last_firstpilot_fault_pass[i] = hrt_absolute_time();

			if (hrt_elapsed_time(&time_last_firstpilot_fault_fail[i]) > 600_s)
			{
				firstpilotfaults_1[i] = false;
			}
		}






		if (secondpilotfaults[i] == true)
		{
			time_last_secondpilot_fault_fail[i] = hrt_absolute_time();

			if (hrt_elapsed_time(&time_last_secondpilot_fault_pass[i]) > 1_s)
			{
				secondpilotfaults_1[i] = true;
			}



		} else {

			time_last_secondpilot_fault_pass[i] = hrt_absolute_time();

			if (hrt_elapsed_time(&time_last_secondpilot_fault_fail[i]) > 600_s)
			{
				secondpilotfaults_1[i] = false;
			}
		}


	}



	for (int i = 0; i < error_flags_number; i++)
	{
		if (mypilotfaults_1[i] == true)
		{
			counter_pilotfailing[0] = counter_pilotfailing[0] + 1;
		}

		if (firstpilotfaults_1[i] == true)
		{
			counter_pilotfailing[1] = counter_pilotfailing[1] + 1;
		}
		if (secondpilotfaults_1[i] ==true)
		{
			counter_pilotfailing[2] = counter_pilotfailing[2] + 1;
		}

	}




	if (counter_pilotfailing[0] > 0)
	{
		mypilotfailing = true;
	}

	if (counter_pilotfailing[1] > 0)
	{
		firstpilotfailing = true;
	}


	if (counter_pilotfailing[2] > 0)
	{
		secondpilotfailing = true;
	}



	for (size_t i = 0; i < 3; i++)
	{


		if (counter_pilotfailing[i] >= counter_failing_worst)
		{
			//increase counter_pilotfailing[i] priority_1
			pilot_priority_1[i] = 3;
			counter_failing_worst = counter_pilotfailing[i];
			if (i > 0)
			{
				if (counter_pilotfailing[i] > counter_pilotfailing[i-1])
				{
					if (pilot_priority_1[i-1] > 1 )
					{
						pilot_priority_1[i-1] = pilot_priority_1[i-1] - 1;
					}

					if (counter_pilotfailing[i-1] > 0)
					{
						counter_failing_worst_2 = counter_pilotfailing[i-1];
					}


				}

				if(i==2)
				{
					if (counter_pilotfailing[i] > counter_pilotfailing[i-2])
					{
						if (pilot_priority_1[i-2] > 1 )
						{
							pilot_priority_1[i-2] = pilot_priority_1[i-2] - 1;
						}

						if (counter_pilotfailing[i-2] > 0)
						{
							counter_failing_worst_2 = counter_pilotfailing[i-1];
						}

					}

				}

			}


		}else{
			if (counter_pilotfailing[i] >= counter_failing_worst_2)
			{
				//increase counter_pilotfailing[i] to middle value
				pilot_priority_1[i] = 2;
				counter_failing_worst_2 = counter_pilotfailing[i];

				if (i > 0)
				{
					if (counter_pilotfailing[i] > counter_pilotfailing[i-1])
					{
						if (pilot_priority_1[i-1] > 1)
						{
							pilot_priority_1[i-1] = pilot_priority_1[i-1] - 1;
						}



					}

					if(i==2)
					{
						if (counter_pilotfailing[i] > counter_pilotfailing[i-2])
						{
							if (pilot_priority_1[i-2] > 1)
							{
								pilot_priority_1[i-2] = pilot_priority_1[i-2] - 1;
							}



						}

					}

				}

			}


		}

	}


	if (communication_fail_firstpilot)
	{
		pilot_priority_1[1] = 3;
	}

	if (communication_fail_secondpilot)
	{
		pilot_priority_1[2] = 3;
	}


}



void *redundancy_manager::sending_trampoline(void * /*unused*/)
{
	instance->send();
	return nullptr;
}


void redundancy_manager::send()
{

	pthread_setname_np(pthread_self(), "autopilots_send");




	while (true)
	{


		//sending mavlink messages to the two autopilots



		//send_heartbeat();

		mavlink_redundant_vehicle_status_t vehicle_msg_1 = {}, vehicle_msg_2 = {};

		mavlink_sys_status_t vehicle_sensors_msg = {};



		//Codify to the right target_systems
		switch (px4_instance)
		{
		case 0:
			redundancy_vehicle_status(&vehicle_msg_1,2);
			redundancy_vehicle_status(&vehicle_msg_2,3);
			break;
		case 1:
			redundancy_vehicle_status(&vehicle_msg_1,1);
			redundancy_vehicle_status(&vehicle_msg_2,3);
			break;
		case 2:
			redundancy_vehicle_status(&vehicle_msg_1,1);
			redundancy_vehicle_status(&vehicle_msg_2,2);
			break;
		default:
			break;
		}


		redundancy_sensors_state(&vehicle_sensors_msg);

		mavlink_message_t message_1 = {}, message_2 = {};
		mavlink_msg_redundant_vehicle_status_encode(_param_mav_sys_id.get(), _param_mav_comp_id.get(), &message_1, &vehicle_msg_1);
		mavlink_msg_redundant_vehicle_status_encode(_param_mav_sys_id.get(), _param_mav_comp_id.get(), &message_2, &vehicle_msg_2);


		//Sensor messages
		/*
		mavlink_msg_sys_status_encode(_param_mav_sys_id.get(), _param_mav_comp_id.get(), &message_3, &vehicle_sensors_msg);
		*/


		PX4_DEBUG("sending redundancy messages t=%ld (%ld)", _vehicle_status.timestamp, vehicle_msg_1.time_usec);


		//To stop sending message to other pilots comment inside send_mavlink_message() function

		send_mavlink_message(message_1,1);
		send_mavlink_message(message_2,2);


		//Send to both pilots the message from sensors
		/*
*		send_mavlink_message(message_3,0);
		*/

		px4_sleep(1);


		pthread_mutex_lock(&lock0);

		while (data_ready)
		{
			pthread_cond_wait(&cond0,&lock0);
		}

		data_ready=1;
		pthread_cond_signal(&cond0);
		pthread_mutex_unlock(&lock0);



	}
}

void redundancy_manager::send_mavlink_message(const mavlink_message_t &aMsg, int system_target)
{
	uint8_t  buf[MAVLINK_MAX_PACKET_LEN];
	uint16_t bufLen = 0;

	bufLen = mavlink_msg_to_send_buffer(buf, &aMsg);

	ssize_t len_1, len_2;



	//Comment here to simulate the communications failure

	if (px4_instance == 0)
	{

		int ret = ::poll(&fds[1], 3, 2000);
		for (size_t i = 1; i < 4; i++)
		{


			if (ret < 0)
			{
				PX4_ERR("Poll error on sending thread\n");
				//study the "continue" option because it breaks the forwarding cycle too from the second pilot socket
				continue;
			}

			if (ret == 0)
			{
				printf("Poll timeout on sending thread\n");
				continue;
			}

			if(fds[i].revents & POLLOUT)
			{



				if (i==1 && (hrt_elapsed_time(&_vehicle_status.takeoff_time) < 30_s))
				{
					if (system_target==2 ||system_target==0)
					{



						len_1 = ::sendto(fds[i].fd, buf, bufLen, 0, (struct sockaddr*)&_myaddr_second, _myaddr_second_len);

						if(len_1 <= 0)
						{
							PX4_WARN("Failed sending mavlink message second socket: %s", strerror(errno));

						}
						PX4_INFO("sent message to the second socket");
					}

				} else {

					if (i==3 && (hrt_elapsed_time(&_vehicle_status.takeoff_time) < 30_s))
					{
						if (system_target==1 ||system_target==0)
						{

							len_2 = ::sendto(fds[i].fd, buf, bufLen, 0, (struct sockaddr*)&_myaddr_first, _myaddr_first_len);

							if (len_2 <= 0)
							{
								PX4_WARN("Failed sending mavlink message first socket: %s", strerror(errno));
							}
							PX4_INFO("sent message to the first socket");
						}

					}

				}
			}


		}
	}





	if (px4_instance == 2)
	{


		int ret = ::poll(&fds[0], 2, 2000);
		for (size_t i = 0; i < 2; i++)
		{


			if (ret < 0)
			{
				PX4_ERR("Error on poll\n");

				continue;
			}

			if (ret == 0)
			{
				printf("timeout\n");
				continue;
			}

			if(fds[i].revents & POLLOUT)
			{

				if (i==0)
				{
					if (system_target==2 ||system_target==0)
					{
						len_1 = ::sendto(fds[i].fd, buf, bufLen, 0, (struct sockaddr*)&_myaddr_second, _myaddr_second_len);

						if (len_1 <= 0)
						{
							PX4_WARN("Failed sending mavlink message on second socket: %s", strerror(errno));


						}

						PX4_INFO("sent message to the second socket");
					}


				} else {

					if (i==1)
					{
						if (system_target==1 ||system_target==0)
						{
							len_2 = ::sendto(fds[i].fd, buf, bufLen, 0, (struct sockaddr*)&_myaddr_first, _myaddr_first_len);

							if (len_2 <= 0)
							{
								PX4_WARN("Failed sending mavlink message first socket: %s", strerror(errno));
							}

							PX4_INFO("sent message to the first socket");
						}
					}

				}
			}


		}


	}



	if (px4_instance == 1)
	{


		int ret = ::poll(&fds[1], 2, 2000);
		for (size_t i = 1; i < 3; i++)
		{


			if (ret < 0)
			{
				PX4_ERR("Error on poll\n");

				continue;
			}

			if (ret == 0)
			{
				PX4_ERR("timeout\n");
				continue;
			}

			if(fds[i].revents & POLLOUT)
			{

				if (i==1)
				{
					if (system_target==2 ||system_target==0)
					{
						len_1 = ::sendto(fds[i].fd, buf, bufLen, 0, (struct sockaddr*)&_myaddr_second, _myaddr_second_len);

						if (len_1 <= 0)
						{
							PX4_WARN("Failed sending mavlink message on second socket: %s", strerror(errno));
						}

						PX4_INFO("sent message to the second socket");

					}

				} else {

					if (i==2)
					{
						if (system_target==1 ||system_target==0)
						{

							len_2 = ::sendto(fds[i].fd, buf, bufLen, 0, (struct sockaddr*)&_myaddr_first, _myaddr_first_len);

							if (len_2 <= 0)
							{
								PX4_WARN("Failed sending mavlink message first socket: %s", strerror(errno));
							}
							PX4_INFO("sent message to the first socket");
						}
					}

				}
			}


		}


	}



}


void redundancy_manager::send_heartbeat()
{

	mavlink_heartbeat_t hb_2 = {};

	mavlink_message_t message_2 = {};


	parameters_update(true);

	hb_2.autopilot = 12;



	hb_2.base_mode |= (_vehicle_status.arming_state == vehicle_status_s::ARMING_STATE_ARMED) ? 128 : 0;



	mavlink_msg_heartbeat_encode(_param_mav_sys_id.get(), _param_mav_comp_id.get(), &message_2, &hb_2);


	send_mavlink_message(message_2,0);



}

void redundancy_manager::redundancy_vehicle_status(mavlink_redundant_vehicle_status_t *msg, int target_system)
{
	memset(msg, 0, sizeof(mavlink_redundant_vehicle_status_t));

	uint16_t storage_1, storage_2;
	storage_1 = 0;
	storage_2 = 0;

	msg->time_usec = hrt_absolute_time();


	//Vehicle_status_flags
	if (_vehicle_status_flags.condition_system_sensors_initialized == true)
	{
		storage_1 |= 1 << 0;
	}

	if (_vehicle_status_flags.condition_angular_velocity_valid == true)
	{
		storage_1 |= 1 << 1;
	}
	if (_vehicle_status_flags.condition_attitude_valid == true)
	{
		storage_1 |= 1 << 2;
	}
	if (_vehicle_status_flags.condition_local_altitude_valid == true)
	{
		storage_1 |= 1 << 3;
	}
	if (_vehicle_status_flags.condition_local_position_valid == true)
	{
		storage_1 |= 1 << 4;
	}
	if (_vehicle_status_flags.condition_local_velocity_valid == true)
	{
		storage_1 |= 1 << 5;
	}
	if (_vehicle_status_flags.condition_home_position_valid == true)
	{
		storage_1 |= 1 << 6;
	}
	if (_vehicle_status_flags.condition_power_input_valid == true)
	{
		storage_1 |= 1 << 7;
	}
	if (_vehicle_status_flags.condition_battery_healthy == true)
	{
		storage_1 |= 1 << 8;
	}
	if (_vehicle_status_flags.condition_escs_error == true)
	{
		storage_1 |= 1 << 9;
	}
	if (_vehicle_status_flags.condition_escs_failure == true)
	{
		storage_1 |= 1 << 10;
	}
	if (_vehicle_status_flags.condition_global_position_valid == true)
	{
		storage_1 |= 1 << 11;
	}


	//Vehicle_status_main
	if (_vehicle_status.failsafe == true)
	{
		storage_2 |= 1 << 0;
	}
	if (_vehicle_status.data_link_lost == true)
	{
		storage_2 |= 1 << 1;
	}
	if (_vehicle_status.engine_failure == true)
	{
		storage_2 |= 1 << 2;
	}
	if (_vehicle_status.mission_failure == true)
	{
		storage_2 |= 1 << 3;
	}




	msg->redundant_vehicle_status_flags = storage_1;
	msg->redundant_vehicle_status_main = storage_2;
	msg->redundant_vehicle_status_failuredetector = _vehicle_status.failure_detector_status;
	msg->data_link_lost_counter = _vehicle_status.data_link_lost_counter;
	msg->failsafe_timestamp = _vehicle_status.failsafe_timestamp;
	msg->takeoff_time = _vehicle_status.takeoff_time;






	msg->target_system = target_system;
	msg->target_component = 1;




}


void redundancy_manager::redundancy_sensors_state(mavlink_sys_status_t *msg)
{
	memset(msg, 0, sizeof(mavlink_sys_status_t));

	msg->onboard_control_sensors_present = _vehicle_status.onboard_control_sensors_present;
	msg->onboard_control_sensors_enabled = _vehicle_status.onboard_control_sensors_enabled;
	msg->onboard_control_sensors_health = _vehicle_status.onboard_control_sensors_health;

}



