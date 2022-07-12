#include <stdio.h>          
#include <stdlib.h>        
#include <sys/types.h>    
#include <netinet/in.h>   
#include <sys/socket.h>   
#include <arpa/inet.h>    
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <mosquitto.h>

#define PORT_NUM 10000
#define GROUP_ADDR "225.1.1.1"
//////////////////////////////

pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

int ta_on = 0;
int pa_on = 0;
int ma_on = 0;

char ip_adress[200];
char this_ip_addres[] = "192.168.1.109";

struct mosquitto *mosq1;
int rc;

void* actuator_temp(){
    unsigned int         server_s;                // Server socket descriptor
	unsigned int         multi_server_sock;       // Multicast socket descriptor
	struct sockaddr_in   addr_dest;               // Multicast group address
	struct ip_mreq       mreq;                    // Multicast group descriptor
	unsigned char        TTL;                     // TTL for multicast packets
	struct in_addr       recv_ip_addr;            // Receive IP address
	unsigned int         addr_len;                // Internet address length
	unsigned char        buffer[256];             // Datagram buffer
	int                  retcode;                 // Return code

	multi_server_sock = socket(AF_INET, SOCK_DGRAM, 0);
	addr_dest.sin_family = AF_INET;
	addr_dest.sin_addr.s_addr = inet_addr(GROUP_ADDR);
	addr_dest.sin_port = htons(PORT_NUM);
	TTL = 1;

	retcode = setsockopt(multi_server_sock, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&TTL, sizeof(TTL));
	if (retcode < 0)
		printf("*** ERROR - setsockopt() failed with retcode = %d \n", retcode);
	
	addr_len = sizeof(addr_dest);
	printf("*** Sending multicast datagrams to '%s' (port = %d) \n", GROUP_ADDR, PORT_NUM);
	
	while (1)
	{
		sprintf(buffer, "A/T -> %d", ta_on);
		sendto(multi_server_sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr_dest, addr_len);
		sleep(1);
	}
	close(multi_server_sock);
}

void* actuator_p(){
    unsigned int         server_s;                // Server socket descriptor
	unsigned int         multi_server_sock;       // Multicast socket descriptor
	struct sockaddr_in   addr_dest;               // Multicast group address
	struct ip_mreq       mreq;                    // Multicast group descriptor
	unsigned char        TTL;                     // TTL for multicast packets
	struct in_addr       recv_ip_addr;            // Receive IP address
	unsigned int         addr_len;                // Internet address length
	unsigned char        buffer[256];             // Datagram buffer
	int                  retcode;                 // Return code

	multi_server_sock = socket(AF_INET, SOCK_DGRAM, 0);
	addr_dest.sin_family = AF_INET;
	addr_dest.sin_addr.s_addr = inet_addr(GROUP_ADDR);
	addr_dest.sin_port = htons(PORT_NUM);
	TTL = 1;

	retcode = setsockopt(multi_server_sock, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&TTL, sizeof(TTL));
	if (retcode < 0)
		printf("*** ERROR - setsockopt() failed with retcode = %d \n", retcode);
	
	addr_len = sizeof(addr_dest);
	printf("*** Sending multicast datagrams to '%s' (port = %d) \n", GROUP_ADDR, PORT_NUM);
	
	while (1)
	{
		sprintf(buffer, "A/P -> %d", pa_on);
		sendto(multi_server_sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr_dest, addr_len);
		sleep(1);
	}
	close(multi_server_sock);
}

void* actuator_m(){
    unsigned int         server_s;                // Server socket descriptor
	unsigned int         multi_server_sock;       // Multicast socket descriptor
	struct sockaddr_in   addr_dest;               // Multicast group address
	struct ip_mreq       mreq;                    // Multicast group descriptor
	unsigned char        TTL;                     // TTL for multicast packets
	struct in_addr       recv_ip_addr;            // Receive IP address
	unsigned int         addr_len;                // Internet address length
	unsigned char        buffer[256];             // Datagram buffer
	int                  retcode;                 // Return code

	multi_server_sock = socket(AF_INET, SOCK_DGRAM, 0);
	addr_dest.sin_family = AF_INET;
	addr_dest.sin_addr.s_addr = inet_addr(GROUP_ADDR);
	addr_dest.sin_port = htons(PORT_NUM);
	TTL = 1;

	retcode = setsockopt(multi_server_sock, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&TTL, sizeof(TTL));
	if (retcode < 0)
		printf("*** ERROR - setsockopt() failed with retcode = %d \n", retcode);

	addr_len = sizeof(addr_dest);
	printf("*** Sending multicast datagrams to '%s' (port = %d) \n", GROUP_ADDR, PORT_NUM);
	
	while (1)
	{
		sprintf(buffer, "A/M -> %d", ma_on);
		sendto(multi_server_sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr_dest, addr_len);
		sleep(1);
	}
	close(multi_server_sock);
}
// Temperature subscriber
void on_message_temp(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg){
    int message = atoi(msg->payload);
    pthread_mutex_lock(&mutex_lock);

    if(message == 0)
        ta_on = 0;
    else if(message == 1)
        ta_on = 1;
    else if(message == 2)
        ta_on = 2;
    
    pthread_mutex_unlock(&mutex_lock);
}

void on_connect_temp(struct mosquitto *mosq, void *obj, int rc){
    if(rc == 0)
        mosquitto_subscribe(mosq, NULL, "field/actuator/temperature", 0);
    else
        mosquitto_disconnect(mosq);
}

