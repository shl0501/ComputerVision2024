#define _CRT_SECURE_NO_WARNINGS

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <string>
#include<math.h>

#include<stack>

using namespace cv;
using namespace std;

stack<pair<int, int>> st;


void get_line(int startx, int starty, int endx, int endy, int visited[400][396])
{   
    if (endx == startx) {                   //수직선인 경우
        int x = startx;
        int minY = min(starty, endy);
        int maxY = max(starty, endy);
        for (int y = minY; y <= maxY; y++) {
            if (0 <= y && y < 396) {
                visited[x][y] += 1;         //수직선을 따라 해당 픽셀의 값 1씩 증가
            }
        }
    }
    else {
        double m = (double)(endy - starty) / (endx - startx);   //직선의 기울기 구하기
        for (int i = 0; i < 400; i++) {
            int y = m * (i - startx) + starty;                  //직선의 y좌표 구하기
            if (0 <= y && y < 396)
                visited[i][y] += 1;                             //해당 픽셀 1씩 증가
        }
    }


}

int main() {
    /*********************** open file ************************/   
    FILE* fp = fopen("coins_396x400.raw", "rb");
    if (fp == NULL) {
        cout << "Failed to open" << endl;
        return -1;
    }
    /*********************************************************/

	unsigned char* src = new unsigned char[396 * 400];
	Mat coin_image;

    /******************** read file pointer ********************/ 
	fread(src, 1, 396 * 400, fp);
	coin_image = Mat(Size(396, 400), CV_8U, src);
    /**********************************************************/

    imshow("coin image", coin_image);
    waitKey(0);
    /*********************************************************************/
    /*                          가우시안 필터 적용                       */    
    /*********************************************************************/

    Mat gaussian_filtered_coin_image = Mat(Size(396, 400), CV_8U);


    /********************* 가우시안 커널 **********************/
    const double gaussian_kernel[5][5] = {
        {0.0000, 0.0000, 0.0002, 0.0000, 0.0000},
        {0.0000, 0.0113, 0.0837, 0.0113, 0.0000},
        {0.0002, 0.0837, 0.6187, 0.0837, 0.0002},
        {0.0000, 0.0113, 0.0837, 0.0113, 0.0000},
        {0.0000, 0.0000, 0.0002, 0.0000, 0.0000}
    };
    /*********************************************************/

    /************** 가우시안 blur 연산 ***************/
    for (int i = 0; i < 400; i++) {
        for (int j = 0; j < 396; j++) {
            double sum = 0.0;
            for (int k = -2; k <= 2; k++) {
                for (int l = -2; l <= 2; l++) {
                    int x = i + k;
                    int y = j + l;
                    if (x >= 0 && x < 400 && y >= 0 && y < 396) {
                        sum += coin_image.at<uchar>(x, y) * gaussian_kernel[k + 2][l + 2];
                    }
                }
            }
            gaussian_filtered_coin_image.at<uchar>(i, j) = (int)sum;
        }
    }
    /************************************************/


    /*********************************************************************/

    
    imshow("Gaussian Filtered Image", gaussian_filtered_coin_image);
    waitKey(0);


    //********************************************************************/
    /*                        sobel 필터 적용                            */    
    /*********************************************************************/
    Mat gradient_x;
    Mat gradient_y;
    Sobel(gaussian_filtered_coin_image, gradient_x, CV_16S, 1, 0, 5);
    Sobel(gaussian_filtered_coin_image, gradient_y, CV_16S, 0, 1, 5);
    /*********************************************************************/


    imshow("gradient_x", gradient_x);
    waitKey(0);
    imshow("gradient_y", gradient_y);
    waitKey(0);

    /*********************************************************************/
    /*                       gradient 구하기                             */
    /*********************************************************************/
    Mat gradient_magnitude;
    gradient_magnitude = abs(gradient_x) + abs(gradient_y);
    /*********************************************************************/

    // 결과 이미지를 표시합니다.
    imshow("gradient magnitude", gradient_magnitude);
    waitKey(0);


    /***************************************************************************************************/
    /*                              calculate and normalize the angle                                  */
    /***************************************************************************************************/
    Mat gradient_angle = Mat(Size(396, 400), CV_16S);
    gradient_angle = Mat::zeros(Size(396, 400), CV_16S);
    for (int i = 0; i < gradient_x.rows; i++) {
        for (int j = 0; j < gradient_x.cols; j++) {
            double angle = atan2(gradient_y.at<short>(i, j), gradient_x.at<short>(i, j)) * 180 / CV_PI;              //get the angle
            short temp_angle = (short)round(angle);
            gradient_angle.at<short>(i, j) = (short)round(angle);
            /*************************  normalize angle **********************/
            if (angle < 0)
                angle += 180;
            if (0 <= angle && angle < 22.5)
                gradient_angle.at<short>(i, j) = 0;
            else if (22.5 <= angle && angle < 67.5)
                gradient_angle.at<short>(i, j) = 45;
            else if (67.5 <= angle && angle < 112.5)
                gradient_angle.at<short>(i, j) = 90;
            else if (112.5 <= angle && angle < 157.5)
                gradient_angle.at<short>(i, j) = 135;
            else if (157.5 <= angle && angle <= 180)
                gradient_angle.at<short>(i, j) = 0;
        }
        /********************************************************************/
    }
    /**************************************************************************************************/

    // 결과 이미지를 표시합니다.
    imshow("Gradient Angle", gradient_angle);
    waitKey(0);

    

    /******************************************************************************************************/
    /*                                      Non-maximum suppression                                       */
    /******************************************************************************************************/
    for (int i = 0; i < gradient_x.rows; i++) {
        for (int j = 0; j < gradient_x.cols; j++) {
            if (gradient_angle.at<short>(i, j) == 0) {                                                  //if angle == 0, compare with down pixel
                if (j + 1 < gradient_x.cols) {                                                          
                    if (gradient_magnitude.at<short>(i, j + 1) > gradient_magnitude.at<short>(i, j))
                        gradient_magnitude.at<short>(i, j) = 0;
                }
                if (j - 1 >= 0) {                                                                      
                    if (gradient_magnitude.at<short>(i, j - 1) > gradient_magnitude.at<short>(i, j))
                        gradient_magnitude.at<short>(i, j) = 0;
                }
            }
            else if (gradient_angle.at<short>(i, j) == 135) {                                           //if angle == 45, compare with 135 pixel
                if (i - 1 >= 0 && j + 1 < gradient_x.cols) {
                    if (gradient_magnitude.at<short>(i - 1, j + 1) > gradient_magnitude.at<short>(i, j))
                        gradient_magnitude.at<short>(i, j) = 0;
                }
                if (i + 1 <= gradient_x.rows && j - 1 >= 0) {
                    if (gradient_magnitude.at<short>(i + 1, j - 1) > gradient_magnitude.at<short>(i, j))
                        gradient_magnitude.at<short>(i, j) = 0;
                }
            }
            else if (gradient_angle.at<short>(i, j) == 90) {                                           //if angle == 90, compare with side pixel
                if (i - 1 >= 0) {
                    if (gradient_magnitude.at<short>(i - 1, j) > gradient_magnitude.at<short>(i, j))
                        gradient_magnitude.at<short>(i, j) = 0;
                }
                if (i + 1 < gradient_x.rows) {
                    if (gradient_magnitude.at<short>(i + 1, j) > gradient_magnitude.at<short>(i, j))
                        gradient_magnitude.at<short>(i, j) = 0;
                }
            }
            else if (gradient_angle.at<short>(i, j) == 45) {                                        //if angle == 135, compare with 45 pixel
                if (i - 1 >= 0 && j - 1 >= 0) {
                    if (gradient_magnitude.at<short>(i - 1, j - 1) > gradient_magnitude.at<short>(i, j))
                        gradient_magnitude.at<short>(i, j) = 0;
                }
                if (i + 1 < gradient_x.rows && j + 1 < gradient_x.cols) {
                    if (gradient_magnitude.at<short>(i + 1, j + 1) > gradient_magnitude.at<short>(i, j))
                        gradient_magnitude.at<short>(i, j) = 0;
                }
            }
        }
    }
    /*******************************************************************************************************/


    imshow("non maximum suppression", gradient_magnitude);
    waitKey(0);

    /******************************************************************/
    /*                          two level threshold                   */
    /******************************************************************/
    int high_threshold = 5000;
    int low_threshold = 4500;
    char threshold[400][396];
    for (int i = 0; i < gradient_x.rows; i++) {
        for (int j = 0; j < gradient_x.cols; j++) {
            int temp = (int)gradient_magnitude.at<short>(i, j);
            if (temp < low_threshold)                                   //if lower than low_threshold, make it 0
                threshold[i][j] = 0;
            if (low_threshold <= temp && temp < high_threshold)         //weak edge
                threshold[i][j] = 1;
            if (high_threshold <= temp)                                 //strong edge
                threshold[i][j] = 2;
        }
    }
    /******************************************************************/

    int visited[400][396] = { 0, };

    /**********************************************************************************************************************/
    /*                                          경계선 추적을 통한 잘못된 경계선 제거                                     */
    /**********************************************************************************************************************/
    int dx[4] = { -1, 0, 1, 0 };
    int dy[4] = { 0, 1, 0, -1 };
    for (int i = 0; i < gradient_x.rows; i++) {
        for (int j = 0; j < gradient_x.cols; j++) {
            if (threshold[i][j] == 2) {                     // 만약 strong edge라면, 방문처리하고 stack에 넣기
                visited[i][j] = 1;
                st.push({ i, j });                          
                while (!st.empty()) {                       //strong edge에 붙어있는 선(픽셀) 따라가기
                    int x = st.top().first;
                    int y = st.top().second;
                    st.pop();
                    for (int k = 0; k < 4; k++) {
                        int nx = x + dx[k];
                        int ny = y + dy[k];
                        if (0 <= nx && nx < 400 && 0 <= ny && ny < 396 && !visited[nx][ny] && threshold[nx][ny] == 1)
                        {
                            visited[nx][ny] = 1;            // strong edge와 연결된 픽셀은 방문처리 후 stack에 넣어서 해당 픽셀과 연결된 선 찾기
                            st.push({ nx, ny });
                        }
                    }
                }
            }
        }
    }
    /****************** 잘못된 엣지(방문하지 않은 픽셀)은 0으로 ************/
    for (int i = 0; i < gradient_x.rows; i++) {
        for (int j = 0; j < gradient_x.cols; j++) {
            if (visited[i][j] == 0)
                gradient_magnitude.at<short>(i, j) = 0;
        }
    }
    /**********************************************************************/

    /*********************************************************************************************************************/


    for (int i = 0; i < 400; i++) {
        for (int j = 0; j < 396; j++) {
            visited[i][j] = 0;
        }
    }

    Mat gradient_magnitude32F = gradient_magnitude.clone();
    gradient_magnitude.convertTo(gradient_magnitude32F, CV_32F);
    imshow("strong edge", gradient_magnitude32F);
    waitKey(0);

    /***********************************************************************/
    /*                      그래디언트를 기반으로 선긋기                   */
    /***********************************************************************/
    for (int i = 0; i < gradient_x.rows; i++) {
        for (int j = 0; j < gradient_x.cols; j++) {
            if (gradient_magnitude.at<short>(i, j) != 0) {
                int startx = i;
                int starty = j;
                int endx = starty + (int)gradient_y.at<short>(i, j);
                int endy = startx + (int)gradient_x.at<short>(i, j);
                get_line(startx, starty, endx, endy, visited);          //직선의 방정식을 이용하여 원 검출
            }
        }
    }    
    Mat line = Mat(Size(396, 400), CV_16S);
    for (int i = 0; i < 400; i++) {
        for (int j = 0; j < 396; j++) {
            line.at<ushort>(i, j) = visited[i][j] * 300;
        }
    }
    imshow("Line Image", line);
    waitKey(0);
    /**********************************************************************/


    /**********************************************************************/
    /*                          원의 중심점 찾기                          */
    /**********************************************************************/
    int M[4] = { 0, };
    Point p[4];
    for (int i = 0; i < 200; i++) {     //2사분면
        for (int j = 0; j < 198; j++) {
            if ((int)line.at<ushort>(i, j) > M[0]) {
                M[0] = (int)line.at<ushort>(i, j);
                p[0].x = j;
                p[0].y = i;
            }
        }
    }
    for (int i = 0; i < 200; i++) {     //1사분면
        for (int j = 198; j < 396; j++) {
            if ((int)line.at<ushort>(i, j) > M[1]) {
                M[1] = (int)line.at<ushort>(i, j);
                p[1].x = j;
                p[1].y = i;
            }
        }
    }
    for (int i = 200; i < 400; i++) {   //3사분면
        for (int j = 0; j < 198; j++) {
            if ((int)line.at<ushort>(i, j) > M[2]) {
                M[2] = (int)line.at<ushort>(i, j);
                p[2].x = j;
                p[2].y = i;
            }
        }
    }
    for (int i = 200; i < 400; i++) {   //4사분면
        for (int j = 198; j < 396; j++) {
            if ((int)line.at<ushort>(i, j) > M[3]) {
                M[3] = (int)line.at<ushort>(i, j);
                p[3].x = j;
                p[3].y = i;
            }
        }
    }
    /**********************************************************************/



    /*****************************************************************************/
    /*                  gradient 기반 정답에 가까운 원 구하기                    */   
    /*****************************************************************************/
    int radius[4] = { 0, };
    for (int i = 0; i < 4; i++) {
        int cnt = 0;
        for (int r = 1; r < 200; r++) {
            /****************************원 그리기*********************/;
            Mat circle_images = Mat::zeros(Size(396, 400), CV_16S);
            circle(circle_images, p[i], r, Scalar(255, 255, 255), 1);
            /*********************************************************/

            /******************* 정답에 가까운 원 구하기 ***************/
            int minicnt = 0;
            for (int cx = 0; cx < 400; cx++) {
                for (int cy = 0; cy < 396; cy++) {
                    if (circle_images.at<ushort>(cx, cy) != 0) {
                        if (gradient_magnitude.at<ushort>(cx, cy) != 0) {
                            minicnt++;
                        }
                    }
                }
            }

            if (minicnt > cnt) {
                cnt = minicnt;
                radius[i] = r;  // 반지름 구하기
            }
            /***********************************************************/
        }
    }
    Mat circle_img = Mat(Size(396, 400), CV_16S);
    for (int i = 0; i < 4; i++) {
        circle(circle_img, p[i], radius[i], Scalar(255, 255, 255), 1);
        cout << i << "\t" << "(a, b) = " << p[i].x << " " << p[i].y << "\tr = " << radius[i] << endl;
    }
    imshow("circles", circle_img);
    waitKey(0);
    /*****************************************************************************/


    ///*****************************************************************************/
    ///*                  (a, b, r) 기반 정답에 가까운 원 구하기                   */
    ///*****************************************************************************/
    //int abr_radius[4] = { 0, };
    //Point abr_p[4];

    //int ra[4] = { 80,90,90,60 };
    //int cnt = 0;
    //for (int i = 90; i < 140; i++) {
    //    for (int j = 90; j < 140; j++) {
    //        for (int r = 50; r < ra[0]; r++) {
    //            Point dot;
    //            dot.x = i;
    //            dot.y = j;
    //            /****************************원 그리기*********************/;
    //            Mat abr_circle_images = Mat::zeros(Size(396, 400), CV_16S);
    //            circle(abr_circle_images, dot, r, Scalar(255, 255, 255), 1);

    //            /******************* 정답에 가까운 원 구하기 ***************/
    //            int minicnt = 0;
    //            for (int cx = 40; cx < 200; cx++) {
    //                for (int cy = 40; cy < 200; cy++) {
    //                    if (abr_circle_images.at<ushort>(cx, cy) != 0) {
    //                        if (gradient_magnitude.at<ushort>(cx, cy) != 0) {
    //                            minicnt++;
    //                        }
    //                    }
    //                }
    //            }

    //            if (minicnt > cnt) {
    //                cnt = minicnt;
    //                abr_p[0].x = i;
    //                abr_p[0].y = j;
    //                abr_radius[0] = r;  // 반지름 구하기
    //            }
    //            /***********************************************************/
    //        }
    //    }
    //}
    //cnt = 0;
    //for (int i = 90; i < 140; i++) {
    //    for (int j = 260; j < 310; j++) {
    //        for (int r = 50; r < ra[1]; r++) {
    //            Point dot;
    //            dot.x = i;
    //            dot.y = j;
    //            /****************************원 그리기*********************/;
    //            Mat abr_circle_images = Mat::zeros(Size(396, 400), CV_16S);
    //            circle(abr_circle_images, dot, r, Scalar(255, 255, 255), 1);
    //            /*********************************************************/

    //            /******************* 정답에 가까운 원 구하기 ***************/
    //            int minicnt = 0;
    //            for (int cx = 0; cx < 400; cx++) {
    //                for (int cy = 0; cy < 396; cy++) {
    //                    if (abr_circle_images.at<ushort>(cx, cy) != 0) {
    //                        if (gradient_magnitude.at<ushort>(cx, cy) != 0) {
    //                            minicnt++;
    //                        }
    //                    }
    //                }
    //            }

    //            if (minicnt > cnt) {
    //                cnt = minicnt;
    //                abr_p[1].x = i;
    //                abr_p[1].y = j;
    //                abr_radius[1] = r;  // 반지름 구하기
    //            }
    //            /***********************************************************/
    //        }
    //    }
    //}
    //cnt = 0;
    //for (int i = 250; i < 300; i++) {
    //    for (int j = 80; j < 136; j++) {
    //        for (int r = 50; r < ra[2]; r++) {
    //            Point dot;
    //            dot.x = i;
    //            dot.y = j;
    //            /****************************원 그리기*********************/;
    //            Mat abr_circle_images = Mat::zeros(Size(396, 400), CV_16S);
    //            circle(abr_circle_images, dot, r, Scalar(255, 255, 255), 1);
    //            /*********************************************************/

    //            /******************* 정답에 가까운 원 구하기 ***************/
    //            int minicnt = 0;
    //            for (int cx = 200; cx < 380; cx++) {
    //                for (int cy = 30; cy < 396; cy++) {
    //                    if (abr_circle_images.at<ushort>(cx, cy) != 0) {
    //                        if (gradient_magnitude.at<ushort>(cx, cy) != 0) {
    //                            minicnt++;
    //                        }
    //                    }
    //                }
    //            }

    //            if (minicnt > cnt) {
    //                cnt = minicnt;
    //                abr_p[2].x = i;
    //                abr_p[2].y = j;
    //                abr_radius[2] = r;  // 반지름 구하기
    //            }
    //            /***********************************************************/
    //        }
    //    }
    //}
    //cnt = 0;
    //for (int i = 250; i < 310; i++) {
    //    for (int j = 250; j < 300; j++) {
    //        for (int r = 40; r < ra[3]; r++) {
    //            Point dot;
    //            dot.x = i;
    //            dot.y = j;
    //            /****************************원 그리기*********************/;
    //            Mat abr_circle_images = Mat::zeros(Size(396, 400), CV_16S);
    //            circle(abr_circle_images, dot, r, Scalar(255, 255, 255), 1);
    //            /*********************************************************/

    //            /******************* 정답에 가까운 원 구하기 ***************/
    //            int minicnt = 0;
    //            for (int cx = 210; cx < 360; cx++) {
    //                for (int cy = 100; cy < 250; cy++) {
    //                    if (abr_circle_images.at<ushort>(cx, cy) != 0) {
    //                        if (gradient_magnitude.at<ushort>(cx, cy) != 0) {
    //                            minicnt++;
    //                        }
    //                    }
    //                }
    //            }

    //            if (minicnt > cnt) {
    //                cnt = minicnt;
    //                abr_p[3].x = i;
    //                abr_p[3].y = j;
    //                abr_radius[3] = r;  // 반지름 구하기
    //            }
    //            /***********************************************************/
    //        }
    //    }
    //}

    //Mat abr_circle_img = Mat(Size(396, 400), CV_16S);
    //for (int i = 0; i < 4; i++) {
    //    circle(abr_circle_img, abr_p[i], abr_radius[i], Scalar(255, 255, 255), 1);
    //    cout << i << "\t" << "(a, b) = " << abr_p[i].x << " " << abr_p[i].y << "\tr = " << abr_radius[i] << endl;
    //}
    //imshow("abr_circles", abr_circle_img);
    //waitKey(0);



    delete[] src; // Clean up memory
    return 0;
}
