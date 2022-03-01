#include <stdio.h>
#include "request.h"
#include "common.h"
#include "common_threads.h"
#include "io_helper.h"

#define N_CONSUMERS_DEFAULT 3
#define MAX 1000

int buffer[MAX];
int fill_ptr = 0;
int use_ptr = 0;
int count = 0;
char default_root[] = "../test_files";
pthread_cond_t not_empty, not_full;
pthread_mutex_t lock;

void put(int value) {
	buffer[fill_ptr] = value;
	fill_ptr = (fill_ptr + 1) % MAX;
	count++;
}

int get() {
	int tmp = buffer[use_ptr];
	use_ptr = (use_ptr + 1) % MAX;
	count--;
	return tmp;
}

void *consumer(void *arg) {
	int tid = (int) arg;
	pthread_detach(pthread_self());
	while(1) {
		Pthread_mutex_lock(&lock);
		while(count == 0) Pthread_cond_wait(&not_empty, &lock);
		int conn_fd = get();
		Pthread_cond_signal(&not_full);
		Pthread_mutex_unlock(&lock);
		// perform action on cd
		request_handle(conn_fd, tid);
		close_or_die(conn_fd);
	} 
}
int main(int argc, char *argv[]) {
    int c, nconsumers;
    char *root_dir = default_root;
    int port = 10000;
	nconsumers = N_CONSUMERS_DEFAULT;
	Pthread_cond_init(&not_empty, NULL);
	Pthread_cond_init(&not_full, NULL);
	Pthread_mutex_init(&lock, NULL);
    
    while ((c = getopt(argc, argv, "d:p:t:")) != -1)
	switch (c) {
	case 'd':
	    root_dir = optarg;
	    break;
	case 'p':
	    port = atoi(optarg);
	    break;
	case 't':
	    nconsumers = atoi(optarg);
	    break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t n_consumers]\n");
	    exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);
	pthread_t tid;

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);

	for (int i = 0; i < nconsumers; i++) {
		Pthread_create(&tid, NULL, consumer, i);
	}
	
	// "producer"
    while (1) {
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
		Pthread_mutex_lock(&lock);
		while (count == MAX) Pthread_cond_wait(&not_full, &lock);
		put(conn_fd);
		Pthread_cond_signal(&not_empty);
		Pthread_mutex_unlock(&lock);
    }
    return 0;
}


    


 
