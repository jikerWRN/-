#include "detect.h"

void marker_detect(IplImage* src,CvSeq* objects,int i,double scale)
{
	CvScalar colors = cvScalar(0,255,0);
	double alpha = 0.1;
	IplImage* img = cvCreateImage(cvGetSize(src),src->depth,src->nChannels);
	cvCopy(src,img,NULL);//创建img是为了叠加在src上实现透明效果

	//绘制方框
	CvRect* r = (CvRect*)cvGetSeqElem( objects, i );
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

	//将画出的东西都透明化
	cvAddWeighted(img,1-alpha,src,alpha,0,src);

}

int fixTimes(void)
{
    char* pBuf;
    FILE* pFileRead = fopen("C:\\Users\\WYF\\Desktop\\InTact\\Debug\\times.txt","r");
    if(pFileRead==NULL)
    {
        return 0;
    }
	
    fseek(pFileRead,0,SEEK_END);
    long filesize=ftell(pFileRead);
    fseek(pFileRead,0,SEEK_SET);

    pBuf=(char*)malloc(filesize+1);
    memset(pBuf,0,filesize+1);
    fread(pBuf,filesize+1,1,pFileRead);
    fclose(pFileRead);

    int times = atoi(pBuf);
	cout<<"已注册的人脸数: "<<times<<endl;
	//cout<<"The number of Registration is: "<<times<<endl;
    free(pBuf);
    pBuf=NULL;
    return times;
}

//faces是人脸的信息
//frameCopy是输入的帧
//num是第几个人的信息
void registerNew(CvSeq* faces,IplImage* frameCopy,int num,CvCapture* capture,double scale,int* ptimes)
{
	//face中是否有多人的人脸信息
	/*****************在图像中画出感兴趣的区域*********************/
	//在人脸中提取第几个人的信息
	//人脸信息是怎么排列的？
    CvRect* r = (CvRect*)cvGetSeqElem( faces, num );
    CvPoint center;
    int radius;
    center.x = cvRound((r->x + r->width*0.5)*scale);
    center.y = cvRound((r->y + r->height*0.5)*scale);
    radius = cvRound((r->width + r->height)*0.25*scale);
    int x=center.x-radius;
    int y=center.y-radius;
    int width = r->width*scale;
    int height = r->height*scale;
    CvRect rectROI = cvRect(x,  y,  width,  height );
    cvSetImageROI(frameCopy,rectROI);
	/********************************************/
	/*****************截取图像并存储***************************/
    IplImage* imgTemp[9];
    for(int i=0; i<9; i++)
    {
        imgTemp[i] = cvCreateImage(cvGetSize(frameCopy),frameCopy->depth,frameCopy->nChannels);
    }

    for(int i=0; i<9; i++)
    {
        cvWaitKey(10);
        frameCopy = cvQueryFrame(capture);
        cvSetImageROI(frameCopy,rectROI);
        cvCopy(frameCopy, imgTemp[i], NULL);
        cvShowImage("face_selected",imgTemp[i]);
    }

    cout<<"Do you want to keep the face_selected images? y/n"<<endl;
    char ans;
    cin>>ans;
    switch (ans)
    {
    case 'y':
    {
        fflush(stdin);//清空输入缓存
        int times = *ptimes;

        //存储样本图像并写入csv.txt文件
		//csv.ext文件存储的就是不同人的相片
        char buffer[128];
        //cvNamedWindow("face_selected");
        for(int i=0; i<9; i++)
        {

            sprintf(buffer,"C:\\Users\\WYF\\Desktop\\InTact\\Debug\\person%d\\%d.jpg",times,i);
			//在这里没有创建文件夹，我怀疑是他自己创建的
            cvSaveImage(buffer,imgTemp[i]);
            cvShowImage("face_selected",imgTemp[i]);

            char fileTemp[8]="csv.txt";
            char csvBuffer[64]="C:\\Users\\WYF\\Desktop\\InTact\\Debug\\";
            strcat(csvBuffer, fileTemp);
            FILE* pFileWrite = fopen(csvBuffer,"a");

            char labelName[20];
            sprintf(labelName,";%d\n",times);
            strcat(buffer,labelName);

            fwrite(buffer,strlen(buffer),1,pFileWrite);
            fclose(pFileWrite);
        }

        cout<<"Please enter the friend's information, separated by commas:"<<endl;

        char info[256];
        scanf("C:\\Users\\WYF\\Desktop\\InTact\\Debug\\person%[^\n]",info);

        char infoPath[64]="C:\\Users\\WYF\\Desktop\\InTact\\Debug\\info.txt";
        FILE* pFileWrite = fopen(infoPath,"a");
        char infoTemp[2]="\n";
        strcat(info,infoTemp);
        fwrite(info,strlen(info),1,pFileWrite);
        fclose(pFileWrite);
        fflush(stdin);//清空输入缓存

        cout<<"Registration is successful!"<<endl<<endl;;

        //cvWaitKey(0);
        for(int i=0; i<7; i++)
        {
            cvReleaseImage(&imgTemp[i]);//imgTemp是在本函数中创建并Copy自frameCopy的ROI而来的所以可以释放
        }
        cvResetImageROI(frameCopy);//frameCopy传进来的是指针
        //	cvReleaseImage(&frameCopy);//不能在子函数中对frameCopy释放，因为它是主函数中循环前创建的，循环中还要用到
        cvDestroyWindow("face_selected");
        (*ptimes)++;
        return;

    }
    case 'n':
    {
        fflush(stdin);//清空输入缓存
        for(int i=0; i<7; i++)
        {
            cvReleaseImage(&imgTemp[i]);//imgTemp是在本函数中创建并Copy自frameCopy的ROI而来的所以可以释放
        }
        cvResetImageROI(frameCopy);//frameCopy传进来的是指针
        //	cvReleaseImage(&frameCopy);//不能在子函数中对frameCopy释放，因为它是主函数中循环前创建的，循环中还要用到
        cvDestroyWindow("face_selected");
        return;

    }
    default:
    {
        fflush(stdin);//清空输入缓存
        for(int i=0; i<7; i++)
        {
            cvReleaseImage(&imgTemp[i]);//imgTemp是在本函数中创建并Copy自frameCopy的ROI而来的所以可以释放
        }
        cvResetImageROI(frameCopy);//frameCopy传进来的是指针
        //	cvReleaseImage(&frameCopy);//不能在子函数中对frameCopy释放，因为它是主函数中循环前创建的，循环中还要用到
        cvDestroyWindow("face_selected");
        cout<<"Illegal input!"<<endl;
        return;

    }
    }


}

