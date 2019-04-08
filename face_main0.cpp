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

//��ȡ�ı����ݵ�������
char infoList[256][256];

void getInfo()
{

	FILE* pFileRead = fopen(".\\data\\info.txt","r");
	for(int i=0; i<256; i++)
	{
		 fgets(infoList[i],256,pFileRead);
	}
	fclose(pFileRead);
}

//��ȡcsv.ext�ļ�
static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';')
{
    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file) {
        string error_message = "�ļ�����Ч�������ļ���.";
		//string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    string line, path, classlabel;
    while (getline(file, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty()) {
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
	//scale�����ò���Դ������������ȶ�ԭͼ�������ţ�����ʱ��͸߶�����scale��Ȼ����Ŀ��Ļ�����ôĿ���λ����Ϣ��������ڴ�����ͼ�������Ҫ��ԭͼ�ж�Ŀ������ʶ�Ļ�����Ҫ��λ����Ϣ���Ż������ԭͼ����Ŀ��λ�õĿ�߳���scale������Ȼ���û�н��й��κ����ţ���scale=1����
	//alpha��������͸���ȣ�ȡֵ��ΧΪ0��1��ֵԽ��Խ͸��

	IplImage imgIpl =  imgMat;
	IplImage* src = &imgIpl;

	IplImage* img = cvCreateImage(cvGetSize(src),src->depth,src->nChannels);
	cvCopy(src,img,NULL);//����img��Ϊ�˵�����src��ʵ��͸��Ч��

	//���Ʒ���
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

	//�����Ľ�
	int d_corner = width/20+3;//d_corner��С�Ƕ�����Ŀ���ߵ�ˮƽ����ֱ���룬��3��Ϊ�˱�֤�ĸ��ǵ�С����Ŀ���̫С��ʱ�����ڿ�����
	int length_angle = width/7;
	//���Ͻǣ�����Ľǵ�����Ϊ��x,y��
	cvLine(img,cvPoint((x+d_corner),(y+d_corner)),cvPoint((x+d_corner+length_angle),(y+d_corner)),colors,1);
	cvLine(img,cvPoint((x+d_corner),(y+d_corner)),cvPoint((x+d_corner),(y+d_corner+length_angle)),colors,1);

	//���Ͻǣ�����Ľǵ�����Ϊ��x+width��y��
	cvLine(img,cvPoint((x+width-d_corner),(y+d_corner)),cvPoint((x+width-d_corner-length_angle),(y+d_corner)),colors,1);
	cvLine(img,cvPoint((x+width-d_corner),(y+d_corner)),cvPoint((x+width-d_corner),(y+d_corner+length_angle)),colors,1);

	//���½ǣ�����Ľǵ�����Ϊ��x��y+height��
	cvLine(img,cvPoint((x+d_corner),(y+height-d_corner)),cvPoint((x+d_corner),(y+height-d_corner-length_angle)),colors,1);
	cvLine(img,cvPoint((x+d_corner),(y+height-d_corner)),cvPoint((x+d_corner+length_angle),(y+height-d_corner)),colors,1);

	//���½�
	cvLine(img,cvPoint((x+width-d_corner),(y+height-d_corner)),cvPoint((x+width-d_corner),(y+height-d_corner-length_angle)),colors,1);
	cvLine(img,cvPoint((x+width-d_corner),(y+height-d_corner)),cvPoint((x+width-d_corner-length_angle),(y+height-d_corner)),colors,1);
	
	//������Ϣ��ʶ��
	int width_box = 155;
	int height_box = 25;
	cvLine(img,cvPoint(x,y+height),cvPoint((x-length_angle),(y+height+length_angle)),colors,2);//������

	CvPoint ** pt = new CvPoint*[1];
	pt[0] = new CvPoint[4];  //pt��һ��1*4������
	pt[0][0] = cvPoint((x-length_angle),(y+height+length_angle));
	pt[0][1] = cvPoint((x-length_angle+width_box),(y+height+length_angle));
	pt[0][2] = cvPoint((x-length_angle+width_box),(y+height+length_angle+height_box));
	pt[0][3] = cvPoint((x-length_angle),(y+height+length_angle+height_box));
	int npts[1];
	npts[0] = 4;
	cvFillPoly(img,pt,npts,1,colors);
	//cvRectangle(img,cvPoint((x-length_angle),(y+height+length_angle)),cvPoint((x-length_angle+width_box),(y+height+length_angle+height_box)),CV_RGB(255,255,255),1);//ԭ���㻭����Ϣ��ʶ��ı߽�

	//��Ϣ��ʶ����������ַ�����ʾ��ָ��λ��
	CvFont font;
	double hScale=0.6;
	double vScale=0.6;
	int lineWidth=1;
	cvInitFont(&font,CV_FONT_HERSHEY_TRIPLEX,hScale,vScale,0,lineWidth);

	CvPoint textPos = cvPoint((x-length_angle+5),(y+height+length_angle+height_box-5));//������+5��������-5��Ϊ��ʹ����λ�ڱ�ʶ�������

	getInfo();
	char xname[] = "��ȴ�...\n";
	if(flag == 0)//����������flag����Ϊ0��������Ϣ��ʶ������ʾUnknown
	{
		info = xname;
	}

	info[strlen(info)-1]='\0';//�����һ���ַ���ASCII��Ϊ0����������־��������ȥ���˻��з�
	cvPutText(img,info,textPos,&font,CV_RGB(255,255,255));

	//�������Ķ�����͸����
	cvAddWeighted(img,1-alpha,src,alpha,0,src);

}

int main(int argc, const char *argv[]) 
{

detect:	int detect_return=detect();
	while(!detect_return)
	{
		//����haar��������CSV�ļ�
		string fn_haar = "haarcascade_frontalface_alt2.xml"; 
		string fn_csv = ".\\data\\csv.ext";
		int deviceId = 0;
		// ��������ͼ��ͱ�ǩ
		vector<Mat> images;
		vector<int> labels;
		// ��ȡ����
		try
		{
			read_csv(fn_csv, images, labels);
		} 
		catch (cv::Exception& e) 
		{
			cerr << "���ļ����� \"" << fn_csv << "\". Reason: " << e.msg << endl;
			exit(1);
		}
	
		//�������
		CascadeClassifier haar_cascade;
		haar_cascade.load(fn_haar);
		VideoCapture cap(deviceId);
		if(!cap.isOpened())
		{
        cerr << "Capture Device ID " << deviceId << "cannot be opened." << endl;
        return -1;
		}
	
		// ����һ��FaceRecognizer�����ĵ���ͼƬѵ��
		Ptr<FaceRecognizer> model = createLBPHFaceRecognizer(1,8,8,8,100);
		model->train(images, labels);

		// ����Ƶ�豸��ȡ��ǰ֡
		Mat frame;
		for(;;) 
		{
			cap >> frame;
			// ���Ƶ�ǰ֡
			Mat original = frame.clone();
			// ת����ǰ֡Ϊ�Ҷ�ͼ
			Mat gray;
			cvtColor(original, gray, CV_BGR2GRAY);
		
			vector< Rect_<int> > faces;
			
			haar_cascade.detectMultiScale(gray, faces);

			for(int i = 0; i < faces.size(); i++) 
			{
				Rect face_i = faces[i];
				Mat face = gray(face_i);
				// ִ��Ԥ��
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
			
			imshow("����ͨѶ¼", original);
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