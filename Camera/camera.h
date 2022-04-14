typedef struct Camera{
	char name[10] ; 												// name allocated by the system
	unsigned batchCnt = 0 ; 								// count of video file batches maintained only by the topmost node 
  Camera *queueAddr = NULL ;							// address allocated in the queue
	Camera *next = NULL ; 									// address of the next batch of frames in queue for given camera
} camera ;
