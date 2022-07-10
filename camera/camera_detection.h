/* This will be the daemon thread that will detect cameras connected or disconnected to the system */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

struct Args{
 	DIR *dp;
	struct dirent *fp;
} ;

void printArray( int *str, int len)
{
	for( int i=0; i < len; i++ ){
		printf("%d\t", *(str+i));
	} printf("\n");
}

void slice( char* src, char* dest, int offset, int stop )
{
int len = strlen(src) ;
int iterations = stop-offset+1 ;
for( int i=0; i < iterations; i++ ){
	*(dest+i) = *(src+offset+i) ;
}
*(dest+iterations) = '\0' ;
}

void sliceint( char* src, int* dest, int offset )
{
*(dest) = atoi((src+offset));
}


int compar(const void* key, const void* member)
{
return (*(int*)key - *(int*)member);
}


void *cameraDetection( struct Args *args )
{
char cameraLocation[30] = "/dev" ;
int  cameraCnt = 0,
		 cameraModules[100],
		 camera = 0;
// while(true){
	
// Cycle through all files
	while(((args->fp) = readdir((args->dp))) != NULL){
		if( strcmp(".", (args->fp)->d_name) == 0 || strcmp("..", (args->fp)->d_name) == 0 )
			continue ;

// Checking for camera-name which are saved as video
		slice((args->fp)->d_name, cameraLocation, 0, strlen((args->fp)->d_name)-2) ;
		if( strcmp("video", cameraLocation) == 0 ){
			printf("camera%d detected: /dev/%s\n", ++cameraCnt, (args->fp)->d_name) ;
			sliceint((args->fp)->d_name, &camera, strlen((args->fp)->d_name)-1) ;
			printf("camera :%d\n", camera);
			cameraModules[cameraCnt-1] = camera ;
			
			qsort(cameraModules, cameraCnt, sizeof(*cameraModules), compar);
			// if(bsearch(&camera, cameraModules, cameraCnt, sizeof(camera), (void *)(compar)) != NULL){
				// fprintf(stdout, "camera%d: added to centralqueue\n", camera) ;
			// }
		}
	}
printArray(cameraModules, cameraCnt) ;
// }
}
