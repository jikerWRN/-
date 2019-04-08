#include <highgui.h>
#include <cv.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace cv;
using namespace std;

void marker_detect(IplImage* src,CvSeq* objects,int i,double scale);
int fixTimes(void);
void registerNew(CvSeq* faces,IplImage* frameCopy,int num,CvCapture* capture,double scale,int* ptimes);
int detect(void);


