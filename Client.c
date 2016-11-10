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
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE sizeof(struct sockaddr_in)

#define MAXX(x,y)	((x)>(y)?(x):(y))
#define MINN(x,y)	((x)<(y)?(x):(y))

#define CENTER(min,max)		((min)+(max)) / 2

#define WIDTH_C		320
#define HEIGHT_C	240

#define MAX_LABEL	1000
#define EQ_INTV		5

#define TH_B	200
#define TH_G	210
#define TH_R	220

#define TEST_P		1
#define MAP_TEST	0


void catcher(int sig);
int newsockfd, sockfd;
struct sockaddr_in server = {AF_INET, 9009};

void wait(float seconds);
void tcp_client(void);

int glabel, glo_i, debugC,  i,j, g,b, m,n, p,q;
int label = 0;
int labeling[HEIGHT_C][WIDTH_C];
int maxL,minL;
int min_axis[MAX_LABEL][2];
int max_axis[MAX_LABEL][2];

CvCapture *capture;
IplImage *img0;
IplImage *img1;

int is_data_ready = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *streamClient(void *arg);
void quit(const char *msg, int retval);

void printvalue(FILE *rfp, FILE *gfp, FILE *bfp, IplImage *frame);
void Threshold(IplImage *img);
void Labeling(IplImage *img);
void frame_init(void);
void debugging(void);
void pre_image_set(IplImage *img);
void print_labeling(void);
void print_yellow_rect(IplImage *img);


int main(int argc, char ** argv)
{

		pthread_t thread_s;
		int key;
        server.sin_addr.s_addr = inet_addr("127.0.0.1");

        capture = cvCaptureFromCAM(0);
        if(!capture)    quit("cvCapture failed", 1);

		for(i=0;i<MAX_LABEL;i++)
		{
			min_axis[i][0] = WIDTH_C-1;
			min_axis[i][1] = HEIGHT_C-1;
		}

        cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, WIDTH_C);
        cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, HEIGHT_C);

		img0 = cvQueryFrame(capture);
		img1 = cvCreateImage(cvGetSize(img0), IPL_DEPTH_8U, 3);
		cvZero(img1);


		//	printf("img1->imageSize = %d\n", img1->imageSize);
		//	printf("img1->widthStep = %d\n", img1->widthStep);
		//	printf("img1->height = %d\n", img1->height);
		//	printf("img1->width = %d\n", img1->width);

		cvNamedWindow("Client", CV_WINDOW_AUTOSIZE);
		//cvNamedWindow("Unblurred", CV_WINDOW_AUTOSIZE);
		//cvNamedWindow("Client_Test", CV_WINDOW_AUTOSIZE);


		fprintf(stdout, "width: %d\nheight: %d\n\n", img0->width, img0->height);
		fprintf(stdout, "Press 'q' to quit \n\n");

		// Connect to Server
		//tcp_client();

		// Start Thread...
		//if(pthread_create(&thread_s, NULL, streamClient, NULL))    quit("pthread_create failed", 1);

		while(key!='q')
		{
			wait(0.1);
			img0 = cvQueryFrame(capture);
			if(!img0)	break;
			img0->origin = 0;
			//cvFlip(img0, img0, 1);
			cvCopy(img0, img1, NULL);

////////////////////////////////////////////////////////////////////////
/////////// 		Image Processing 		////////////////
////////////////////////////////////////////////////////////////////////

		pthread_mutex_lock(&mutex);

		// Step 1 : blurring
		cvSmooth(img1, img1, CV_BLUR,3,0,0,0);

		// Step 2 : Threasholding
		Threshold(img1);

		// Step 3 : Labeling
		//pre_image_set(img1);

		//if(debugC==100 || debugC==200 || debugC==300 || debugC==400 || debugC == 500)
		Labeling(img1);
		//print_labeling();
		//print_yellow_rect(img1);

		debugC++;
//////////////////////////////////////////////////////////////////////////////////
	//		pthread_mutex_lock(&mutex);
//			cvCvtColor(img0, img1, CV_BGR2RGB);

			is_data_ready = 1;
			pthread_mutex_unlock(&mutex);

		//	cvCvtColor(img1, img0, CV_YCrCb2RGB);


			//cvCvtColor(img0, img1, CV_YCrCb);
	//		cvShowImage("Client", img1);
			cvShowImage("Client", img1);

			//cvSmooth(img1, img1, CV_BLUR,3,0,0,0);
			//cvShowImage("Client_Test", img1);

			frame_init();
			key = cvWaitKey(10);
		}

		if(pthread_cancel(thread_s))	quit("pthread_cancel failed", 1);

		cvDestroyWindow("Client");
		cvDestroyWindow("Unblurred");
		quit(NULL, 0);

	return 0;
}


