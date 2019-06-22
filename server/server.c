#include "opencv2/highgui/highgui.hpp"
#include "opencv/cv.h"
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE sizeof(struct sockaddr_in)

void catcher(int sig);
int colorsockfd, redsockfd, sockfd;
struct sockaddr_in server = {AF_INET, 9009, INADDR_ANY};
void tcp_server(void);
void wait(float seconds);

void* Func_Client_Color(void *arg);
void* Func_Client_Red(void *arg);

void* Func_Super_Color(void *arg);
void* Func_Super_Red(void *arg);

void quit(const char *msg, int retval);

IplImage *color_img, *red_img;
pthread_mutex_t color_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t red_mutex = PTHREAD_MUTEX_INITIALIZER;
int key;
int color_is_data_ready = 0;
int red_is_data_ready = 0;

pthread_t Th_Client_Color, Th_Client_Red;
pthread_t Th_SuperClient_Color, Th_SuperClient_Red;


int main(int argc, char ** argv)
{

	int th_num = 0;
	void *thread_result;

	//** width, height **//
	int width = 320; 
	int height = 240;

	color_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	red_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	cvZero(color_img);
	cvZero(red_img);

//	printf("img->imageSize = %d\n ", img->imageSize);
//	printf("img->widthStep = %d\n", img->widthStep);
//	printf("img->height = %d\n", img->height);
//	printf("img->width = %d\n", img->width);

//	cvNamedWindow("Server", CV_WINDOW_AUTOSIZE);

	tcp_server();

	//	Thread_Client_Color
	if( (colorsockfd = accept(sockfd, NULL, NULL)) == -1 )
		perror(" 1st accept call failed");
	printf(" First accept call success...\n", th_num);

	if( pthread_create(&Th_Client_Color, NULL, Func_Client_Color, NULL))
		quit("Th_Client_Color Failed...", 1);

	//	Thread_Client_Red
	if( (redsockfd = accept(sockfd, NULL, NULL)) == -1 )
                perror(" Second accept call failed");
        printf(" Second accept call success...\n", th_num);

        if( pthread_create(&Th_Client_Red, NULL, Func_Client_Red, NULL))
                quit("Th_Client_Red Failed...", 1);



	if( pthread_join(Th_Client_Color, &thread_result) )
		quit("Thread_Client_Color join failed...",1);
	if(thread_result == NULL)
	{
		//return 0;
		if( pthread_join(Th_Client_Color, &thread_result) )
          	      quit("Thread_Client_Color join failed...",1);
		if(thread_result == NULL){
			close(colorsockfd);
			close(redsockfd);
			return 0;
		}
		else{
			close(colorsockfd);
			close(redsockfd);
			return 1;
		}
	}
	else{
		close(colorsockfd);
		close(redsockfd);
		return 1;
	}
} // end of main();

//cvDestroyWindow("Server");



			//////////////////////////////
			//// 	    Client	  ////
			//////////////////////////////

void* Func_Client_Color(void *arg)
{
	printf("Func_Client_Color Thread Start ! \n");
	if(pthread_create(&Th_SuperClient_Color, NULL, Func_Super_Color, NULL))
		quit("Th_SuperClient_Color create failed", 1);

        //fprintf(stdout, "Press 'q' to quit \n\n");
        cvNamedWindow("Server_Color", CV_WINDOW_AUTOSIZE);

        while(key != 'q')
        {
                pthread_mutex_lock(&color_mutex);
                //printf(" cvShowImage mutex IN  \n");
                if(color_is_data_ready)
                {
                        cvShowImage("Server_Color", color_img);
                        color_is_data_ready = 0;
                }
                pthread_mutex_unlock(&color_mutex);
                //printf(" cvShowImage mutex OUT  \n");

                key = cvWaitKey(10);
        }

	if(pthread_cancel(Th_SuperClient_Color))
		 quit("pthread_cancel failed", 1);
        cvDestroyWindow("Server_Color");
        quit(NULL, 0);

}

