#include <stdio.h>          
#include <stdlib.h>        
#include <sys/types.h>    
#include <netinet/in.h>   
#include <sys/socket.h>   
#include <arpa/inet.h>    
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <mosquitto.h>

#define PORT_NUM    10000             
#define GROUP_ADDR  "225.1.1.1"
#define VALUE           35
#define UPPER_T_LIMIT   25
#define LOWER_T_LIMIT   10
#define UPPER_P_LIMIT   75
#define LOWER_P_LIMIT   25
#define UPPER_M_LIMIT   40
#define LOWER_M_LIMIT   10
#define MOVE            2
//*******************************
// For setting actuators to on or off state
int ta_on = 0;
int pa_on = 0;
int ma_on = 0;

int check_temp = 0;
int check_p = 0;
int check_m = 0;

int t_count = 0;
int p_count = 0;
int m_count = 0;

int temperature = 20;
int pressure = 40;
int moisture = 25;
/*
int temperature = rand() % VALUE;
int pressure = rand() % VALUE;
int moisture = rand() % VALUE;
*/

char temp_sensor_topic[256];
char p_sensor_topic[256];
char m_sensor_topic[256];

char temp_actuator_topic[256];
char p_actuator_topic[256];
char m_actuator_topic[256];

char hostname_actuator[200] = "";
char hostname_sensor[200] = "";

pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

struct mosquittp* mosq;
int value_move;

char this_ip_addres[] = "192.168.1.109";

void on_connect_temp_sensor(struct mosquitto *mosq, void *obj, int rc){
	if (rc == 0) {
		mosquitto_subscribe(mosq, NULL, "field/sensor/temperature", 0);
	} else {
		mosquitto_disconnect(mosq);
	}
}

void on_connect_p_sensor(struct mosquitto *mosq, void *obj, int rc){
	if (rc == 0) {
		mosquitto_subscribe(mosq, NULL, "field/sensor/pressure", 0);
	} else {
		mosquitto_disconnect(mosq);
	}
}

void on_connect_m_sensor(struct mosquitto *mosq, void *obj, int rc){
	if (rc == 0) {
		mosquitto_subscribe(mosq, NULL, "field/sensor/moisture", 0);
	} else {
		mosquitto_disconnect(mosq);
	}
}

void on_connect_temp_actuator(struct mosquitto *mosq, void *obj, int rc){
	if (rc == 0) {
		mosquitto_subscribe(mosq, NULL, "field/actuator/temperature", 0);
	} else {
		mosquitto_disconnect(mosq);
	}
}

void on_connect_p_actuator(struct mosquitto *mosq, void *obj, int rc){
	if (rc == 0) {
		mosquitto_subscribe(mosq, NULL, "field/actuator/pressure", 0);
	} else {
		mosquitto_disconnect(mosq);
	}
}

void on_connect_m_actuator(struct mosquitto *mosq, void *obj, int rc){
	if (rc == 0) {
		mosquitto_subscribe(mosq, NULL, "field/actuator/moisture", 0);
	} else {
		mosquitto_disconnect(mosq);
	}
}

void on_message_temp_sensor(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg){
    value_move = atoi(msg->payload);
    strcpy(temp_sensor_topic, atoi(msg->topic));
    pthread_mutex_lock(&mutex_lock);
    
    if(temperature < LOWER_T_LIMIT)
        check_temp = 1;
    else if(temperature > UPPER_T_LIMIT)
        check_temp = 2;
    else
        check_temp = 0;
    
    if(check_temp == 0){
        mosquitto_publish(mosq, NULL, "field/actuator/temperature", 2, "0", 0, false);
        int move = rand() % MOVE - 1;
        if(move == 0)
            temperature += value_move;
        else
            temperature -= value_move;
		printf("T -> %d M -> %d\t", temperature, value_move);
    }
    else if(check_temp == 1){
        mosquitto_publish(mosq, NULL, "field/actuator/temperature", 2, "1", 0, false);
        temperature += value_move;
		printf("T -> %d M -> %d\t", temperature, value_move);
    }
    else{
        mosquitto_publish(mosq, NULL, "field/actuator/temperature", 2, "2", 0, false);
        temperature -= value_move;
		printf("T -> %d M -> %d\t", temperature, value_move);
    }
    pthread_mutex_unlock(&mutex_lock);
}

void on_message_p_sensor(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg){
    value_move = atoi(msg->payload);
    strcpy(p_sensor_topic, atoi(msg->topic));
    pthread_mutex_lock(&mutex_lock);
    
    if(pressure < LOWER_P_LIMIT)
        check_p = 1;
    else if(pressure > UPPER_P_LIMIT)
        check_p = 2;
    else
        check_p = 0;
    
    if(check_p == 0){
        mosquitto_publish(mosq, NULL, "field/actuator/pressure", 2, "0", 0, false);
        int move = rand() % MOVE - 1;
        if(move == 0)
            pressure += value_move;
        else
            pressure -= value_move;
    }
    else if(check_p == 1){
        mosquitto_publish(mosq, NULL, "field/actuator/pressure", 2, "1", 0, false);
        pressure += value_move;
    }
    else{
        mosquitto_publish(mosq, NULL, "field/actuator/pressure", 2, "2", 0, false);
        pressure -= value_move;
    }
    pthread_mutex_unlock(&mutex_lock);
}

