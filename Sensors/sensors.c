#include <stdio.h>          
#include <stdlib.h>        
#include <sys/types.h>    
#include <netinet/in.h>   
#include <sys/socket.h>   
#include <arpa/inet.h>    
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <mosquitto.h>
#include <pthread.h>

#define PORT_NUM    10000
#define GROUP_ADDR  "225.1.1.1"
///////////////////////////////

pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

char ip_adress[200];
char this_ip_address[] = "192.168.1.109";

struct mosquitto* mosq;

unsigned int         server_s;                // Server socket descriptor
unsigned int         multi_server_sock;       // Multicast socket descriptor
struct sockaddr_in   addr_dest;               // Multicast group address
struct ip_mreq       mreq;                    // Multicast group descriptor
unsigned char        TTL;                     // TTL for multicast packets
struct in_addr       recv_ip_addr;            // Receive IP address
unsigned int         addr_len;                // Internet address length
unsigned char        buffer[256];             // Datagram buffer
int                  retcode;                 // Return code

void* sensor_tmp(){
	while(1){
		pthread_mutex_lock(&mutex_lock);

		sprintf(buffer, "Temperature sensor");
		sendto(multi_server_sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr_dest, addr_len);
		int temp_move = rand() % 3 - 1;
		char msg[2];

		sprintf(msg, "%d", temp_move);
		mosquitto_publish(mosq, NULL, "field/sensor/temperature", 2, msg, 1, false);
		printf("%s", msg);
		fflush(stdout);
		pthread_mutex_unlock(&mutex_lock);
		sleep(2);
	}
}

void* sensor_p(){
	while(1){
		pthread_mutex_lock(&mutex_lock);

		sprintf(buffer, "Pressure sensor");
		sendto(multi_server_sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr_dest, addr_len);
		int press_move = rand() % 3 - 1;
		char msg[2];

		sprintf(msg, "%d", press_move);
		mosquitto_publish(mosq, NULL, "field/sensor/pressure", 2, msg, 1, false);
		printf("%s", msg);
		fflush(stdout);
		pthread_mutex_unlock(&mutex_lock);
		sleep(2);
	}
}

void* sensor_m(){
	while(1){
		pthread_mutex_lock(&mutex_lock);

		sprintf(buffer, "Moisture sensor");
		sendto(multi_server_sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr_dest, addr_len);
		int moist_move = rand() % 3 - 1;
		char msg[2];

		sprintf(msg, "%d", moist_move);
		mosquitto_publish(mosq, NULL, "field/sensor/moisture", 2, msg, 1, false);
		printf("%s", msg);
		fflush(stdout);
		pthread_mutex_unlock(&mutex_lock);
		sleep(2);
	}
}

int main(){
	gethostname(ip_adress, sizeof(ip_adress));
	int rc;

	mosquitto_lib_init();
	mosq = mosquitto_new("pub-project", true, NULL);
	rc = mosquitto_connect(mosq, "localhost", 1883, 60);

	if(rc != 0){
		printf("Client couldn't connect to broker! Error code: %d\n", rc);
		mosquitto_destroy(mosq);
	}

	pthread_mutex_init(&mutex_lock, NULL);

	multi_server_sock = socket(AF_INET, SOCK_DGRAM, 0);
	addr_dest.sin_family = AF_INET;
	addr_dest.sin_addr.s_addr = inet_addr(GROUP_ADDR);
	addr_dest.sin_port = htons(PORT_NUM);
	TTL = 1;
	retcode = setsockopt(multi_server_sock, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&TTL, sizeof(TTL));
	addr_len = sizeof(addr_dest);
	printf("*** Sending multicast datagrams to '%s' (port = %d) \n", GROUP_ADDR, PORT_NUM);
	mosquitto_loop_start(mosq);

	pthread_t t, p, m;

	pthread_create(&(t), NULL, &sensor_tmp, NULL);
	pthread_create(&(p), NULL, &sensor_p, NULL);
	pthread_create(&(m), NULL, &sensor_m, NULL);

	pthread_detach(t);
	pthread_detach(p);
	pthread_detach(m);
	sleep(100);

	mosquitto_loop_stop(mosq, false);

	close(multi_server_sock);
	return 0;
}