void* Func_Client_Red(void *arg)
{
	printf("Func_Client_Red Thread Start ! \n");
	if(pthread_create(&Th_SuperClient_Red, NULL, Func_Super_Red, NULL))
		 quit("Th_SuperClient_Red create failed", 1);

	//fprintf(stdout, "Press 'q' to quit \n\n");
        cvNamedWindow("Server_Red", CV_WINDOW_AUTOSIZE);

        while(key != 'q')
        {
                pthread_mutex_lock(&red_mutex);
                //printf(" cvShowImage mutex IN  \n");
                if(red_is_data_ready)
                {
                        cvShowImage("Server_Red", red_img);
                        red_is_data_ready = 0;
                }
                pthread_mutex_unlock(&red_mutex);
                //printf(" cvShowImage mutex OUT  \n");

                key = cvWaitKey(10);
        }

        if(pthread_cancel(Th_SuperClient_Red))
                 quit("pthread_cancel failed", 1);
        cvDestroyWindow("Server_Red");
        quit(NULL, 0);
}


			//////////////////////////////
			////	  Super Client	  ////
			//////////////////////////////


void* Func_Super_Color(void *arg)
{
	 printf("-------Func_Super_Color Thread Start ! \n");
        int imgsize = color_img->imageSize;
        char sockdata[imgsize];
        int i,j,k,bytes;

        while(1)
        {
                for(i=0; i<imgsize; i+=bytes)
                {
         if((bytes = recv(colorsockfd, sockdata + i, imgsize-i, 0)) == -1)
                                quit("recv failed", 1);
                }

                pthread_mutex_lock(&color_mutex);
                //printf("mutex in \n");
                for(i=0, k=0;i< color_img->height; i++)
                {
                        for(j=0;j< color_img->widthStep; j++)
                        {
     ((uchar*)(color_img->imageData + i*color_img->widthStep))[j] = sockdata[k];
   			//((uchar*)(color_img->imageData + i*color_img->widthStep))[j+1] = sockdata[k+1];
          		//((uchar*)(color_img->imageData + i*color_img->widthStep))[j+2] = sockdata[k+2];
                  	//k+=3;
                        k++;
                        }
                }
                color_is_data_ready = 1;
                pthread_mutex_unlock(&color_mutex);
                //printf("mutex out \n");

                pthread_testcancel();
                usleep(100);
        }
}

void* Func_Super_Red(void *arg)
{
	printf("-------Func_Super_Red Thread Start ! \n");
        int imgsize = red_img->imageSize;
        char sockdata[imgsize];
        int i,j,k,bytes;

        while(1)
        {
                for(i=0; i<imgsize; i+=bytes)
                {
         	if((bytes = recv(redsockfd, sockdata + i, imgsize-i, 0)) == -1)
                	quit("recv failed", 1);
                }

                pthread_mutex_lock(&red_mutex);
                //printf("mutex in \n");
                for(i=0, k=0;i< red_img->height; i++)
                {
                        for(j=0;j< red_img->widthStep; j++)
                        {
     			((uchar*)(red_img->imageData + i*red_img->widthStep))[j] = sockdata[k];
                        k++;
                        }
                }
                red_is_data_ready = 1;
                pthread_mutex_unlock(&red_mutex);
                //printf("mutex out \n");
                pthread_testcancel();
                usleep(100);
        }

}


void tcp_server(void)
{

	if( ( sockfd = socket(AF_INET, SOCK_STREAM, 0 )) == -1)
        {
                perror("socket call failed");
                exit(1);
        }
        printf("sockcet call success...\n");

        if( bind(sockfd, (struct sockaddr *) &server, SIZE) == -1 )
        {
                perror("bind call failed");
                exit(1);
        }
        printf("bind call success...\n");

        if( listen(sockfd, 2) == -1 )
        {
                perror("listen call failed");
                exit(1);
        }
        printf("listen call success...\n");

}


void wait(float seconds)
{
	clock_t endwait;
	endwait = clock() + seconds * CLOCKS_PER_SEC;
	while(clock() < endwait)	{}
}


void catcher(int sig)
{
//	close(newsockfd);
	exit(0);
}

void quit(const char* msg, int retval)
{
	if(retval == 0)
	{
		fprintf(stdout, (msg == NULL ? "" : msg));
		fprintf(stdout, "\n");
	} else {
		fprintf(stderr, (msg == NULL ? "" : msg));
		fprintf(stderr, "\n");
	}
//	if(sock) close(sock);
	if(color_img) cvReleaseImage(&color_img);

	pthread_mutex_destroy(&color_mutex);
	pthread_mutex_destroy(&red_mutex);

	exit(retval);
}



