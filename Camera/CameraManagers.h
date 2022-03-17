#include <stdio.h> 											
#include <unistd.h> 											// used for close
#include <fcntl.h> 												// used for open
#include <stdlib.h> 											// used for malloc ...
#include <sys/ioctl.h> 										// used for ioctl function
#include <sys/mman.h> 										// Used for mmap function
#include <string.h> 											// Used for memset function
#include <linux/videodev2.h> 							// Used to take raw input from camera
#include <errno.h> 												// Used for checking errno
#include "videoframes.h" 									// Used for videoframes 
/* #include <asm/types.h> 										// for printing capabilities */

videoframes *CameraManagers( int deviceCnt )
{
	/* int deviceCnt = 2 ;											// used fot testing purpose */
// File descriptor for external webcam
	int camera ;
// Accessing the camera module
	char pathDev[20]; 											// Path to the camera module
	sprintf(pathDev,"/dev/video%d",deviceCnt) ;
	/* sprintf(pathDev, "/sys/class/video4linux/video%d",deviceCnt) ; */
	/* printf("%s", pathDev) ; */
	camera = open(pathDev,O_RDWR) ;
	if( camera < 0 ){
		fprintf(stderr,"Camera module unaccessible\n") ;
		/* printf("%s",pathDev) ; */
		return NULL ;
	}

// Ask if the device can capture video device
	// "struct v4l2_capability" will fill the structure variable here capability with 
	// capabilities (information) the camera has
	// VIDIOC_QUERYCAP flag queries device capabilities
	struct v4l2_capability capability ;
	if ( ioctl( camera, VIDIOC_QUERYCAP, &capability ) == -1 ){
		fprintf(stderr,"Cameera ran into problem : VIDIOC_QUERYCAP\n") ;
		fprintf(stderr,"Error : %s", strerror(errno)) ;
		return NULL ;
	}

// Set Image format required from camera
	struct v4l2_format imageFormat ;
	imageFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE ;
	imageFormat.fmt.pix.width = 640 ;
	imageFormat.fmt.pix.height = 480 ; 							// 480 p video sample
	imageFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG ; 
	imageFormat.fmt.pix.field = V4L2_FIELD_NONE ; 	// Indicates progressive compression

// Telling the device format required of the frames extracted
	if( ioctl( camera, VIDIOC_S_FMT, &imageFormat ) == -1 ){
		fprintf(stderr, "Image extraction requirements not met, VIDIOC_S_FMT \n") ;
		fprintf(stderr, "Error : %s", strerror(errno)) ;
		return NULL ;
	}

// Requesting buffer from the device
	struct v4l2_requestbuffers requestBuffer = {0} ;
	requestBuffer.count = 1 ; 								// Number of buffers requested
	requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE ; 
	requestBuffer.memory = V4L2_MEMORY_MMAP ; // memory mapping required

	if( ioctl( camera, VIDIOC_REQBUFS, &requestBuffer ) == -1 ) {
		fprintf(stderr, "Unable to request buffers, VIDIOC_REQBUFS\n") ;
		fprintf(stderr, "Error : %s", strerror(errno)) ;
		return NULL ;
	}

// Querry the buffer for getting the raw data i,e asks the buffer you requested
// and maps the buffer to physical address in memory
	struct v4l2_buffer queryBuffer = {0};
	queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE ;
	queryBuffer.index = 0;
	queryBuffer.memory = V4L2_MEMORY_MMAP ;

	if ( ioctl( camera, VIDIOC_QUERYBUF, &queryBuffer ) == -1 ){
		fprintf(stderr, "Unable to querry the buffer, VIDIOC_QUERYBUF\n") ;
		fprintf(stderr, "Error : %s", strerror(errno)) ;
		return NULL ;
	}

	char *buffer = (char *)mmap( NULL, queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, camera, queryBuffer.m.offset ) ;

	memset( buffer, 0, queryBuffer.length) ;

// Get Frames from the device
// Create a buffer so device knows which buffer we are talking about

	struct v4l2_buffer bufferinfo ;
	memset( &bufferinfo, 0, sizeof(bufferinfo) ) ;
	bufferinfo.index = 0 ;
	bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE ;
	bufferinfo.memory = V4L2_MEMORY_MMAP ;

// Activating Streaming	
	int type = bufferinfo.type ;
	if( ioctl( camera, VIDIOC_STREAMON, &type ) == -1 ){
		fprintf(stderr, "Unable to start the streaming , VIDIOC_STREAMON\n" ) ;
		fprintf(stderr, "Error : %s", strerror(errno)) ;
		return NULL ;
	}

/********************** Looping begins here *****************************/
	int frameNum = 0 ; 											// It counts the number of images captured
	char frameName[45] ;
	sprintf( frameName, "%s.yuy", pathDev ) ;
	videoframes *frames ;
	strcpy( frames->location, frameName ) ;
	frames->deviceId = deviceCnt ;

// Capture 30 frames from the device
	while( frameNum < 30 ) {
		// Queing the buffer
			if( ioctl( camera, VIDIOC_QBUF, &bufferinfo ) == -1 ){
				fprintf(stderr, "unable to start buffering, VIDIOC_QBUF\n") ;
				fprintf(stderr, "Error : %s", strerror(errno)) ;
				return NULL ;
			}

		// Dequing the buffer
			if( ioctl( camera, VIDIOC_DQBUF, &bufferinfo ) == -1 ){
				fprintf(stderr, "unable to start dequeing the buffer, VIDIOC_DQBUF\n") ;
				fprintf(stderr, "Error : %s", strerror(errno)) ;
				return NULL ;
			}

		// Frames get written after dequeing the buffer
			printf("The buffer has %d kilobytes of data \n", bufferinfo.bytesused/1024 ) ;

		// Write data to the file
			int outFile = open("output.yuy", O_APPEND | O_CREAT) ;
			/* printf("%d", outFile) ; */
			if( outFile == -1 ){
				/* fprintf(stderr, "unable to open file: %s", frameName) ; */
				fprintf(stderr, "unable to open file: output.yuy") ,
				strerror(errno) ;
				return NULL ;
			}

			int bufPos = 0,													// position in buffer
					outFileMemBlockSize = 0, 						// amount to copy from buffer
					remainingBufferSize = bufferinfo.bytesused ; 
			// remainingBufferSize is dec by memBlockSize amount on each loop so we do not
			// overwrite the buffer
			char *outFileMemBlock = NULL ; 					// new memory block to copy from buffer
			int itr = 0 ; 													// counts no of iterations
			
			while( remainingBufferSize > 0 ){
				bufPos += outFileMemBlockSize ; 			// Updating the buffer on each loop
				outFileMemBlockSize = 1024 ; 					// Setting the reqd memory block size
				
				// Allocating memory to pointer to copy data from buffer
				outFileMemBlock = (char *)malloc( outFileMemBlockSize ) ;
				if( outFileMemBlock == NULL ){
					fprintf(stderr, "unable to allocate memory to outFileMemBlock") ;
					strerror(errno) ;
				}
				// Copying data from the memory block into the buffer
				memcpy( outFileMemBlock, buffer+bufPos, outFileMemBlockSize ) ;
				// Writing data to file
				write( outFile, outFileMemBlock, outFileMemBlockSize ) ;
				/* fprintf( outFile, "%s", outFileMemBlock ) ; */

				// Calculating remaining data to be read
				remainingBufferSize -= outFileMemBlockSize ;
				// Updating the amount of memory to be read (if necessary)
				if( outFileMemBlockSize > remainingBufferSize ){
					outFileMemBlockSize = remainingBufferSize ;
				}
				// displaying remaining buffer size
				printf("%d Remaining bytes: %d\n",itr++,remainingBufferSize ) ;
				free(outFileMemBlock) ;
			}

			close(outFile) ;
	}

// End streaming from device
	if( ioctl( camera, VIDIOC_STREAMOFF, &type ) == -1 ){
		fprintf(stderr, "Could not end streaming, VIDIOC_STREAMOFF\n") ;
		fprintf(stderr, "Error : %s", strerror(errno)) ;
		return NULL ;

		frameNum++ ;
	}

/* Closing the read files */
	close(camera) ;
	return frames ;
}
