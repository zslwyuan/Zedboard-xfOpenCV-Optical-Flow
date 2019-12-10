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
  if (x>255) x=255;
  if (x<0) x=0;

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

int main(int, char**)
{
    //cv::VideoCapture cap("in.avi"); // open the default camera
    cv::VideoCapture cap(0+CV_CAP_V4L2); 
    cap.set(CV_CAP_PROP_FRAME_WIDTH,640);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,480);

    if(!cap.isOpened())  // check if we succeeded
    {
        std::cout << "failed to load video \n";
        return -1;
    }   
    else
    {
         std::cout << "succeed to load video \n";
    }
    cv::Mat image_last_rgb;
    cv::Mat tmp;
    for (int cnt=0;cnt<100;cnt++)
{
    bool suc = cap.read(image_last_rgb);
    for (int tmp_cnt=0;tmp_cnt<6;tmp_cnt++)
    {
        suc = cap.grab();
    }
    
    
    while (!suc || image_last_rgb.empty())
    {
        suc = cap.read(image_last_rgb);
	std::cout << "tring again to load frame #" << cnt << "\n";
    }


    cv::imwrite(std::to_string(cnt)+".bmp",image_last_rgb);
    std::cout << "writing frame #" << cnt << "\n";
    
}
image_last_rgb.release();
cap.release();
tmp.release();
    return 0;
}
