#include "CameraManagers.h"
#include "videoframes.h"

int main( int argc, char *argv[] )
{
	videoframes *frame ;
	frame = CameraManagers(0) ;
	if ( frame != NULL ){
		fprintf(stderr, "Frames not allocated") ;
	}
	return 0 ;
}