//人脸识别
int detect(void)
{
	//显示已经注册好的人数
	//对times进行初始化
    int times = fixTimes();

	//加载训练好的分类器
	//在哪里训练好的？
	//我觉得这里不该用训练好的分类器，因为你执行到这里的时候，分类器还没有训练好
    const char* cascade_filename ="C:\\Users\\WYF\\Desktop\\InTact\\Debug\\haarcascade_frontalface_alt2.xml";

    CvHaarClassifierCascade* cascade = (CvHaarClassifierCascade*)cvLoad( cascade_filename, 0, 0, 0 );

    if( !cascade )
    {
        cout<<"Error: Can not load the .xml file of the Haar classifier!"<<endl;
        return -1;
    }

    //动态存储结构，用来存储人脸在图像中的位置，在使用前应使用函数cvClearMemStorage进行清空
    static	CvMemStorage* storage = cvCreateMemStorage(0);

    //读取摄像头实时采集的视频
	//capture指向的摄像头中的内容
    CvCapture* capture = cvCreateCameraCapture(0);
    if(capture==NULL)
    {
        cout<<"Error: Can not find any camera device!"<<endl;
        return -1;
    }

    IplImage* frame = NULL;//循环之后记得释放
    IplImage* frameCopy=NULL;//防止传输样本时带有方框而创建的帧副本
    CvSeq* faces=NULL;

    //循环开始了，注意while循环之前的量记得在循环之后正确的释放
    while(1)
    {
        frame = cvQueryFrame(capture);
        frameCopy = cvCreateImage(cvGetSize(frame),frame->depth,frame->nChannels);
        cvCopy(frame,frameCopy,NULL);

        if(!frame)//如果视频帧为NULL则退出读取视频帧的while循环
        {
            break;
        }

        double scale = 1.3;
        IplImage* gray = cvCreateImage( cvSize(frame->width,frame->height), 8, 1 );//循环结束时应在循环内释放
        cvSmooth(gray,gray,CV_BLUR,3,3,0);//高斯平滑
        IplImage* small_frame = cvCreateImage( cvSize( cvRound (frame->width/scale),cvRound (frame->height/scale)),8, 1 );//循环结束时应在循环内释放

        cvCvtColor( frame, gray, CV_BGR2GRAY );
        cvResize( gray, small_frame, CV_INTER_LINEAR );
        cvEqualizeHist( small_frame, small_frame );
        cvClearMemStorage( storage );

        if( cascade )
        {
            //double t = (double)cvGetTickCount();
            faces = cvHaarDetectObjects( small_frame, cascade, storage,1.1, 2, CV_HAAR_DO_CANNY_PRUNING,cvSize(30, 30));//通过调整最后两个参数的大小可以提高检测速度，这两个参数可以依据实际中人脸与摄像头的距离导致人脸在窗口中大小范围来做出实际调整
            //t = (double)cvGetTickCount() - t;

            //cout<<"detect time: "<<t/((double)cvGetTickFrequency()*1000)<<"ms"<<endl;
            //cout<<"当前检测到的人脸个数为："<<faces->total<<endl<<endl;

            for( int i = 0; i < (faces ? faces->total : 0); i++ )
            {
                /*画圆
                CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
                CvPoint center;
                int radius;
                center.x = cvRound((r->x + r->width*0.5)*scale);
                center.y = cvRound((r->y + r->height*0.5)*scale);
                radius = cvRound((r->width + r->height)*0.25*scale);
                cvCircle( frame, center, radius, colors[0], 2, 8, 0 );
                */

                //画方框
				marker_detect(frame,faces,i,scale);

            }
        }

        cvNamedWindow( "检测与注册", 1 );
        cvShowImage( "检测与注册", frame );

        cvReleaseImage( &gray );
        cvReleaseImage( &small_frame );

        //int c = cvWaitKey(1);
		//检验有没有键按下
	     int c = cvWaitKey(1);

		 //opencv中人脸检测并不是智能检测一个人脸，当有多个人脸存在时，会存储多张人脸的信息
		 //空格
        if(c==32)
        {
			//**********给检测窗口的人脸进行标号*******************/
            for( int i = 0; i < (faces ? faces->total : 0); i++ )
            {
                CvRect* r = (CvRect*)cvGetSeqElem( faces, i );

                int x = cvRound((r->x + r->width*0.5)*scale);
                int y = cvRound((r->y + r->height*0.5)*scale);

				//设置字体
                CvFont font;
                double hScale=1;
                double vScale=1;
                int lineWidth=1;
                cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC,hScale,vScale,0,lineWidth);
                //
				char buffer[2];
                sprintf(buffer,"%d",i);
                cvPutText(frame,buffer,cvPoint(x,y),&font, CV_RGB(0,255,0));
                cvShowImage("检测与注册", frame );

            }

            int d = cvWaitKey(0);

            switch (d)
            {
            case '0':
            {

                registerNew(faces,frameCopy,0,capture,scale,&times);
                break;

            }
            case '1':
            {
                registerNew(faces,frameCopy,1,capture,scale,&times);
                break;
            }
            case '2':
            {
                registerNew(faces,frameCopy,2,capture,scale,&times);
                break;
            }
            case '3':
            {
                registerNew(faces,frameCopy,3,capture,scale,&times);
                break;
            }
            case '4':
            {
                registerNew(faces,frameCopy,4,capture,scale,&times);
                break;
            }
            default:
            {
                cout<<"错误: The selected number does not appear in the detection result frame!"<<endl;
                break;
            }

            }//switch语句完结
        }//判断空格出现的if语句完结
		//tab键
        if (c == 9)
        {
            //cvReleaseImage( &frame );//不知道是何原因一释放frame，Debug编译后运行就报错
            cvReleaseImage(&frameCopy);
            break;
        }
		//esc键
        if (c == 27)
        {
            cvReleaseImage(&frameCopy);

            char timesPath[64]="C:\\Users\\WYF\\Desktop\\InTact\\Debug\\times.txt";
            FILE* pFileWrite = fopen(timesPath,"w");
            char timesBuffer[5];
            sprintf(timesBuffer,"%d",times);
            fwrite(timesBuffer,strlen(timesBuffer),1,pFileWrite);
            fclose(pFileWrite);

            cvReleaseCapture( &capture );
            cvDestroyAllWindows();

            return 1;
        }
    }//显示摄像头采集到的视频的while循环完结

    char timesPath[64]="C:\\Users\\WYF\\Desktop\\InTact\\Debug\\times.txt";
    FILE* pFileWrite = fopen(timesPath,"w");
    char timesBuffer[5];
    sprintf(timesBuffer,"%d",times);
    fwrite(timesBuffer,strlen(timesBuffer),1,pFileWrite);
    fclose(pFileWrite);

    cvReleaseCapture( &capture );
    cvDestroyAllWindows();

    return 0;
}
