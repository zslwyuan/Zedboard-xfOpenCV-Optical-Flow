/***************************************************************************
 Copyright (c) 2019, Xilinx, Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 3. Neither the name of the copyright holder nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ***************************************************************************/
#include "xf_headers.h"
#include "xf_pyr_dense_optical_flow_config.h"
#include "opencv2/opencv.hpp"
#include <cstring>
#include <unistd.h>
#include <sys/time.h>
#define MILLION 1000000

/* Color Coding */
// kernel returns this type. Packed strcuts on axi ned to be powers-of-2.
typedef struct __rgba{
	  IN_TYPE r, g, b;
	  IN_TYPE a;    // can be unused         
} rgba_t;
typedef struct __rgb{
	  IN_TYPE r, g, b;
} rgb_t;

typedef cv::Vec<unsigned short, 3> Vec3u;
typedef cv::Vec<IN_TYPE, 3> Vec3ucpt;

const float powTwo15 = pow(2,15);
#define THRESHOLD 3.0
#define THRESHOLD_R 3.0
/* color coding */

// custom, hopefully, low cost colorizer.
void getPseudoColorInt (IN_TYPE pix, signed char fx, signed char fy, rgba_t& rgba)
{
  // TODO get the normFac from the host as cmdline arg
  const int normFac = 2;

  int y = 127 + (int) (fy * normFac);
  int x = 127 + (int) (fx * normFac);
  if (y>255) y=255;
  if (y<0) y=0;

  if (y<127+30 && y > 127-30) y =127;


  if (x>255) x=255;
  if (x<0) x=0;

  if (x<127+30 && x > 127-30) x =127;

  rgb_t rgb;
  if (x > 127) {
    if (y < 128) {
      // 1 quad
      rgb.r = x - 127 + (127-y)/2;
      rgb.g = (127 - y)/2;
      rgb.b = 0;
    } else {
      // 4 quad
      rgb.r = x - 127;
      rgb.g = 0;
      rgb.b = y - 127;
    }
  } else {
    if (y < 128) {
      // 2 quad
      rgb.r = (127 - y)/2;
      rgb.g = 127 - x + (127-y)/2;
      rgb.b = 0;
    } else {
      // 3 quad
      rgb.r = 0;
      rgb.g = 128 - x;
      rgb.b = y - 127;
    }
  }

  rgba.r = pix*1/2 + rgb.r*1/2; 
  rgba.g = pix*1/2 + rgb.g*1/2; 
  rgba.b = pix*1/2 + rgb.b*1/2;
  rgba.a = 0;
}