void frame_init(void)
{
		label = 0, maxL = 0, minL = 0;

		for(int i=0; i<HEIGHT_C;i++)
			for(int j=0; j<WIDTH_C;j++)
				labeling[i][j] = 0;


        for(int i=0;i<MAX_LABEL;i++)
        {
                min_axis[i][0] = WIDTH_C-1;
                min_axis[i][1] = HEIGHT_C-1;
        }

		for(int i=0;i<MAX_LABEL;i++)
		{
				max_axis[i][0] = 0;
				max_axis[i][1] = 0;
		}

}

void debugging(void)
{

}


void Labeling(IplImage *img)
{

		int i, j, r_ch, b_ch, g_ch, min_eq,  cnt, temp;
		int width, height, nchannels, step, offset;
		int *hash;
		int map_count_x = 0, map_count_y = 0;
		unsigned char** map;
		unsigned char** ptr;

		width = img->width;
		height = img->height;
		nchannels = img->nChannels;
		step = img->widthStep;

		int eq_tb1[MAX_LABEL][2] = {{0,}};

		map = (unsigned char**)malloc(sizeof(unsigned char*)*height);
		for(i=0;i<height;i++)
		{
			map[i] = (unsigned char*)malloc(sizeof(unsigned char)*width);
		}

		ptr = (unsigned char**)malloc(sizeof(unsigned char*)*height);
		for(i=0; i<height; i++)
		{
			ptr[i] = (unsigned char*)malloc(sizeof(unsigned char)*width);
		}

		// map, ptr init
		for(i=0;i<height;i++)
		{
			for(j=0;j<width;j++)
			{
				map[i][j]=0;
				ptr[i][j]=0;
			}
		}



		// Labeling Start : X, Y axis data store...

		for(i=0; i< height; i++)
		{
			for(j=0; j< width; j++)
			{
					offset = j*nchannels;

					// Labeling Start
					if(img->imageData[i*step+offset+0] >= TH_B)
					   //img->imageData[i*step+offset+1] >= TH_G &&
					   //img->imageData[i*step+offset+2] >= TH_R)
					{


						if(i==0 && j==0)	// Case 1 : (0,0)
						{
							label++;
							ptr[i][j] = label;
							eq_tb1[label-1][0] = label;
							eq_tb1[label-1][1] = label;
						}
						else if(i==0)	// Case 2 : (0,y)
						{
							if(ptr[i][j-1] != 0)
								ptr[i][j] = ptr[i][j-1];
							else
							{
								label++;
								ptr[i][j] = label;
								eq_tb1[label-1][0] = label;
								eq_tb1[label-1][1] = label;
							}
						}
						else if(j==0)	// Case 3 : (x,0)
						{
							if(ptr[i-1][0] != 0)
								ptr[i][j] = ptr[i-1][j];
							else
							{
								label++;
								ptr[i][j] = label;
								eq_tb1[label-1][0] = label;
								eq_tb1[label-1][1] = label;
							}
						}
						else	// Case 4 : Others...
						{
							if( (ptr[i-1][j] !=0) && (ptr[i][j-1] !=0 ) )
							{
								if(ptr[i-1][j] == ptr[i][j-1]) // if i-1 equal j-1
								{
									ptr[i][j] = ptr[i-1][j];
								}
								else // if i-1 not_equal j-1
								{
									maxL = MAXX(ptr[i-1][j], ptr[i][j-1]);
									minL = MINN(ptr[i-1][j], ptr[i][j-1]);

									ptr[i][j] = minL;

									min_eq = MINN(eq_tb1[maxL-1][1], eq_tb1[minL-1][1]);

									eq_tb1[(eq_tb1[maxL-1][1])-1][1] = min_eq;
									eq_tb1[maxL-1][1] = min_eq;
									eq_tb1[minL-1][1] = min_eq;

									//for(g=i;g>=0;g--)
									//{
									//	if(labeling[g][j]==maxL)
									//		labeling[g][j] = minL;
									//}

									//if( labeling[i-1][j] == maxL)
									//{
										//if(TEST_P)	printf("   lC-- x:%d ", i);
										//if(TEST_P)	printf(" y:%d / ",j);
										//lC--;
									//}

									//if( labeling[i-1][j] < labeling[i][j-1] )
									//	labeling[i][j-1] = minL;

								}
							}
							else if( ptr[i-1][j] !=0 )
							{
								ptr[i][j] = ptr[i-1][j];
							}
							else if( ptr[i][j-1] !=0 )
							{
								ptr[i][j] = ptr[i][j-1];
							}
							//else if( labeling[i-1][j-1] !=0 )
							//{
							//	labeling[i][j] = labeling[i-1][j-1];
							//}
							//else if( labeling[i-1][j] == 0 && labeling[i][j-1] == 0 && labeling[i-1][j-1] == 0 )
							else
							{
								//if(i!=159 && j!=119)
								//if(TEST_P)	printf("  else lC++ x:%d ", i);
								//if(TEST_P)	printf("y:%d /", j);
								label++;
								ptr[i][j] = label;
								eq_tb1[label-1][0] = label;
								eq_tb1[label-1][1] = label;
							}
						}
					}
			}
		}
		// Labeling End


		hash = (int*)malloc(sizeof(int)*(label));
		memset(hash,0,sizeof(int)*(label));

		for(int i=0;i<label;i++)
			hash[eq_tb1[i][1]-1] = eq_tb1[i][1];	// standard for eq_tb1[][1] to label
		cnt = 1;

		for(int i=0;i<label;i++)
			if(hash[i]!=0)
				hash[i] = cnt++;
		for(int i=0;i<label;i++)
			eq_tb1[i][1] = hash[eq_tb1[i][1]-1];	// reassignment to eq_tb1 base on cnt(new label numbering)


		for(int i=0;i<height;i++){
			for(int j=0;j<width;j++){
				if(ptr[i][j]!=0){

					temp = ptr[i][j];
					map[i][j] = eq_tb1[temp-1][1];
					if(i < min_axis[map[i][j]-1][0]){
						min_axis[map[i][j]-1][0] = i;
					}
					if(i > max_axis[map[i][j]-1][0]){
						max_axis[map[i][j]-1][0] = i;
					}
					if(j < min_axis[map[i][j]-1][1]){
						min_axis[map[i][j]-1][1] = j;
					}
					if(j > max_axis[map[i][j]-1][1]){
						max_axis[map[i][j]-1][1] = j;
					}
				}
			}
		}


		if(MAP_TEST)
		{
			printf("\n\n *****************  original map  ***************** \n");
			//for(i=height-1;i>=0;i--)
			//{
				//for(j=width-1;j>=0;j--)
				//{

			for(i=0;i<height;i++)
			{
				for(j=0;j<width;j++)
				{
					//map_count_x++;
					printf("%d", map[i][j]);

					if(map_count_x == 10)
					{
						//printf(".");
						map_count_x = 0;
					}
				}
				//map_count_y++;
				if(map_count_y == 10)
				{
					//printf("..");
					map_count_y = 0;
				}
				printf("\n");
			}
		}




		printf("\n================================  label number : %d ==============================\n", cnt-1);
		for(i=0;i<cnt-1;i++)
		{
			printf("label= %d   ", i+1);
			printf("    i  ");
			for(j=0;j<2;j++)
			{
				printf("min = %d  max = %d   ", min_axis[i][j], max_axis[i][j] );
			}
			printf("  j    ");
			printf("\n Center = (%d,%d)",CENTER(min_axis[i][1],max_axis[i][1]),CENTER(min_axis[i][0],max_axis[i][0]));
			printf("\n");
		}



/*
		for(j=0; j< img->height;j++)
        {
                for(i=0; i< img->width;i++)
                {
                        offset = i*nchannels;

			if(labeling[i][j] != 0)
			{
				if(i < min_axis[labeling[i][j]][0])
					min_axis[labeling[i][j]][0] = i;
				if(i >  max_axis[labeling[i][j]][0])
					max_axis[labeling[i][j]][0] = i;
				if(j < min_axis[labeling[i][j]][1])
					min_axis[labeling[i][j]][1] = j;
				if(j > max_axis[labeling[i][j]][1])
					max_axis[labeling[i][j]][1] = j;
			}

                }
        }
*/

/*
	// Filtering Start -> make like this -> min, max X = 0, Y = 0

	for(int i=1;i<=lC;i++)
		if( max_axis[i][0] == 0 && max_axis[i][1] == 0 ||
		    max_axis[i][0] == min_axis[lC][0] && max_axis[i][1] == min_axis[i][1]  )
		{
			min_axis[i][0]	= 0;
			min_axis[i][1] = 0;
			max_axis[i][0]	= 0;
			max_axis[i][1]	= 0;
		}
*/

		free(hash);
		for(i=0;i<height;i++)
			free(ptr[i]);
		free(ptr);

		for(i=0;i<height;i++)
			free(map[i]);
		free(map);

		glabel = cnt -1;

}



