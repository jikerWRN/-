#include <opencv2/core/core.hpp>
#include <opencv2/contrib/contrib.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include "detect.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace cv;
using namespace std;

//获取文本数据到数组中
char infoList[256][256];
//
void getInfo()
{

	FILE* pFileRead = fopen("C:\\Users\\WYF\\Desktop\\InTact\\Debug\\info.txt","r");
	for(int i=0; i<256; i++)
	{
		//每一个数组读到的数据都一样吗
		 fgets(infoList[i],256,pFileRead);
	}
	fclose(pFileRead);
}

//读取csv.ext文件   真的是ext吗？武宏师兄？
static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';')
{
    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file) {
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    string line, path, classlabel;
    while (getline(file, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty()) {

			//cout << "path :" << path << endl;
            images.push_back(imread(path, 0));
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
}

void marker_recognize(Mat imgMat,Rect objects,char* info,int flag)
{
	CvScalar colors = cvScalar(255,170,86);
	double scale = 1;
	double alpha = 0.1;
	//scale——该参数源自于如果你事先对原图进行缩放（缩放时宽和高都除以scale）然后检测目标的话，那么目标的位置信息就是相对于处理后的图像，如果你要在原图中对目标作标识的话，还要将位置信息缩放回相对于原图（将目标位置的宽高乘以scale），当然如果没有进行过任何缩放，令scale=1即可
	//alpha——设置透明度，取值范围为0到1，值越大越透明

	IplImage imgIpl =  imgMat;
	IplImage* src = &imgIpl;

	IplImage* img = cvCreateImage(cvGetSize(src),src->depth,src->nChannels);
	cvCopy(src,img,NULL);//创建img是为了叠加在src上实现透明效果

	//绘制方框
	//CvRect* r = (CvRect*)cvGetSeqElem( objects, i );
	CvRect* r;
	r->x = objects.tl().x;
	r->y = objects.tl().y;
	r->width = objects.size().width;
	r->height = objects.size().height;

	CvPoint center;
	int radius;

	center.x = cvRound((r->x + r->width*0.5)*scale);
	center.y = cvRound((r->y + r->height*0.5)*scale);

	radius = cvRound((r->width + r->height)*0.25*scale);

	int x=center.x-radius;
	int y=center.y-radius;

	int width = r->width*scale;
	int height = r->height*scale;
	cvRectangle( img, cvPoint(x,y),cvPoint(x+width,y+height), colors, 3 );

	//绘制四角
	int d_corner = width/20+3;//d_corner是小角顶点与目标框边的水平或竖直距离，加3是为了保证四个角的小框在目标框太小的时候不至于看不见
	int length_angle = width/7;
	//左上角，方框的角点坐标为（x,y）
	cvLine(img,cvPoint((x+d_corner),(y+d_corner)),cvPoint((x+d_corner+length_angle),(y+d_corner)),colors,1);
	cvLine(img,cvPoint((x+d_corner),(y+d_corner)),cvPoint((x+d_corner),(y+d_corner+length_angle)),colors,1);

	//右上角，方框的角点坐标为（x+width，y）
	cvLine(img,cvPoint((x+width-d_corner),(y+d_corner)),cvPoint((x+width-d_corner-length_angle),(y+d_corner)),colors,1);
	cvLine(img,cvPoint((x+width-d_corner),(y+d_corner)),cvPoint((x+width-d_corner),(y+d_corner+length_angle)),colors,1);

	//左下角，方框的角点坐标为（x，y+height）
	cvLine(img,cvPoint((x+d_corner),(y+height-d_corner)),cvPoint((x+d_corner),(y+height-d_corner-length_angle)),colors,1);
	cvLine(img,cvPoint((x+d_corner),(y+height-d_corner)),cvPoint((x+d_corner+length_angle),(y+height-d_corner)),colors,1);

	//右下角
	cvLine(img,cvPoint((x+width-d_corner),(y+height-d_corner)),cvPoint((x+width-d_corner),(y+height-d_corner-length_angle)),colors,1);
	cvLine(img,cvPoint((x+width-d_corner),(y+height-d_corner)),cvPoint((x+width-d_corner-length_angle),(y+height-d_corner)),colors,1);
	
	//绘制信息标识框
	int width_box = 155;
	int height_box = 25;
	cvLine(img,cvPoint(x,y+height),cvPoint((x-length_angle),(y+height+length_angle)),colors,2);//连接线

	CvPoint ** pt = new CvPoint*[1];
	pt[0] = new CvPoint[4];  //pt是一个1*4的数组
	pt[0][0] = cvPoint((x-length_angle),(y+height+length_angle));
	pt[0][1] = cvPoint((x-length_angle+width_box),(y+height+length_angle));
	pt[0][2] = cvPoint((x-length_angle+width_box),(y+height+length_angle+height_box));
	pt[0][3] = cvPoint((x-length_angle),(y+height+length_angle+height_box));
	int npts[1];
	npts[0] = 4;
	cvFillPoly(img,pt,npts,1,colors);
	//cvRectangle(img,cvPoint((x-length_angle),(y+height+length_angle)),cvPoint((x-length_angle+width_box),(y+height+length_angle+height_box)),CV_RGB(255,255,255),1);//原打算画出信息标识框的边界

	//信息标识，将传入的字符串显示在指定位置
	CvFont font;
	double hScale=0.6;
	double vScale=0.6;
	int lineWidth=1;
	cvInitFont(&font,CV_FONT_HERSHEY_TRIPLEX,hScale,vScale,0,lineWidth);

	CvPoint textPos = cvPoint((x-length_angle+5),(y+height+length_angle+height_box-5));//横坐标+5，纵坐标-5是为了使文字位于标识框的中央

	getInfo();
	char xname[] = "Recognizing...\n";
	if(flag == 0)//如果输入参数flag设置为0，则在信息标识框上显示Unknown
	{
		info = xname;
	}

	info[strlen(info)-1]='\0';//让最后一个字符的ASCII码为0，即结束标志，这样就去掉了换行符
	cvPutText(img,info,textPos,&font,CV_RGB(255,255,255));

	//将画出的东西都透明化
	cvAddWeighted(img,1-alpha,src,alpha,0,src);

}

int main(int argc, const char *argv[]) 
{

detect:	int detect_return=detect();
	//如果检测出东西时，执行人脸识别操作
	/********************人脸识别***********************/
	while(!detect_return)
	{
		/****************是在训练分类器吗？**************************/
		//加载haar分类器和CSV文件
		//csv是txt文件，其中究竟放的是什么东西？
		//文件名
		string fn_haar = "C:\\Users\\WYF\\Desktop\\InTact\\Debug\\haarcascade_frontalface_alt2.xml"; 
		string fn_csv = "C:\\Users\\WYF\\Desktop\\InTact\\Debug\\csv.txt";
		int deviceId = 0;
		// 容器保存图像和标签
		//怎么将图片存入容器中？
		vector<Mat> images;
		vector<int> labels;
		// 读取数据
		try
		{
			//csv.txt中放的是什么数据啊？为什么可以同时填入两个容器，而且图片不是在data文件夹中放着吗？
			read_csv(fn_csv, images, labels);
		} 
		catch (cv::Exception& e) 
		{
			cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
			exit(1);
		}

		//人脸检测
		CascadeClassifier haar_cascade;
		haar_cascade.load(fn_haar);
		VideoCapture cap(deviceId);
		if(!cap.isOpened())
		{
        cerr << "Capture Device ID " << deviceId << "cannot be opened." << endl;
        return -1;
		}
	
		// 创建一个FaceRecognizer并用拍到的图片训练
		Ptr<FaceRecognizer> model = createLBPHFaceRecognizer(1,8,8,8,100);
		model->train(images, labels);

		// 从视频设备获取当前帧
		Mat frame;
		for(;;) 
		{
			cap >> frame;
			// 复制当前帧
			Mat original = frame.clone();
			// 转换当前帧为灰度图
			Mat gray;
			cvtColor(original, gray, CV_BGR2GRAY);
		
			vector< Rect_<int> > faces;
			
			haar_cascade.detectMultiScale(gray, faces);

			for(int i = 0; i < faces.size(); i++) 
			{
				Rect face_i = faces[i];
				Mat face = gray(face_i);
				// 执行预测
				int prediction = model->predict(face);
				if(prediction >= 0)
				{
					marker_recognize(original,face_i,infoList[prediction],1);
				}
				else
				{
					marker_recognize(original,face_i,infoList[prediction],0);
				}
			}
			
			imshow("智能通讯录V1.0", original);
			char key = (char) waitKey(1);
			if(key == 27)
			{
				cvDestroyAllWindows();
				goto end;
			}
			if(key == 9)
			{
				cvDestroyAllWindows();
				goto detect;
			}
		}
	detect_return = 1;
	end:       break;
	}
    return 0;
}