void pyrof_hw(cv::Mat im0, cv::Mat im1, signed char flowUmat[HEIGHT][WIDTH], signed char flowVmat[HEIGHT][WIDTH], xf::Mat<XF_32UC1,HEIGHT,WIDTH,XF_NPPC1> & flow, xf::Mat<XF_32UC1,HEIGHT,WIDTH,XF_NPPC1> & flow_iter, xf::Mat<XF_8UC1,HEIGHT,WIDTH,XF_NPPC1> mat_imagepyr1[NUM_LEVELS] , xf::Mat<XF_8UC1,HEIGHT,WIDTH,XF_NPPC1> mat_imagepyr2[NUM_LEVELS] , int pyr_h[NUM_LEVELS], int pyr_w[NUM_LEVELS])
{	                                                                              
	for(int l=0; l<NUM_LEVELS; l++)
	{
		mat_imagepyr1[l].rows = pyr_h[l];
		mat_imagepyr1[l].cols = pyr_w[l];
		mat_imagepyr1[l].size = pyr_h[l]*pyr_w[l];
		mat_imagepyr2[l].rows = pyr_h[l];
		mat_imagepyr2[l].cols = pyr_w[l];	
		mat_imagepyr2[l].size = pyr_h[l]*pyr_w[l];	
	}
	
	// mat_imagepyr1[0].copyTo(im0.data);
	// mat_imagepyr2[0].copyTo(im1.data);
	
	std::cout << "Copying data for pydown \n";

	unsigned int *m1 = (unsigned int *) mat_imagepyr1[0].data;
	unsigned int *ptr_im0 = (unsigned int *) im0.data;
	unsigned int *m2 = (unsigned int *) mat_imagepyr2[0].data;
	unsigned int *ptr_im1 = (unsigned int *) im1.data;
	int new_w = pyr_w[0] / 4;
	
	for(int i=0; i<pyr_h[0]; i++)
	{
		int offset = i*new_w;
		for(int j=0; j<new_w; j++)
		{
			*(m1+offset + j) = *(ptr_im0+offset + j);
			*(m2+offset + j) = *(ptr_im1+offset + j);
		}	
	}

	// memcpy(mat_imagepyr1[0].data, im0.data, sizeof(im0.data));
	// memcpy(mat_imagepyr2[0].data, im1.data, sizeof(im1.data));

	// for(int i=0; i<pyr_h[0]; i++)
	// {
	// 	for(int j=0; j<pyr_w[0]; j++)
	// 	{
	// 		mat_imagepyr1[0].write(i*pyr_w[0] + j,im0.data[i*pyr_w[0] + j]);
	// 		mat_imagepyr2[0].write(i*pyr_w[0] + j,im1.data[i*pyr_w[0] + j]);
	// 	}	
	// }


	std::cout << "Copied data for pydown \n";
	//creating image pyramid

	std::cout << "FPGA launch\n";
	#if __SDSCC__
		perf_counter hw_ctr;
		hw_ctr.start();
	#endif
	pyr_dense_optical_flow_pyr_down_accel(mat_imagepyr1, mat_imagepyr2);
	
	bool flag_flowin = 1;
	flow.rows = pyr_h[NUM_LEVELS-1];
	flow.cols = pyr_w[NUM_LEVELS-1];
	flow.size = pyr_h[NUM_LEVELS-1]*pyr_w[NUM_LEVELS-1];
	flow_iter.rows = pyr_h[NUM_LEVELS-1];
	flow_iter.cols = pyr_w[NUM_LEVELS-1];
	flow_iter.size = pyr_h[NUM_LEVELS-1]*pyr_w[NUM_LEVELS-1];
	
	for (int l=NUM_LEVELS-1; l>=0; l--) {
		
		//compute current level height
		int curr_height = pyr_h[l];
		int curr_width = pyr_w[l];
		
		//compute the flow vectors for the current pyramid level iteratively
		for(int iterations=0;iterations<NUM_ITERATIONS; iterations++)
		{
			bool scale_up_flag = (iterations==0)&&(l != NUM_LEVELS-1);
			int next_height = (scale_up_flag==1)?pyr_h[l+1]:pyr_h[l]; 
			int next_width  = (scale_up_flag==1)?pyr_w[l+1]:pyr_w[l]; 
			float scale_in = (next_height - 1)*1.0/(curr_height - 1);
			ap_uint<1> init_flag = ((iterations==0) && (l==NUM_LEVELS-1))? 1 : 0;
			if(flag_flowin)
			{
				flow.rows = pyr_h[l];
				flow.cols = pyr_w[l];
				flow.size = pyr_h[l]*pyr_w[l];
				pyr_dense_optical_flow_accel(mat_imagepyr1[l], mat_imagepyr2[l], flow_iter, flow, l, scale_up_flag, scale_in, init_flag);
				flag_flowin = 0;
			}
			else
			{
				flow_iter.rows = pyr_h[l];
				flow_iter.cols = pyr_w[l];
				flow_iter.size = pyr_h[l]*pyr_w[l];
				pyr_dense_optical_flow_accel(mat_imagepyr1[l], mat_imagepyr2[l], flow, flow_iter, l, scale_up_flag, scale_in, init_flag);
				flag_flowin = 1;
			}
		}//end iterative coptical flow computation
	} // end pyramidal iterative optical flow HLS computation
	#if __SDSCC__
		hw_ctr.stop();
		uint64_t hw_cycles = hw_ctr.avg_cpu_cycles();
	#endif
	std::cout << "FPGA done\n";
	
//write output flow vectors to Mat after splitting the bits.
	if(flag_flowin)
	{
		
		for(int i=0; i<pyr_h[0]; i++)
		{
			int offset = i*pyr_w[0];
			for(int j=0; j<pyr_w[0]; j++)
			{
				unsigned int tempcopy = 0;
				tempcopy = flow_iter.read(offset + j);
				short splittemp1 = (tempcopy>>16);
				short splittemp2 = (0x0000FFFF & tempcopy);
				signed char *uflow= (signed char*) &splittemp1;
				signed char *vflow= (signed char*) &splittemp2;
				
				flowUmat[i][j] = (signed char ) *uflow;
				flowVmat[i][j] = (signed char ) *vflow;
			}
		}
	}
	else
	{
		for(int i=0; i<pyr_h[0]; i++)
		{
			int offset = i*pyr_w[0];
			for(int j=0; j<pyr_w[0]; j++)
			{
				unsigned int tempcopy = 0;
				tempcopy = flow.read(offset + j);
				short splittemp1 = (tempcopy>>16);
				short splittemp2 = (0x0000FFFF & tempcopy);
				signed char *uflow= (signed char*) &splittemp1;
				signed char *vflow= (signed char*) &splittemp2;
				
				flowUmat[i][j] = (signed char ) *uflow;
				flowVmat[i][j] = (signed char ) *vflow;
			}
		}
	}

	return;
}