void Threshold(IplImage *img)
{
        int i, j, r_ch, b_ch, g_ch;
        int width, height, nchannels, step, offset;

        width = img->width;
        height = img->height;
        nchannels = img->nChannels;
        step = img->widthStep;


        for(i=0; i< height;i++)
        {
                for(j=0; j< width;j++)
                {
                        offset = j*nchannels;

						// Thresholding Value Setting
						if(//img->imageData[i*step+offset+0] <= TH_B &&
						   //img->imageData[i*step+offset+1] <= TH_G &&
						   img->imageData[i*step+offset+2] <= TH_R)
						{
							img->imageData[i*step+offset] = 0;
							img->imageData[i*step+offset+1] = 0;
							img->imageData[i*step+offset+2] = 0;

						}
                }
        }

	//printf("Threshold End\n");
}


void print_labeling(void)
{

	int offset;

	printf("\n\nlC : %d\n\n", label);

	int debuc=0;
	// Debugging
	for(int i=0; i<HEIGHT_C;i++)
    	{
        	for(int j=0; j<WIDTH_C;j++)
        	{
            		//offset = j*nchannels;
					//printf("%d", labeling[i][j]);
        	}
			printf("\n");
			debuc++;
		if(debuc%10 ==0)	printf(".");
 	}


	for(int i=1;i<=label;i++)
	{
		printf("Label Number : %d\n",i);
		printf("min X = %d   ",   min_axis[i][0] );
		printf("min Y = %d   \n", min_axis[i][1] );
		printf("max X = %d   ",   max_axis[i][0] );
		printf("max Y = %d   \n\n", max_axis[i][1] );


	}
}



