#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include<vector>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <unistd.h>
#include <iostream>

#define LOCALPORT 8888
#define BUF_SIZE 655350
int main(int argc,char *argv[])
{
	int mysocket,len;
	int i=0;
	struct sockaddr_in6 addr;
	int addr_len;
	char msg[200];
	char buf[300];
 
	if((mysocket=socket(AF_INET6,SOCK_DGRAM,0))<0)
	{
		perror("error:");
		return(1);
	}
	else
	{
		printf("socket created ...\n");
		printf("socket id :%d \n",mysocket);
	}
 
	addr_len=sizeof(struct sockaddr_in6);
	bzero(&addr,sizeof(addr));
	addr.sin6_family=AF_INET6;
	addr.sin6_port=htons(LOCALPORT);
	addr.sin6_addr=in6addr_any;
 
	if(bind(mysocket,(struct sockaddr *)&addr,sizeof(addr))<0)
	{
		perror("connect");
		return(1);
	}
	else
	{
		printf("bink ok .\n");
		printf("local port : %d\n",LOCALPORT);
	}

    char buffer[ BUF_SIZE ];
    std::vector<char> vec(BUF_SIZE);
    cv::Mat img_decode; 
    std::string filename="";   
    int size = 0;
    int mylen = 0;
    long long j= 0;
    char cokstart[ 1 ]={0x33};
    memset( buffer, 0, sizeof(buffer) );

    while(1)
    {
        //size = recv(mysocket,buffer,4,0);
		size=recvfrom(mysocket,buffer,4,0,(struct sockaddr *)&addr,(socklen_t*)&addr_len);

        std::cout << " receive length buffer size=" << size << "bytes\n";
    
        mylen = ((buffer[0]<<24)&(0xFF000000))|((buffer[1]<<16)&(0xFF0000))|((buffer[2]<<8)&(0xFF00))|((buffer[3])&(0xFF));
        if (mylen>0){
            /*received length then send ack signal cokstart.*/  
            //send(mysocket, cokstart, 1, 0);   
			sendto(mysocket,cokstart,1,0,(struct sockaddr *)&addr,addr_len);
        }

        /* receive one frame of jpg data */
        j++;
        std::cout << " receiving one frame of jpg data # " << j << " image size=" << mylen << "bytes\n";
        vec.clear();

        while(mylen) 
        {
			
            //size=recv(mysocket,buffer,mylen,0);
			std::cout << " start to recv left msg size=" << mylen << "bytes\n"; 
			if (mylen>32*1024)
			{
				size=recvfrom(mysocket,buffer,32*1024,0,(struct sockaddr *)&addr,(socklen_t*)&addr_len);
			}
			else
			{
				size=recvfrom(mysocket,buffer,mylen,0,(struct sockaddr *)&addr,(socklen_t*)&addr_len);
			}
            
            /* put char values to vector */
            for(int i = 0 ; i < size ; i ++)  
            {  
                vec.push_back(buffer[i]);
            }
            mylen = mylen-size; 
        }

		std::cout << " received one frame of jpg data\n";
        
        /* decode jpg data */
        img_decode = cv::imdecode(vec, CV_LOAD_IMAGE_COLOR); 

        /* display the jpg in windows */
        cv::namedWindow("pic",cv::WINDOW_NORMAL);
        cv::setWindowProperty("pic",cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
        if(!img_decode.empty()){
            imshow("pic",img_decode);     
        }
        else
        {
            std::cout << " receive empty frame of jpg data # " << j << "\n";
        }
        
        /* reflash display window in every 33ms */
        cvWaitKey(10);
        
        /* send ack signal cok to tell that I'm ready to handler next frame */
        char cok[ 1 ]={0x55};
        //send(mysocket, cok, 1, 0);
		sendto(mysocket,cok,1,0,(struct sockaddr *)&addr,addr_len);
    }
}