//using namespace cv;



#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;//init cond
cv::Mat global_image_last_rgb;
int processing = 1234;
int global_cnt = 0;

void *thread_cam(void*);
void *thread_of(void*);

int main(void){
    pthread_t t_a;
    pthread_t t_b;//two thread
 
    pthread_create(&t_a,NULL,thread_of,(void*)NULL);
    pthread_create(&t_b,NULL,thread_cam,(void*)NULL);//Create thread
    
    printf("t_a:0x%x, t_b:0x%x:", t_a, t_b);
    pthread_join(t_b,NULL);//wait a_b thread end
    pthread_join(t_a,NULL);//wait a_b thread end

    global_image_last_rgb.release();
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    exit(0);
}

void *thread_cam(void *junk)
{
    struct timespec tpstart;
    struct timespec tpend;
    //cv::VideoCapture cap("in.avi"); // open the default camera
    cv::VideoCapture cap(0+CV_CAP_V4L2); 
    cap.set(CV_CAP_PROP_FRAME_WIDTH,640);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,480);
    cap.set(CV_CAP_PROP_FPS,5.0);
    //cap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('Y', 'U', 'Y', '2'));
    //cap.set(CV_CAP_PROP_AUTO_EXPOSURE, 0.25);

    if(!cap.isOpened())  // check if we succeeded
    {
        std::cout << "failed to load video \n";
		processing = 0;
        return 0;
    }   
    else
    {
         std::cout << "succeed to load video \n";
    }
    
	int cnt = 0;

	bool the_first_frame = 1;
	long timedif;

    while (true)
    {
		cnt++;

		if (the_first_frame)
		{
			clock_gettime(CLOCK_MONOTONIC, &tpstart);
			the_first_frame = 0;
		}
		else
		{
			clock_gettime(CLOCK_MONOTONIC, &tpend);
        	timedif = MILLION*(tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_nsec-tpstart.tv_nsec)/1000;
			if (timedif > 30*1000000) break;
		}

		pthread_mutex_lock(&mutex);
		
		global_cnt++;
        bool suc = cap.read(global_image_last_rgb);
        for (int tmp_cnt=0;tmp_cnt<6;tmp_cnt++)
        {
            suc = cap.grab();
        }       
        while (!suc || global_image_last_rgb.empty())
        {
            suc = cap.read(global_image_last_rgb);
    		std::cout << "tring again to load frame #" << cnt << "\n";
        }
        std::cout << "got frame #" << cnt << "\n";
		pthread_mutex_unlock(&mutex); 
		

		usleep(10000);
    }
    processing = 0;

    cap.release();
    return 0;
}