void print_yellow_rect(IplImage *img)
{
        int i, j, r_ch, b_ch, g_ch;
        int width, height, nchannels, step, offset;

        width = img->width;
        height = img->height;
        nchannels = img->nChannels;
        step = img->widthStep;

/*
	//test
	for(i=0; i< img->height; i++)
		for(j=0; j< img->width; j++)
		{
			offset = j*nchannels;

			for(int labelN; labelN < glabel; labelN++)
				if( j == CENTER(min_axis[i][0],max_axis[i][0])	&&   i == CENTER(min_axis[i][1],max_axis[i][1]) )
				{
 					img->imageData[i*step+offset+0] = 0;
                                        img->imageData[i*step+offset+1] = 0;
                                        img->imageData[i*step+offset+2] = 0;

				}
		}
*/



		// Paint Yellow Rectangle
       for(i=0; i< img->height;i++)
            for(j=0; j< img->width;j++)
                {
                    offset = j*nchannels;

			for(int labelN=0; labelN < glabel; labelN++)
			{

				if(i == CENTER(min_axis[labelN][0],max_axis[labelN][0]))
				{
					for(int m = min_axis[labelN][1]; m <= max_axis[labelN][1]; m++)
					{
						offset = m*nchannels;

						img->imageData[i*step+offset+0] = 0;
						img->imageData[i*step+offset+1] = 0;
						img->imageData[i*step+offset+2] = 255;
					}
				}

				if(j == CENTER(min_axis[labelN][1],max_axis[labelN][0]))
				{
					for(int n = min_axis[labelN][0]; n <= max_axis[labelN][1]; n++)
                                        {
                                                offset = j*nchannels;

                                                img->imageData[ n *step+offset+0] = 0;
                                                img->imageData[ n *step+offset+1] = 0;
                                                img->imageData[ n *step+offset+2] = 255;
                                        }

				}
/*
				// at MAX Y
				if(i == max_axis[labelN][0])
				{
					for( int m = min_axis[labelN][1]; m <= max_axis[labelN][1]; m++ )
					{
						offset = m*nchannels;

						img->imageData[i*step+offset+0] = 0;
						img->imageData[i*step+offset+1] = 255;
						img->imageData[i*step+offset+2] = 255;
					}
				}

				// at MIN X
				if(j == min_axis[labelN][0])
				{
					for(int n=min_axis[labelN][1]; n<=max_axis[labelN][1]; n++)
					{
				                offset = j*nchannels;

						img->imageData[ n *step+offset+0] = 0;
						img->imageData[ n *step+offset+1] = 255;
						img->imageData[ n *step+offset+2] = 255;
					}
				}

				// at MAX X
				if(j == max_axis[labelN][0])
				{
					for(int n=min_axis[labelN][1]; n<=max_axis[labelN][1]; n++)
					{
				                offset = j*nchannels;

						img->imageData[ n *step+offset+0] = 0;
						img->imageData[ n *step+offset+1] = 255;
						img->imageData[ n *step+offset+2] = 255;
					}
				}
*/

			}
		}



}