void* sub_temp(){
    struct mosquitto *mosq;
	int rc;

	mosquitto_lib_init();
	mosq = mosquitto_new(NULL, true, NULL);
	if (mosq == NULL)
		printf("Failed to create client instance.\n");

	mosquitto_connect_callback_set(mosq, on_connect_temp);
	mosquitto_message_callback_set(mosq, on_message_temp);

	rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	if (rc != MOSQ_ERR_SUCCESS)
		printf("Connect failed: %s\n", mosquitto_strerror(rc));
    
	mosquitto_loop_forever(mosq, -1, 1);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}
// Pressure subscriber
void on_message_p(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg){
    int message = atoi(msg->payload);
    pthread_mutex_lock(&mutex_lock);

    if(message == 0)
        ta_on = 0;
    else if(message == 1)
        ta_on = 1;
    else if(message == 2)
        ta_on = 2;
    
    pthread_mutex_unlock(&mutex_lock);
}

void on_connect_p(struct mosquitto *mosq, void *obj, int rc){
    if(rc == 0)
        mosquitto_subscribe(mosq, NULL, "field/actuator/pressure", 0);
    else
        mosquitto_disconnect(mosq);
}

void* sub_p(){
    struct mosquitto *mosq;
	int rc;

	mosquitto_lib_init();
	mosq = mosquitto_new(NULL, true, NULL);
	if (mosq == NULL)
		printf("Failed to create client instance.\n");

	mosquitto_connect_callback_set(mosq, on_connect_p);
	mosquitto_message_callback_set(mosq, on_message_p);

	rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	if (rc != MOSQ_ERR_SUCCESS)
		printf("Connect failed: %s\n", mosquitto_strerror(rc));
    
	mosquitto_loop_forever(mosq, -1, 1);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}
// Moisture subscriber
void on_message_m(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg){
    int message = atoi(msg->payload);
    pthread_mutex_lock(&mutex_lock);

    if(message == 0)
        ta_on = 0;
    else if(message == 1)
        ta_on = 1;
    else if(message == 2)
        ta_on = 2;
    
    pthread_mutex_unlock(&mutex_lock);
}

void on_connect_m(struct mosquitto *mosq, void *obj, int rc){
    if(rc == 0)
        mosquitto_subscribe(mosq, NULL, "field/actuator/moisture", 0);
    else
        mosquitto_disconnect(mosq);
}

void* sub_m(){
    struct mosquitto *mosq;
	int rc;

	mosquitto_lib_init();
	mosq = mosquitto_new(NULL, true, NULL);
	if (mosq == NULL)
		printf("Failed to create client instance.\n");

	mosquitto_connect_callback_set(mosq, on_connect_m);
	mosquitto_message_callback_set(mosq, on_message_m);

	rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	if (rc != MOSQ_ERR_SUCCESS)
		printf("Connect failed: %s\n", mosquitto_strerror(rc));
    
	mosquitto_loop_forever(mosq, -1, 1);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}

void* pub_temp(){
    while(1){
        sleep(2);
		pthread_mutex_lock(&mutex_lock);
		if(ta_on == 1)
            mosquitto_publish(mosq1, NULL, "field/actuator/temperature", 2, "1", 1, false);
		if(ta_on == 2)
			mosquitto_publish(mosq1, NULL, "field/actuator/temperature", 2, "-1", 1, false);

		pthread_mutex_unlock(&mutex_lock);
    }
}

void* pub_p(){
    while(1){
        sleep(2);
		pthread_mutex_lock(&mutex_lock);
		if(pa_on == 1)
            mosquitto_publish(mosq1, NULL, "field/actuator/pressure", 2, "1", 1, false);
		if(pa_on == 2)
			mosquitto_publish(mosq1, NULL, "field/actuator/pressure", 2, "-1", 1, false);

		pthread_mutex_unlock(&mutex_lock);
    }
}

void* pub_m(){
    while(1){
        sleep(2);
		pthread_mutex_lock(&mutex_lock);
		if(ma_on == 1)
            mosquitto_publish(mosq1, NULL, "field/actuator/moisture", 2, "1", 1, false);
		if(ma_on == 2)
			mosquitto_publish(mosq1, NULL, "field/actuator/moisture", 2, "-1", 1, false);

		pthread_mutex_unlock(&mutex_lock);
    }
}

int main(){
    pthread_mutex_init(&mutex_lock, NULL);
    // actuator - temp/p/m - (s) - subscriber (p) - publisher
    pthread_t at, ts, tp;
    pthread_t ap, ps, pp;
    pthread_t am, ms, mp;

    mosquitto_lib_init();
    mosq1 = mosquitto_new("pub-project", true, NULL);
	rc = mosquitto_connect(mosq1, "localhost", 1883, 60);
    if (rc != 0) {
        printf("Client could not connect to broker! Error Code: %d\n", rc);
        mosquitto_destroy(mosq1);
    }
    gethostname(ip_adress,sizeof(ip_adress));

    mosquitto_loop_start(mosq1);
    // Actuator
    pthread_create(&(at), NULL, &actuator_temp, NULL);
    pthread_create(&(ap), NULL, &actuator_p, NULL);
    pthread_create(&(am), NULL, &actuator_m, NULL);
    // Subscriber
    pthread_create(&(ts), NULL, &sub_temp, NULL);
    pthread_create(&(ps), NULL, &sub_p, NULL);
    pthread_create(&(ms), NULL, &sub_m, NULL);
    // Publisher
    pthread_create(&(tp), NULL, &pub_temp, NULL);
    pthread_create(&(pp), NULL, &pub_p, NULL);
    pthread_create(&(mp), NULL, &pub_m, NULL);

    pthread_detach(at);
    pthread_detach(ap);
    pthread_detach(am);
    pthread_detach(ts);
    pthread_detach(ps);
    pthread_detach(ms);
    pthread_detach(tp);
    pthread_detach(pp);
    pthread_detach(mp);

    sleep(1000);
    mosquitto_loop_stop(mosq1, false);

    return 0;
}