void *thread_of(void *junk)
{
    struct timespec tpstart;
    struct timespec tpend;
	cv::VideoWriter writer("VideoTest.avi", CV_FOURCC('I', '4', '2', '0'), 5.0, cv::Size(640, 480));

	//allocating memory spaces for all the hardware operations
	static xf::Mat<XF_8UC1,HEIGHT,WIDTH,XF_NPPC1>imagepyr1[NUM_LEVELS];
	static xf::Mat<XF_8UC1,HEIGHT,WIDTH,XF_NPPC1>imagepyr2[NUM_LEVELS];
	static xf::Mat<XF_32UC1,HEIGHT,WIDTH,XF_NPPC1>flow;
	static xf::Mat<XF_32UC1,HEIGHT,WIDTH,XF_NPPC1>flow_iter;
	for(int i=0; i<NUM_LEVELS ; i++)
	{
		imagepyr1[i].init(HEIGHT,WIDTH);
		imagepyr2[i].init(HEIGHT,WIDTH);
	}
	flow.init(HEIGHT,WIDTH);
	flow_iter.init(HEIGHT,WIDTH);
	
	//initializing flow pointers to 0
	//initializing flow vector with 0s
	cv::Mat init_mat= cv::Mat::zeros(HEIGHT,WIDTH, CV_32SC1);
	flow_iter.copyTo((XF_PTSNAME(XF_32UC1,XF_NPPC1)*)init_mat.data);
	flow.copyTo((XF_PTSNAME(XF_32UC1,XF_NPPC1)*)init_mat.data);
	init_mat.release();
	
	signed char glx[HEIGHT][WIDTH]; 
	signed char gly[HEIGHT][WIDTH];

	// Auviz Hardware implementation
	/***********************************************************************************/
	//Setting image sizes for each pyramid level
	int pyr_w[NUM_LEVELS], pyr_h[NUM_LEVELS];
	pyr_h[0] = HEIGHT;
	pyr_w[0] = WIDTH;
	for(int lvls=1; lvls< NUM_LEVELS; lvls++)
	{
		pyr_w[lvls] = (pyr_w[lvls-1]+1)>>1;
		pyr_h[lvls] = (pyr_h[lvls-1]+1)>>1;
	}

	cv::Mat image_last_rgb;
	cv::Mat image_new_rgb;

	while (global_cnt<10)
		usleep(10000);

	pthread_mutex_lock(&mutex);
	global_image_last_rgb.copyTo(image_last_rgb);
	pthread_mutex_unlock(&mutex);

	cv::Mat im0;
	cv::cvtColor(image_last_rgb, im0, CV_BGR2GRAY);
	cv::Mat color_code_img;
	color_code_img.create(im0.size(),CV_8UC3);

	bool the_first_frame = 1;
	long timedif;

	while (processing > 0)
	{
		if (the_first_frame)
		{
			pthread_mutex_lock(&mutex);
			// cv::imwrite(std::to_string(global_cnt)+".bmp",global_image_last_rgb);
			std::cout << "cloning frame #" << global_cnt << "\n===================\n===================\n";
			global_image_last_rgb.copyTo(image_new_rgb);
			pthread_mutex_unlock(&mutex); 
			clock_gettime(CLOCK_MONOTONIC, &tpstart);
			the_first_frame = 0;
		}
		else
		{
			clock_gettime(CLOCK_MONOTONIC, &tpend);
        	timedif = MILLION*(tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_nsec-tpstart.tv_nsec)/1000;

			while (timedif < 200000)
			{
				clock_gettime(CLOCK_MONOTONIC, &tpend);
				timedif = MILLION*(tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_nsec-tpstart.tv_nsec)/1000;
			}
			tpstart = tpend;

			pthread_mutex_lock(&mutex);
			// cv::imwrite(std::to_string(global_cnt)+".bmp",global_image_last_rgb);
			std::cout << "cloning frame #" << global_cnt << "\n===================\n===================\n";
			global_image_last_rgb.copyTo(image_new_rgb);
			pthread_mutex_unlock(&mutex); 
		}
		std::cout << "processing frame#" << global_cnt << "\n";

		cv::Mat im1;		
		cv::cvtColor(image_new_rgb, im1, CV_BGR2GRAY);		

		std::cout << "initilization done for frame#" << global_cnt << "\n";

		//call the hls optical flow implementation
		
		pyrof_hw (im0, im1, glx, gly, flow, flow_iter, imagepyr1, imagepyr2, pyr_h, pyr_w);
		
		std::cout << "acceleration done for frame#" << global_cnt << "\n";

		//Color code the flow vectors on original image

		Vec3ucpt color_px;
		for(int rc=0;rc<im0.rows;rc++)
		{
			for(int cc=0;cc<im0.cols;cc++)
			{
				rgba_t colorcodedpx;
				getPseudoColorInt(im0.at<unsigned char>(rc,cc),glx[rc][cc],gly[rc][cc],colorcodedpx);
				color_px = Vec3ucpt(colorcodedpx.b, colorcodedpx.g, colorcodedpx.r);
				color_code_img.at<Vec3ucpt>(rc,cc) = color_px;
			}
		}

		//end color coding

		std::cout << "post-processing done for frame#" << global_cnt << "\n";

		writer.write(color_code_img);

		std::cout << "wrting to video file done for frame#" << global_cnt << "\n";		
		
		//releaseing mats and pointers created inside the main for loop		
			
		std::cout << "memory release done for frame#" << global_cnt << "\n";

		im1.copyTo(im0);
		im1.release(); 


	}
	std::cout << "Task done\n";
	color_code_img.release();
	im0.release();
	image_last_rgb.release();
    return 0;
}