void pre_image_set(IplImage *img)
{

        int i, j, r_ch, b_ch, g_ch;
        int width, height, nchannels, step, offset;

        width = img->width;
        height = img->height;
        nchannels = img->nChannels;
        step = img->widthStep;


		for(j=0; j< img->height-EQ_INTV;j++)
        {
            for(i=0; i< img->width-EQ_INTV;i++)
            {
                offset = i*nchannels;

				for(m=0; m<=EQ_INTV;m++)
				{
					for(n=0; n<=EQ_INTV;n++)
					{
						if(//img->imageData[j*step+offset+0] >= TH_B &&
						   //img->imageData[j*step+offset+1] >= TH_G &&
						   img->imageData[j*step+offset+2] >= TH_R)
						{
							for(p=0;p<=m;p++)
							{
								for(q=0;q<=n;q++)
								{
									offset = (i+n-q)*nchannels;
									
									// equal pre_pixel to now_pixel
									img->imageData[(j+m-p)*step+offset] = img->imageData[j*step+offset+0];
									img->imageData[(j+m-p)*step+offset+1] = img->imageData[j*step+offset+1];
									img->imageData[(j+m-p)*step+offset+2] = img->imageData[j*step+offset+2];
								}
							}
						}
					}
				}  
            }
        }
		
}

void tcp_client(void)
{

	if(( sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket call failed");
		exit(1);
	}
	printf("socket call success...\n");

	if( connect(sockfd, (struct sockaddr *) &server, SIZE) == -1)
	{
		perror("connect call failed");
		exit(1);
	}
	printf("connect call success...\n");

}

void* streamClient(void* arg)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	int imgsize = img1->imageSize;
	int bytes, i;

	while(1)
	{
		pthread_mutex_lock(&mutex);
		if(is_data_ready)
		{
			bytes = send(sockfd, img1->imageData, imgsize, 0);
			//printf("send complete \n");
			is_data_ready = 0;
		}
		pthread_mutex_unlock(&mutex);

		//if(bytes != imgsize)
		//{
			//printf("bytes != imgsize... \n");
			//fprintf(stderr, "Connection closed \n");
			//close(sockfd);
			//if(connect(sockfd, (struct sockaddr *) &server, SIZE) == -1)
			//	quit("byte!=imgsize connection failed \n", 1);
		//}

		pthread_testcancel();
		usleep(100);
	}
}

void printvalue(FILE *rfp, FILE *gfp, FILE *bfp, IplImage *img)
{
	int i, j, r_ch, b_ch, g_ch;
	int width, height, nchannels, step, offset;

	width = img->width;
	height = img->height;
	nchannels = img->nChannels;
	step = img->widthStep;

	for(j=0;img->height;j++)
	{
		uchar *data = (uchar*)(img->imageData + j*step);

		for(i=0; i< img->width;i++)
		{
			offset = i*nchannels;
			b_ch = data[offset];
			g_ch = data[offset+1];
			r_ch = data[offset+2];

			fprintf(rfp, "%4d", r_ch);
			fprintf(gfp, "%4d", g_ch);
			fprintf(bfp, "%4d", b_ch);
		}
		fprintf(rfp,"\n");
		fprintf(gfp,"\n");
		fprintf(bfp,"\n");
	}
	//fprintf(rfp,"\n\n");
	//fprintf(rfp,"\n\n");
	//fprintf(rfp,"\n\n");

}


void wait(float seconds)
{
	clock_t endwait;
	endwait = clock() + seconds * CLOCKS_PER_SEC;
	while(clock() < endwait) {}
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
        if(sockfd) close(sockfd);
	if(capture)  cvReleaseCapture(&capture);
        if(img1) cvReleaseImage(&img1);

        pthread_mutex_destroy(&mutex);

        exit(retval);
}




