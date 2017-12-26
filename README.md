# Image Processing for Fire Detection
The objective of this project is to build the high performance system that can detect fire object by using image data from both visible light and infrared light. 

- Capstone Design Project
- 2014.03 ~ 2014.06
- 지도교수: 김윤관, 팀원: 김영수, 권지헌, 한지웅, 홍단비

The report for this project is in [**here**](https://1drv.ms/w/s!AllPqyV9kKUrgXGdeFJ06NZIc4FE)


## Environment
* 3 Raspberry pi board ( 2 client, 1 server )
* 1 Infrared pi camera, 1 pi camera
* OpenCV Library
* Linux

## Algorithm
* labeling 
* time-series dynamic filter ( the change amount of centroid of each object, whether or not the centroid from visible light coincides with the centroid from infrared light, whether or not RGB and YCbCr of the object coincide with the fire model ) 
* the structure of client-server to imitate the system which can process both visible light and infrared light

## Conclusion
The main point is to use the different property between infrared image and visible image. Unlike visible image, infrared image is processed by detecting heat wave of objects. In other words, the visible image emphasizes the color of objects whereas the infrared image emphsizes the light of objects. Thus, I've implemented something like hybrid version, in order to enhance the performance. 

## Reivew
My hope was to build a embedded system so as to experience both software and hardware at the same time, before graduating school. Thus, I decided to build it as a capstone design project. Since I am lack of knowledge of hardware, I selected the hardware platform packaged with core and some peripheral devices, such as Raspberry pi or Arduino.
The goal of project was to build the high performance system that can detect fire object by using image data from both visible light and infrared light. Since we couldn't figure out the separation between **visible light** and **infrared light** in camera lens(i.e., hardware-level), we just imitate it as building server-client system (i.e., software-level).
The logic to detect fire object is a ***labeling*** algorithm and ***time-series dynamic filter*** being able to aware of the change amount of centroid of each object and whether or not the centroid from visible light coincides with the centroid from infrared light and whether or not RGB and YCbCr of the object coincide with the fire model.
The main point of project was to use the different property between infrared image and visible image. Unlike visible image, infrared image is processed by detecting heat wave of objects. In other words, the visible image emphasizes the color of objects whereas the infrared image emphasizes the light of objects. As a result, I've implemented something like a **hybrid version** which can enhance the performance to detect fire objects.
