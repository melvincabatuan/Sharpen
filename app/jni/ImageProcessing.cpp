#include "com_cabatuan_sharpen_MainActivity.h"
#include <android/log.h>
#include <android/bitmap.h>

#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;

#define  LOG_TAG    "Sharpen"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)



Mat *pSharp = NULL;
double t;

// Around 2 to 8 ms
void sharpen(const cv::Mat &image, cv::Mat &result) {
 
  result.create(image.size(), image.type());

  for (int j= 1; j<image.rows-1; j++) { // for all rows
      // (except first and last)
      const uchar* previous = image.ptr<const uchar>(j-1); // previous row
      const uchar* current = image.ptr<const uchar>(j); // current row
      const uchar* next = image.ptr<const uchar>(j+1); // next row
      uchar* output= result.ptr<uchar>(j); // output row
    
      for (int i=1; i<image.cols-1; i++) {
          *output++= cv::saturate_cast<uchar>(5*current[i]-current[i-1]-current[i+1]-previous[i]-next[i]);
      }
   }
   
   result.row(0).setTo(cv::Scalar(0));
   result.row(result.rows-1).setTo(cv::Scalar(0));
   result.col(0).setTo(cv::Scalar(0));
   result.col(result.cols-1).setTo(cv::Scalar(0));
}


// Around 15 to 20+ ms
void sharpen2D(const cv::Mat &image, cv::Mat &result) {
 
   cv::Mat kernel(3,3,CV_32F,cv::Scalar(0));
 
   kernel.at<float>(1,1)= 5.0;
   kernel.at<float>(0,1)= -1.0;
   kernel.at<float>(2,1)= -1.0;
   kernel.at<float>(1,0)= -1.0;
   kernel.at<float>(1,2)= -1.0;
 
   cv::filter2D(image,result,image.depth(),kernel);
}



/*
 * Class:     com_cabatuan_sharpen_MainActivity
 * Method:    process
 * Signature: (Landroid/graphics/Bitmap;[B)V
 */
JNIEXPORT void JNICALL Java_com_cabatuan_sharpen_MainActivity_process
  (JNIEnv *pEnv, jobject clazz, jobject pTarget, jbyteArray pSource){

   AndroidBitmapInfo bitmapInfo;
   uint32_t* bitmapContent; // Links to Bitmap content

   if(AndroidBitmap_getInfo(pEnv, pTarget, &bitmapInfo) < 0) abort();
   if(bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) abort();
   if(AndroidBitmap_lockPixels(pEnv, pTarget, (void**)&bitmapContent) < 0) abort();

   /// Access source array data... OK
   jbyte* source = (jbyte*)pEnv->GetPrimitiveArrayCritical(pSource, 0);
   if (source == NULL) abort();

   /// cv::Mat for YUV420sp source and output BGRA 
    Mat srcGray(bitmapInfo.height, bitmapInfo.width, CV_8UC1, (unsigned char *)source);
    Mat mbgra(bitmapInfo.height, bitmapInfo.width, CV_8UC4, (unsigned char *)bitmapContent);

/***********************************************************************************************/
    /// Native Image Processing HERE... 
    LOGI("Starting native image processing...");

    if(pSharp == NULL)
         pSharp = new Mat(srcGray.size(), srcGray.type());
    
    Mat sharp = *pSharp;
    
    t = (double)getTickCount(); 

    sharpen2D( srcGray, sharp); 

    t = 1000*((double)getTickCount() - t)/getTickFrequency();
    LOGI("Sharpening took %0.2f ms.", t);

    cvtColor(sharp, mbgra, CV_GRAY2BGRA);

    LOGI("Successfully finished native image processing..."); 
/************************************************************************************************/ 
   
   /// Release Java byte buffer and unlock backing bitmap
   pEnv-> ReleasePrimitiveArrayCritical(pSource,source,0);
   if (AndroidBitmap_unlockPixels(pEnv, pTarget) < 0) abort();

}
