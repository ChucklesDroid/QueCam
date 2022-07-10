#include "camera_detection.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


int main( int argc, char *argv[] )
{
	pthread_t id;
	DIR* dp;
	struct dirent* fp ;

	if((dp=opendir("/dev")) == NULL){
		fprintf(stderr, "/dev: could not be opened\n");
		exit(1) ;
	} 

	struct Args args = {dp, fp};
	pthread_create(&id, NULL, (void *)cameraDetection, &args);
	/* sleep(5); */


	pthread_join(id, NULL);
	return 0;
}