void on_message_m_sensor(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg){
    value_move = atoi(msg->payload);
    strcpy(m_sensor_topic, atoi(msg->topic));
    pthread_mutex_lock(&mutex_lock);
    
    if(moisture < LOWER_M_LIMIT)
        check_m = 1;
    else if(moisture > UPPER_M_LIMIT)
        check_m = 2;
    else
        check_m = 0;
    
    if(check_m == 0){
        mosquitto_publish(mosq, NULL, "field/actuator/moisture", 2, "0", 0, false);
        int move = rand() % MOVE - 1;
        if(move == 0)
            moisture += value_move;
        else
            moisture -= value_move;
    }
    else if(check_m == 1)
        mosquitto_publish(mosq, NULL, "field/actuator/moisture", 2, "1", 0, false);
    else
        mosquitto_publish(mosq, NULL, "field/actuator/moisture", 2, "2", 0, false);

    pthread_mutex_unlock(&mutex_lock);
}

void on_message_temp_actuator(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg){	
	pthread_mutex_lock(&mutex_lock);
	int msg_receive = atoi(msg->payload);
    strcpy(temp_actuator_topic, atoi(msg->topic));
	temperature += msg_receive;
	printf("Message  received -> %d", msg_receive);
	pthread_mutex_unlock(&mutex_lock);
}

void on_message_p_actuator(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg){	
	pthread_mutex_lock(&mutex_lock);
	int msg_receive = atoi(msg->payload);
    strcpy(p_actuator_topic, atoi(msg->topic));
	pressure += msg_receive;
	pthread_mutex_unlock(&mutex_lock);
}

void on_message4_m_actuator(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg){	
	pthread_mutex_lock(&mutex_lock);
	int msg_receive = atoi(msg->payload);
    strcpy(m_actuator_topic, atoi(msg->topic));
	moisture += msg_receive;
	pthread_mutex_unlock(&mutex_lock);
}

void* sub_temp_sensor(){
    struct mosquitto *mosq;
	int rc;

	mosquitto_lib_init();
	mosq = mosquitto_new(NULL, true, NULL);
	if (mosq == NULL)
		printf("Failed to create client instance.\n");
	
	mosquitto_connect_callback_set(mosq, on_connect_temp_sensor);
	mosquitto_message_callback_set(mosq, on_message_temp_sensor);
    sleep(2);

	rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	while (rc != MOSQ_ERR_SUCCESS) {
	    pthread_mutex_lock(&mutex_lock);
	    rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	    pthread_mutex_unlock(&mutex_lock);
		sleep(1);
	}

	mosquitto_loop_forever(mosq, -1, 1);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}

void* sub_p_sensor(){
    struct mosquitto *mosq;
	int rc;
    
	mosquitto_lib_init();
	mosq = mosquitto_new(NULL, true, NULL);
	if (mosq == NULL)
		printf("Failed to create client instance.\n");
	
	mosquitto_connect_callback_set(mosq, on_connect_p_sensor);
	mosquitto_message_callback_set(mosq, on_message_p_sensor);
    sleep(2);

	rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	while (rc != MOSQ_ERR_SUCCESS) {
	    pthread_mutex_lock(&mutex_lock);
	    rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	    pthread_mutex_unlock(&mutex_lock);
		sleep(1);
	}

	mosquitto_loop_forever(mosq, -1, 1);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}

void* sub_m_sensor(){
    struct mosquitto *mosq;
	int rc;
    
	mosquitto_lib_init();
	mosq = mosquitto_new(NULL, true, NULL);
	if (mosq == NULL)
		printf("Failed to create client instance.\n");
	
	mosquitto_connect_callback_set(mosq, on_connect_temp_sensor);
	mosquitto_message_callback_set(mosq, on_message_temp_sensor);
    sleep(2);

	rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	while (rc != MOSQ_ERR_SUCCESS) {
	    pthread_mutex_lock(&mutex_lock);
	    rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	    pthread_mutex_unlock(&mutex_lock);
		sleep(1);
	}

	mosquitto_loop_forever(mosq, -1, 1);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}

void* sub_temp_actuator(){
    struct mosquitto *mosq;
	int rc;

	mosquitto_lib_init();
	mosq = mosquitto_new(NULL, true, NULL);
	if (mosq == NULL)
		printf("Failed to create client instance.\n");
	
	mosquitto_connect_callback_set(mosq, on_connect_temp_actuator);
	mosquitto_message_callback_set(mosq, on_message_temp_actuator);
    sleep(2);

	rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	while (rc != MOSQ_ERR_SUCCESS) {
	    pthread_mutex_lock(&mutex_lock);
	    rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	    pthread_mutex_unlock(&mutex_lock);
		sleep(1);
	}

	mosquitto_loop_forever(mosq, -1, 1);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}

