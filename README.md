# Image_processing_for_fire_detection

### Capstone Design
- 2014.03 ~ 2014.06
- 지도교수 김윤관

### Environment
* 3 Raspberry pi board ( 2 client, 1 server )
* 1 Infrared pi camera, 1 pi camera
* OpenCV Library
* Linux

### Report
<a href="https://1drv.ms/w/s!AllPqyV9kKUrgXGdeFJ06NZIc4FE">Fire Detection Report</a>

### The Goal
To build the high performance system that can detect fire object by using image data from both visible light and infrared light. 

### Algorithm
* labeling 
* time-series dynamic filter ( the change amount of centroid of each object, whether or not the centroid from visible light coincides with the centroid from infrared light, whether or not RGB and YCbCr of the object coincide with the fire model ) 
* the structure of client-server to imitate the system which can process both visible light and infrared light

### Conclusion
The main point is to use the different property between infrared image and visible image. Unlike visible image, infrared image is processed by detecting heat wave of objects. In other words, the visible image emphasizes the color of objects whereas the infrared image emphsizes the light of objects. Thus, I've implemented something like hybrid version, in order to enhance the performance. 
