#include<opencv2/core.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>

#include<iostream>
#include<string>
using namespace cv;
using namespace std;

int nx[4] = { 0, 0, 1, 1 };
int ny[4] = { 0, 1, 0, 1 };
int main()
{
	////////////////        File open      /////////////////////////
	FILE *fp = fopen("Suzie_CIF_352x288.raw", "rb");
	if (fp == NULL) {
		cout << "Failed to open" << endl;
		return -1;
	}

	////////////////////////////////////////////////////////////////
	//															  //
	//					Process RGB raw file					  //
	//															  //
	////////////////////////////////////////////////////////////////
	unsigned char* src1 = new unsigned char[352 * 288];
	unsigned char* src2 = new unsigned char[352 * 288];
	unsigned char* src3 = new unsigned char[352 * 288];
	Mat bgr[3];

	//////////// get the imformation of R, G, B ////////////////////
	fread(src1, 1, 352 * 288, fp);
	bgr[2] = Mat(Size(352, 288), CV_8U, src1);//bgr[2] = R

	fread(src2, 1, 352 * 288, fp);
	bgr[1] = Mat(Size(352, 288), CV_8U, src2);//bgr[1] = G

	fread(src3, 1, 352 * 288, fp);
	bgr[0] = Mat(Size(352, 288), CV_8U, src3);//bgr[0] = B

	////////////// Merge the R, G, B information into rgbimage ////////
	Mat rgbimage;
	merge(bgr, 3, rgbimage);


	////////////////// print the result ////////////////////////////////////

	imshow("R", bgr[2]);
	waitKey(0);
	imshow("G", bgr[1]);
	waitKey(0);
	imshow("B", bgr[0]);
	waitKey(0);
	imshow("RGB", rgbimage);
	waitKey(0);

	////////////////////////////////////////////////////////////////
	//															  //
	//					Convert RGB into YCbCr					  //
	//															  //
	////////////////////////////////////////////////////////////////


	//////////////// Conver RGB into Ycbcr 4:4:4 /////////////////////////////
	Mat ycbcr444[3];
	for (int i = 0; i < 3; i++) {
		ycbcr444[i] = Mat(288, 352, CV_8U);
	}

	for (int i = 0; i < 288; i++) {
		for (int j = 0; j < 352; j++) {
			//Convert Y
			ycbcr444[0].at<uchar>(i, j) = bgr[2].at<uchar>(i, j) * 0.299 + bgr[1].at<uchar>(i, j) * 0.587 + bgr[0].at<uchar>(i, j) * 0.114;
			//Convert Cb
			ycbcr444[1].at<uchar>(i, j) = 128 - bgr[2].at<uchar>(i, j) * 0.169 - bgr[1].at<uchar>(i, j) * 0.331 + bgr[0].at<uchar>(i, j) * 0.500;
			//Convert Cr
			ycbcr444[2].at<uchar>(i, j) = 128 + bgr[2].at<uchar>(i, j) * 0.500 - bgr[1].at<uchar>(i, j) * 0.419 - bgr[0].at<uchar>(i, j) * 0.0813;
		}
	}

	///////////////////// Merge Y, Cb, Cr into ycbcr ////////////////////////////////
	Mat ycbcrimage444;
	merge(ycbcr444, 3, ycbcrimage444);

	////////////////////// Print the result //////////////////////////////////////
	imshow("Y 4:4:4", ycbcr444[0]);
	waitKey(0);
	imshow("Cb 4:4:4", ycbcr444[1]);
	waitKey(0);
	imshow("Cr 4:4:4", ycbcr444[2]);
	waitKey(0);
	imshow("YCbCrYUV 4:4:4", ycbcrimage444);
	waitKey(0);
	////////////////// save yuv file /////////////////////////////////////// 
	unsigned char result444[3][288][352];
	for (int k = 0; k < 3; k++) {
		for (int i = 0; i < 288; i++) {
			for (int j = 0; j < 352; j++) {
				result444[k][i][j] = ycbcr444[k].at<uchar>(i, j);
			}
		}
	}
	fp = fopen("Suzie_CIF_352x288_444.yuv", "wb");
	fwrite(result444, 1, 3 * 352 * 288, fp);


	//convert RGB444 and show
	Mat ycbcrRGB444[3];
	for (int i = 0; i < 3; i++) {
		ycbcrRGB444[i] = Mat(288, 352, CV_8U);
	}
	Mat result444YUV;
	ycbcrRGB444[0] = ycbcr444[0] + 1.772*(ycbcr444[1] - 128);
	ycbcrRGB444[1] = ycbcr444[0] - 0.714 * (ycbcr444[2] - 128) - 0.344*(ycbcr444[1] - 128);
	ycbcrRGB444[2] = ycbcr444[0] + 1.402*(ycbcr444[2] - 128);
	merge(ycbcrRGB444, 3, result444YUV);
	imshow("YCbCrRGB 4:4:4", result444YUV);
	waitKey(0);



	//////////////// Conver RGB into Ycbcr 4:2:0 /////////////////////////////
	Mat ycbcr420[3];
	ycbcr420[0] = Mat(288, 352, CV_8U); //ycbcr420[2] = Y
	ycbcr420[1] = Mat(288, 352, CV_8U); //ycbcr420[1] = Cb
	ycbcr420[2] = Mat(288, 352, CV_8U); //ycbcr420[0] = Cr

	////////////////// Use quarter data of Cb, Cr data in Ycbcr 4:4:4 //////////////////////// 
	Mat Cb420 = Mat(144, 176, CV_8U);
	Mat Cr420 = Mat(144, 176, CV_8U);

	resize(ycbcr444[1], Cb420, Size(176, 144));
	resize(ycbcr444[2], Cr420, Size(176, 144));

	resize(Cb420, ycbcr420[1], Size(352, 288));
	resize(Cr420, ycbcr420[2], Size(352, 288));
	resize(ycbcr444[0], ycbcr420[0], Size(352, 288));
	///////////////////// Merge Y, Cb, Cr into ycbcr4:2:0 ////////////////////////////////
	Mat ycbcrimage420;
	merge(ycbcr420, 3, ycbcrimage420);

	////////////////////// Print the result //////////////////////////////////////
	imshow("Y 4:2:0", ycbcr420[0]);
	waitKey(0);
	imshow("Cb 4:2:0", Cb420);
	waitKey(0);
	imshow("Cr 4:2:0", Cr420);
	waitKey(0);
	imshow("YCbCrYUV 4:2:0", ycbcrimage420);
	waitKey(0);

	/////////////////////////////// save 4:2:0 file ////////////////////////////

	unsigned char YData[288][352];
	unsigned char CbData[144][176];
	unsigned char CrData[144][176];
	for (int i = 0; i < 288; i++) {
		for (int j = 0; j < 352; j++) {
			YData[i][j] = ycbcr420[0].at<uchar>(i, j);
		}
	}
	for (int i = 0; i < 144; i++) {
		for (int j = 0; j < 176; j++) {
			CbData[i][j] = Cb420.at<uchar>(i, j);
			CrData[i][j] = Cr420.at<uchar>(i, j);
		}
	}
	fp = fopen("Suzie_CIF_352x288_420.yuv", "wb");
	//write Y data
	for (int i = 0; i < 288; i++) {
		fwrite(YData[i], 1, 352, fp);
	}
	// Cb 데이터 쓰기
	for (int i = 0; i < 144; i++) {
		fwrite(CbData[i], 1, 176, fp);
	}
	// Cr 데이터 쓰기
	for (int i = 0; i < 144; i++) {
		fwrite(CrData[i], 1, 176, fp);
	}

	Mat ycbcrRGB420[3];
	for (int i = 0; i < 3; i++) {
		ycbcrRGB420[i] = Mat(288, 352, CV_8U);
	}

	//show result of converted r, g, b file
	Mat result420YUV;
	ycbcrRGB420[0] = ycbcr420[0] + 1.772 * (ycbcr420[1] - 128);
	ycbcrRGB420[1] = ycbcr420[0] - 0.714 * (ycbcr420[2] - 128) - 0.344 * (ycbcr420[1] - 128);
	ycbcrRGB420[2] = ycbcr420[0] + 1.402 * (ycbcr420[2] - 128);
	merge(ycbcrRGB420, 3, result420YUV);
	imshow("YCbCrRGB 4:2:0", result420YUV);
	waitKey(0);

	

	//delete 
	delete[] src1;
	delete[] src2;
	delete[] src3;

	


	/////////////////////////////////////////////////////////////////////////////////////////////////    Assignement2      ///////////////////////////////////////////////////////////////////////////////////////
	



	//open file
	fp = fopen("RaceHorses_416x240_30.yuv", "rb");
	if (fp == NULL) {
		cout << "Failed to open" << endl;
		return 0;
	}
	//get the information of YCbCr
	unsigned char** Y = new unsigned char* [300];
	for (int i = 0; i < 300; i++) {
		Y[i] = new unsigned char[416 * 240];
	}
	unsigned char** CR = new unsigned char* [300];
	for (int i = 0; i < 300; i++) {
		CR[i] = new unsigned char[208 * 120];
	}
	unsigned char** CB = new unsigned char* [300];
	for (int i = 0; i < 300; i++) {
		CB[i] = new unsigned char[208 * 120];
	}
	Mat YCBCR_YUV[300][3];

	//get the information of Y Cb Cr
	for (int i = 0; i < 300; i++) {
		fread(Y[i], 1, 416 * 240, fp);
		YCBCR_YUV[i][0] = Mat(Size(416, 240), CV_8U, Y[i]);	//YCBCR_YUV[0] = Y
		fread(CB[i], 1, 208 * 120, fp);
		YCBCR_YUV[i][1] = Mat(Size(208, 120), CV_8U, CB[i]);//YCBCR_YUV[1] = Blue
		fread(CR[i], 1, 208 * 120, fp);
		YCBCR_YUV[i][2] = Mat(Size(208, 120), CV_8U, CR[i]);//YCBCR_YUV[2] = Red
	}

	Mat tempYUV[300][3];
	for (int i = 0; i < 300; i++) {
		for(int j = 0; j<3; j++)
			tempYUV[i][j] = Mat(240, 416, CV_8U);
	}

	//resize YCbCr data into 416 x 240
	for (int i = 0; i < 300; i++) {
		tempYUV[i][0] = YCBCR_YUV[i][0];
		resize(YCBCR_YUV[i][1], tempYUV[i][1], Size(416, 240));
		resize(YCBCR_YUV[i][2], tempYUV[i][2], Size(416, 240));
	}


	/*
	//show the result of Y Cb Cr
	imshow("Y", tempYUV[0][0]); waitKey();
	imshow("Cb", tempYUV[0][1]); waitKey();
	imshow("Cr", tempYUV[0][2]); waitKey();
	*/




	Mat RGBvideo[300][3];
	for (int i = 0; i < 300; i++) {
		for (int j = 0; j < 3; j++)
			RGBvideo[i][j] = Mat(240, 416, CV_8U);
	}

	//convert the RGB data
	for (int i = 0; i < 300; i++) {
		for (int j = 0; j < 240; j++) {
			for (int k = 0; k < 416; k++) {
				RGBvideo[i][2].at<uchar>(j, k) = tempYUV[i][0].at<uchar>(j, k) + 1.402 * (tempYUV[i][2].at<uchar>(j, k) - 128);
				RGBvideo[i][1].at<uchar>(j, k) = tempYUV[i][0].at<uchar>(j, k) - 0.714 * (tempYUV[i][2].at<uchar>(j, k) - 128) - 0.344 * (tempYUV[i][1].at<uchar>(j, k) - 128);
				RGBvideo[i][0].at<uchar>(j, k) = tempYUV[i][0].at<uchar>(j, k) + 1.772 * (tempYUV[i][1].at<uchar>(j, k) - 128);
			}
		}
	}

	//merge the video
	Mat Video[300];
	for (int i = 0; i < 300; i++) {
		merge(RGBvideo[i], 3, Video[i]);
	}

	
	/*
	//show the result of RGB
	imshow("Blue", RGBvideo[0][0]); waitKey(0);
	imshow("Green", RGBvideo[0][1]); waitKey(0);
	imshow("Red", RGBvideo[0][2]); waitKey(0);
	*/


	
	//show the result of video
	for (int i = 0; i < 300; i++) {
		imshow("RaceHorses_416x240_30.yuv", Video[i]);
		waitKey(20);
	}
	waitKey();


	//Save the RGB file
	unsigned char*** racehorse1 = new unsigned char** [300];
	unsigned char*** racehorse2 = new unsigned char** [300];
	unsigned char*** racehorse3 = new unsigned char** [300];
	for (int i = 0; i < 300; i++) {
		racehorse1[i] = new unsigned char* [240];
		racehorse2[i] = new unsigned char* [240];
		racehorse3[i] = new unsigned char* [240];
		for (int j = 0; j < 240; j++) {
			racehorse1[i][j] = new unsigned char[416];
			racehorse2[i][j] = new unsigned char[416];
			racehorse3[i][j] = new unsigned char[416];
		}
	}

	for (int i = 0; i < 300; i++) {
		for (int j = 0; j < 240; j++) {
			for (int k = 0; k < 416; k++) {
				racehorse1[i][j][k] = RGBvideo[i][0].at<uchar>(j, k);
				racehorse2[i][j][k] = RGBvideo[i][1].at<uchar>(j, k);
				racehorse3[i][j][k] = RGBvideo[i][2].at<uchar>(j, k);
			}
		}
	}
	
	//write the file
	fp = fopen("RaceHorses_416x240_30.raw", "wb");
	for (int i = 0; i < 300; i++) {
		for (int j = 0; j < 240; j++) {
			fwrite(racehorse3[i][j], 1, 416, fp);
		}
		for (int j = 0; j < 240; j++) {
			fwrite(racehorse2[i][j], 1, 416, fp);
		}
		for (int j = 0; j < 240; j++) {
			fwrite(racehorse1[i][j], 1, 416, fp);
		}
	}

	//delete
	for (int i = 0; i < 300; i++) {
		delete[] Y[i];
		delete[] CR[i];
		delete[] CB[i];
	}
	delete[] Y;
	delete[] CR;
	delete[] CB;
	for (int i = 0; i < 300; i++) {
		for (int j = 0; j < 240; j++) {
			delete[] racehorse1[i][j];
			delete[] racehorse2[i][j];
			delete[] racehorse3[i][j];
		}
		delete[] racehorse1[i];
		delete[] racehorse2[i];
		delete[] racehorse3[i];
	}
	delete[] racehorse1;
	delete[] racehorse2;
	delete[] racehorse3;
	return 0;
}