void* sub_p_actuator(){
    struct mosquitto *mosq;
	int rc;
    
	mosquitto_lib_init();
	mosq = mosquitto_new(NULL, true, NULL);
	if (mosq == NULL)
		printf("Failed to create client instance.\n");
	
	mosquitto_connect_callback_set(mosq, on_connect_p_actuator);
	mosquitto_message_callback_set(mosq, on_message_p_actuator);
    sleep(2);

	rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	while (rc != MOSQ_ERR_SUCCESS) {
	    pthread_mutex_lock(&mutex_lock);
	    rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	    pthread_mutex_unlock(&mutex_lock);
		sleep(1);
	}

	mosquitto_loop_forever(mosq, -1, 1);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}

void* sub_m_actuator(){
    struct mosquitto *mosq;
	int rc;
    
	mosquitto_lib_init();
	mosq = mosquitto_new(NULL, true, NULL);
	if (mosq == NULL)
		printf("Failed to create client instance.\n");
	
	mosquitto_connect_callback_set(mosq, on_connect_temp_actuator);
	mosquitto_message_callback_set(mosq, on_message_temp_actuator);
    sleep(2);

	rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	while (rc != MOSQ_ERR_SUCCESS) {
	    pthread_mutex_lock(&mutex_lock);
	    rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	    pthread_mutex_unlock(&mutex_lock);
		sleep(1);
	}

	mosquitto_loop_forever(mosq, -1, 1);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}

void print_all(){
    printf("***********************************************\n");
    printf("%s ->\t\t%d\n", temp_sensor_topic, temperature);
    printf("%s ->\t\t%d\n", p_sensor_topic, pressure);
    printf("%s ->\t\t%d\n", m_sensor_topic, moisture);
    printf("***********************************************\n");
    printf("%s ->\t\tActive for %d[s]\n", temp_actuator_topic, t_count);
    printf("%s ->\t\tActive for %d[s]\n", p_actuator_topic, p_count);
    printf("%s ->\t\tActive for %d[s]\n", m_actuator_topic, m_count);
}

int main(){
    unsigned int         multi_server_sock; // Multicast socket descriptor
	struct ip_mreq       mreq;              // Multicast group structure
	struct sockaddr_in   client_addr;       // Client Internet address
	unsigned int         addr_len;          // Internet address length
	unsigned char        buffer[256];       // Datagram buffer
	int                  retcode;           // Return code

	pthread_mutex_init(&mutex_lock, NULL);

	// Create a multicast socket and fill-in multicast address information
	multi_server_sock = socket(AF_INET, SOCK_DGRAM, 0);
	mreq.imr_multiaddr.s_addr = inet_addr(GROUP_ADDR);
	mreq.imr_interface.s_addr = INADDR_ANY;

    pthread_t t_temp_sensor;
    pthread_t t_p_sensor;
    pthread_t t_m_sensor;
    pthread_t t_temp_actuator;
    pthread_t t_p_actuator;
    pthread_t t_m_actuator;

    pthread_create(&(t_temp_sensor),NULL,&sub_temp_sensor,NULL);
    pthread_create(&(t_p_sensor),NULL,&sub_p_sensor,NULL);
    pthread_create(&(t_m_sensor),NULL,&sub_m_sensor,NULL);
    pthread_create(&(t_temp_actuator),NULL,&sub_temp_actuator,NULL);
    pthread_create(&(t_p_actuator),NULL,&sub_p_actuator,NULL);
    pthread_create(&(t_m_actuator),NULL,&sub_p_actuator,NULL);

    client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = INADDR_ANY;
	client_addr.sin_port = htons(PORT_NUM);

	retcode = bind(multi_server_sock, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
	if (retcode < 0){
		printf("*** ERROR - bind() failed with retcode = %d \n", retcode);
		return -1;
	}

	// Have the multicast socket join the multicast group
	retcode = setsockopt(multi_server_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
	if (retcode < 0){
		printf("*** ERROR - setsockopt() failed with retcode = %d \n", retcode);
		return -1;
	}

	// Set addr_len
	addr_len = sizeof(client_addr);
	while (1){
		// Receive a datagram from the multicast server
		if ((retcode = recvfrom(multi_server_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
			printf("*** ERROR - recvfrom() failed \n");
			exit(1);
		}
		//check if off
		if(!strcmp(buffer, "A/T -> 0"))
            t_count = 0;
        else
            t_count++;

		if(!strcmp(buffer, "A/P -> 0"))
            p_count = 0;
        else
            p_count++;

		if(!strcmp(buffer, "A/M -> 0"))
            m_count = 0;
        else
            m_count++;
        
        system("clear");
        print_all();
		fflush(stdout);
    }

    close(multi_server_sock);
    return 0;
}
