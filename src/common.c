/**
 * @file common.c
 * @brief Common functions used by both the server and the client
 * 
 * This file contains common functions, structures and macros used by both the server and the client
 * 
 * @date 27/03/2024
 * @authors Francisco Pedrosa
 * @path src/common.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include "common.h"

int validate_port(int server_port) {
    if (server_port < 1024 || server_port > 65535) {
        fprintf(stderr, "Invalid port number. Should be between 1024 and 65535.\n");
        return STATUS_ERROR;
    }
    return STATUS_SUCCESS;
}

void disable_buffering(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}

int close_socket(int server_socket){
    if (close(server_socket) == -1) {
        fprintf(stderr, "Error closing socket\n");
        return STATUS_ERROR;
    }
    return STATUS_SUCCESS;
}

float get_float_value(proto_sensor_data_t *data){
    return *((float *)&(data->data));
}

int generate_random_float(float *p_float_out) {
    if (p_float_out == NULL) {
        return STATUS_ERROR;
    }

    *p_float_out = ((float)rand() / RAND_MAX) * 100.0;

    return STATUS_SUCCESS; 
}

int get_current_time(char **buffer) {
    time_t rawtime;
    struct tm *timeinfo;

    *buffer = (char *)calloc(TIME_BUFFER_SIZE, sizeof(char));
    if (*buffer == NULL) {
        fprintf(stderr, "Error allocating memory for time buffer\n");
        return STATUS_ERROR;
    }

    // Get current time
    time(&rawtime);
    if (rawtime == -1) {
        fprintf(stderr, "Error getting current time\n");
        free(*buffer);
        return STATUS_ERROR;
    }
    timeinfo = localtime(&rawtime);
    if (timeinfo == NULL) {
        fprintf(stderr, "Error getting local time\n");
        free(*buffer);
        return STATUS_ERROR;
    }

    // Format time
    size_t bytes = strftime(*buffer, TIME_BUFFER_SIZE, "%d-%m-%Y %H:%M:%S", timeinfo);
    if (bytes == 0) {
        fprintf(stderr, "Error formatting time\n");
        free(*buffer);
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

int join_threads(pthread_t *tids){
    //printf("Joining threads\n");

    for (uint32_t i = 0; i < NUMBER_OF_SENSORS; i++){
        if (pthread_join(tids[i], NULL) != 0){
            fprintf(stderr, "Error joining thread %d\n", i);
            return STATUS_ERROR;
        }
    }

    //printf("Threads joined\n");
    return STATUS_SUCCESS;
}

int open_logs_file(logs_file_t *logs_file, const char *filename){
    printf("Opening logs file...\n");
    logs_file->file = fopen(filename, "a");
    if (logs_file->file == NULL) {
        fprintf(stderr, "Error opening logs file \"%s\"\n", filename);
        return STATUS_ERROR;
    }

    if (pthread_mutex_init(&logs_file->mutex, NULL) != 0) {
        fprintf(stderr, "Error initializing mutex for logs file\n");
        fclose(logs_file->file);
        return STATUS_ERROR;
    }

    printf("Logs file opened\n");

    return STATUS_SUCCESS;
}

int close_logs_file(logs_file_t *logs_file){
    if (fclose(logs_file->file) == EOF) {
        fprintf(stderr, "Error closing logs file\n");
        return STATUS_ERROR;
    }

    if (pthread_mutex_destroy(&logs_file->mutex) != 0) {
        fprintf(stderr, "Error destroying mutex for logs file\n");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

int log_server_sensor_data(logs_file_t *logs_file, proto_sensor_data_t *sensor_data, uint32_t thread_id){
    return _log_sensor_data(logs_file, sensor_data, thread_id, "[%s] Thread %d received data from sensor %d. Value: %f\n");
}

int log_client_sensor_data(logs_file_t *logs_file, proto_sensor_data_t *sensor_data, uint32_t thread_id){
    return _log_sensor_data(logs_file, sensor_data, thread_id, "[%s] Thread %d sent data from sensor %d. Value: %f\n");
}


int _log_sensor_data(logs_file_t *logs_file, proto_sensor_data_t *sensor_data, uint32_t thread_id, const char* format){
    //printf("Logging sensor data\n");

    char *time = NULL;
    if (get_current_time(&time) == STATUS_ERROR) {
        fprintf(stderr, "Error getting current time\n");
        return STATUS_ERROR;
    }

    if (pthread_mutex_lock(&(logs_file->mutex)) != 0){
        fprintf(stderr, "Error locking logs file mutex\n");
        free(time);
        time = NULL;
        return STATUS_ERROR;
    }
    fprintf(logs_file->file, format, time, thread_id,sensor_data->hdr.sensor_id, get_float_value(sensor_data));
    fflush(logs_file->file);
    if (pthread_mutex_unlock(&(logs_file->mutex)) != 0){
        fprintf(stderr, "Error unlocking logs file mutex\n");
        free(time);
        time = NULL;
        return STATUS_ERROR;
    }

    free(time);
    time = NULL;
    
    //printf("Sensor Data logged\n");

    return STATUS_SUCCESS;
}