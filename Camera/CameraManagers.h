#include <stropts.h> 											// used for ioctl function
#include <sys/mman.h> 										// Used for mmap function
#include <string.h> 											// Used for memset function
#include "videodev2.h" 										// Used to take raw input from camera

CameraManagers( int deviceCnt )
{
// File descriptor for external webcam
	int camera1 ;
// Accessing the camera module
	char pathDev[20]; 											// Path to the camera module
	sprintf(pathDev,"/sys/class/video4linux/video%d",deviceCnt)
	camera1 = open(path,O_RDWR) ;
	if( camera1 < 0 ){
		fprintf(stderr,"Camera module unaccessible\n") ;
		return 1 ;
	}

// Ask if the device can capture video device
	// "v4l2_capability" will fill the structure variable here capability with 
	// capabilities (information) the camera has
	// VIDIOC_QUERYCAP flag queries device capabilities
	v4l2_capability capability ;
	if ( ioctl( camera1, VIDIOC_QUERYCAP, &capability ) ){
		fprintf(stderr,"Cameera ran into problem : VIDIOC_QUERYCAP\n") ;
		return 1 ;
	}

// Set Image format required from camera
	v4l2_format imageFormat ;
	imageFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE ;
	imageFormat.fmt.pix.width = 852 ;
	imageFormat.fmt.pix.height = 480 ; 							// 480 p video sample
	imageFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_SRGGB10 ; // Raw data
	imageFormat.fmt.pix.field = V4L2_FIELD_NONE ; 	// Indicates progressive compression

// Telling the device format required of the frames extracted
	if( ioctl( camera1, VIDIOC_S_FMT, imageFormat ) < 0 ){
		fprintf(stderr, "Image extraction requirements not met \n") ;
		return 1 ;
	}

// Requesting buffer from the device
	v4l2_requestbuffers requestBuffer = {0} ;
	requestBuffer.count = 1 ; 								// Number of buffers requested
	requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE ; 
	requestBuffer.memory = V4L2_MEMORY_MMAP ; // memory mapping required

	if( ioctl( camera1, VIDIOC_REQBUFS, &requestBuffer ) < 0 ) {
		fprintf(stderr, "Unable to request buffers, VIDIOC_REQBUFS") ;
		return 1 ;
	}

// Querry the buffer for getting the raw data i,e asks the buffer you requested
// and maps the buffer to physical address in memory
	v4l2_buffer queryBuffer = {0};
	queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE ;
	queryBuffer.index = 0;
	queryBuffer.memory = V4L2_MEMORY_MMAP ;

	if ( ioctl( camera1, VIDIOC_QUERYBUF, &queryBuffer ) < 0){
		fprintf(stderr, "Unable to querry the buffer, VIDIOC_QUERYBUF") ;
		return 1 ;
	}

	char *buffer = mmap( NULL, queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, camera1, queryBuffer.m.offset ) ;

	memset( buffer, 0, queryBuffer.length) ;

// Get Frames from the device
// Create a buffer so device knows which buffer we are talking about

	v4l2_buffer bufferinfo ;
	memset( &bufferinfo, 0, sizeof(bufferinfo) ) ;
	bufferinfo.index = 0 ;
	bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE ;
	bufferinfo.memory = V4L2_MEMORY_MMAP ;

// Activating Streaming	
	int type = bufferinfo.type ;
	if( ioctl( camera1, VIDIOC_STREAMON, &type ) < 0 ){
		fprintf(stderr, "Unable to start the streaming , VIDIOC_STREAMON" ) ;
		return 1 ;
	}

/********************** Looping begins here *****************************/
// Queing the buffer
	if( ioctl( camera1, VIDIOC_QBUF, &bufferinfo ) )
}
