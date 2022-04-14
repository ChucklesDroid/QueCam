#include <stdio.h>
#include "../Gpu/GpuManagers.h"
#include "../Camera/CameraManagers.h"
#include <stropts.h> 											// To use ioctl function
#include <fcntl.h>
#include <dirent.h> 											// To open a directory
#include "../Camera/camera.h"
#include <pthread.h> 											// Used for creating threads conforming to POSIX standards
