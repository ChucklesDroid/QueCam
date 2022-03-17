/* Compile with flags -std=C99, -lpthread */
#include "../headers/header.h"
bool OPTIONS ; 														/* used to decide the input of video file */

int main( int argc, char *argv[] )
{
	if( argc != 1 ){
		if( *(++*(++argv)) == 'c' ){ 				  /* Checks options in a given program */
			OPTIONS = 1 ;
		} else {
			OPTIONS = 0 ;
		}
	} else {
		fprintf(stderr, "Quecam : options required\n") ;
		fprintf(stderr, "-c | -v <PATH>") ;
	}


/* Executed when camera is connected */
	if( OPTIONS == 1 ){
		char name[30] = "//dev//video" ; 			/* location to search for camera */
		DIR *dp ; 														/* directory pointer */
		struct dirent *fp ; 									/* file pointer */
		char pathname[30] ;
		int cameraCnt = 0 ,  									/* No of camera's connected to the device */
				loopingCnt = 0 ;   								/* looping counter */

		/* Calculating the number of Cameras */
		if( (dp = opendir(name)) == NULL ){
			fprintf(stderr, "%s: could not be opened\n", name) ;
			exit(1) ;
		} while( (fp = readdir(dp)) != NULL ){
		/* skips the parent and current directory */
			if( strcmp(fp->d_name, ".") == 0 || strcmp(fp->d_name, "..")){
				continue ;
			}	cameraCnt++ ;
		} closedir(dp) ;

		/* Creating central queue and allocating cameras and videofiles to it */
		camera *centralQueue[cameraCnt] ;    	// Central queue
		pthread_t id[cameraCnt] ; 						// Threads to work on all Cameras 
		if( (dp = opendir(name)) == NULL ){
			fprintf(stderr, "%s: could not be opened\n", name) ;
			return 1 ;
		} while((fp = readdir(dp)) != NULL){
				centralQueue[loopingCnt] = malloc(sizeof(camera)) ;
				strcpy(centralQueue[loopingCnt]->name, fp->d_name) ;
				pthread_create( &id[loopingCnt], NULL, cameraManager(), &centralQueue[loopingCnt] ) ;
				loopingCnt++ ;
		} 

		/* Executing the videofiles from the queue */

		closedir(dp) ;
	} 

	else{

	} 
	return 0 ;
}
