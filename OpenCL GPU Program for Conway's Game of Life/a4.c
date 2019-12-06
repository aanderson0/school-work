#include <stdlib.h>
#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define PROG_FILE "a4.cl"
#define KERNEL_FUNC "breed"
#define BOARDSIZE 26
#define DELAY 100000
#define CL_TARGET_OPENCL_VERSION 220

int max_y = 0, max_x = 0;
int numKernels = 1;
int board1[BOARDSIZE][BOARDSIZE];
int board2[BOARDSIZE][BOARDSIZE];

int drawScreen(int board) {
   int c, i, j;
   float multx, multy;
   getmaxyx(stdscr, max_y, max_x);
   multx = (float)max_x / BOARDSIZE;
   multy = (float)max_y / BOARDSIZE;
   clear();

   // print board
   if(board == 0) {
      for (i=0; i<BOARDSIZE; i++) {
         for(j=0; j<BOARDSIZE; j++){
            if(board1[i][j]) {
               mvprintw((int)(i*multy), (int)(j*multx), "o");
            }
         }
      }
   }
   else {
   for (i=0; i<BOARDSIZE; i++) {
         for(j=0; j<BOARDSIZE; j++){
            if(board2[i][j]) {
               mvprintw((int)(i*multy), (int)(j*multx), "o");
            }
         }
      }
   }

   refresh();
   usleep(DELAY);

   // read keyboard and exit if 'q' pressed
   c = getch();
   if (c == 'q') return(1);
   else return(0);
}

int main(int argc, char* argv[]) {
   
   int boardnum = 0;
   int loopFlag = 0;
   int n = 0;
   int output = 1;
   cl_device_id device_id = NULL;
   cl_context context = NULL;
   cl_command_queue command_queue = NULL;
   cl_mem buf1, buf2 = NULL;
   cl_program program = NULL;
   cl_kernel kernel = NULL;
   cl_platform_id platform_id = NULL;
   cl_uint ret_num_devices;
   cl_uint ret_num_platforms;
   cl_int ret;

	for(int i = 0; i < argc; i++) {
		if(!strcmp(argv[i], "-o")) {
			output = 0;
		}
		else if (!strcmp(argv[i], "-n")) {
			numKernels = atoi(argv[i + 1]);
		}
	}

   //initialize board 1
   for(int i = 1; i < BOARDSIZE -1; i++) {
      for(int j = 1; j < BOARDSIZE -1; j++) {
         int dice = rand() % 100;
         if(dice > 70) board1[i][j] = 1;
         else board1[i][j] = 0;
      }
   }
   
   if (output) {
	   // initialize curses
	   initscr();
	   noecho();
	   cbreak();
	   timeout(0);
	   curs_set(FALSE);
	   getmaxyx(stdscr, max_y, max_x);
   }

   // initialize OpenCL
   FILE *fp;
   char *source_str;
   size_t source_size;
   fp = fopen(PROG_FILE, "r");
   source_str = (char*)malloc(0x100000);
   source_size = fread(source_str, 1, (0x100000), fp);
   fclose(fp);

   ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
   ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
   context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
   command_queue = clCreateCommandQueueWithProperties(context, device_id, 0, &ret);
   program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
   ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
   kernel = clCreateKernel(program, KERNEL_FUNC, &ret);

   size_t global_work_size[1];
   size_t local_work_size[1];
   global_work_size[0] = (size_t)numKernels;
   local_work_size[0] = (size_t)numKernels;

   while((output && !loopFlag) || n < 1000) {
      if(!boardnum) { 

		 if(numKernels == 1) {
			 for(int i = 1; i < 26 - 1; i++) {
				for(int j = 1; j < 26 - 1; j++){
					int neighbours = 0;
					if(board1[i-1][j+0]) neighbours++;
					if(board1[i-1][j-1]) neighbours++;
					if(board1[i+0][j-1]) neighbours++;
					if(board1[i+1][j+0]) neighbours++;
					if(board1[i+1][j+1]) neighbours++;
					if(board1[i+0][j+1]) neighbours++;
					if(board1[i-1][j+1]) neighbours++;
					if(board1[i+1][j-1]) neighbours++;
				   
					if(board1[i][j] && neighbours == 2 || neighbours == 3) {
						board2[i][j] = 1;
					}
					else if(!board1[i][j] && neighbours == 3) {
						board2[i][j] = 1;
					}
					else {
						board2[i][j] = 0;
					}
				}
			}
		 }
		 else {

			 // set args and run
			 buf1 = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, BOARDSIZE * BOARDSIZE * sizeof(int), board1, &ret);
			 buf2 = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, BOARDSIZE * BOARDSIZE * sizeof(int), board2, &ret);
			 ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&buf1);
			 ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&buf2);
			 ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);

			 // get result
			 ret = clEnqueueReadBuffer(command_queue, buf2, CL_TRUE, 0, BOARDSIZE * BOARDSIZE * sizeof(int), board2, 0, NULL, NULL);

			 ret = clReleaseMemObject(buf1);
			 ret = clReleaseMemObject(buf2);
		 }
	 
         boardnum = 1;
         if(output) loopFlag = drawScreen(boardnum);
      }
      else {
		if(numKernels == 1) {
			 for(int i = 1; i < 26 - 1; i++) {
				for(int j = 1; j < 26 - 1; j++){
					int neighbours = 0;
					if(board2[i-1][j+0]) neighbours++;
					if(board2[i-1][j-1]) neighbours++;
					if(board2[i+0][j-1]) neighbours++;
					if(board2[i+1][j+0]) neighbours++;
					if(board2[i+1][j+1]) neighbours++;
					if(board2[i+0][j+1]) neighbours++;
					if(board2[i-1][j+1]) neighbours++;
					if(board2[i+1][j-1]) neighbours++;
				   
					if(board2[i][j] && neighbours == 2 || neighbours == 3) {
						board1[i][j] = 1;
					}
					else if(!board2[i][j] && neighbours == 3) {
						board1[i][j] = 1;
					}
					else {
						board1[i][j] = 0;
					}
				}
			}
		}
		else {

			 // set args and run
			 buf1 = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, BOARDSIZE * BOARDSIZE * sizeof(int), board1, &ret);
			 buf2 = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, BOARDSIZE * BOARDSIZE * sizeof(int), board2, &ret);
			 ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&buf1);
			 ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&buf2);
			 ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);

			 // get result
			 ret = clEnqueueReadBuffer(command_queue, buf1, CL_TRUE, 0, BOARDSIZE * BOARDSIZE * sizeof(int), board1, 0, NULL, NULL);

			 ret = clReleaseMemObject(buf1);
			 ret = clReleaseMemObject(buf2);
		 }
         boardnum = 0;
         if(output) loopFlag = drawScreen(boardnum);
      }
	  n++;
   }
   //close everything
   if(output) endwin();
   ret = clFlush(command_queue);
   ret = clFinish(command_queue);
   ret = clReleaseKernel(kernel);
   ret = clReleaseProgram(program);
   ret = clReleaseCommandQueue(command_queue);
   ret = clReleaseContext(context);
   free(source_str);

   return 0;
}
