#include "pch.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <stack>

using namespace cv;
using namespace std;
#define FRAMENUMS 8
#define ENTERKEYA 13
#define ENTERKEYB 10
#define SPACEKEY 32
#define QUITKEY 27

string video_path = "F://Video_60fps_multi.mp4";
string eyes_output_path = "eyes_output_path.txt";
string eyescenter_output_path = "eyescenter_output_path.txt";
stack<Point>  mousePoints;
vector<vector<Point>> eyesLocations;
vector<vector<Point>> eyesCenterLocations;

void on_MouseHandle(int event, int x, int y, int flags, void* param);
void Print2Txt();
int GetLastFramecount();

int main()
{
	//读取摄像头
	VideoCapture cap(video_path);
	if (!cap.isOpened())
		return -1;

	//设置鼠标回调
	namedWindow("StandardVideo");
	Mat frame;
	cvSetMouseCallback("StandardVideo", on_MouseHandle, &frame);

	//计算已读行数
	int linecount = GetLastFramecount();

	//进行点击记录
	int framcount = 0;
	//while (++framcount) {
	while (++framcount < FRAMENUMS + 1) {

		cap >> frame;
		if (frame.empty())
			break;

		//跳过已读行
		if (framcount <= linecount)
			continue;

		//读入新的一帧，并在该帧上显示上一次记录的坐标位置	
		cout << "new frame: " << framcount << endl;
		if (!eyesLocations.empty()) {
			vector<Point> lastPoints(eyesLocations[eyesLocations.size() - 1]);
			for (Point point : lastPoints) {
				char temp[16];
				sprintf(temp, "(%d,%d)", point.x, point.y);
				circle(frame, point, 3, cv::Scalar(0, 0, 255), -1);
				putText(frame, temp, point, FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0, 255));
			}
		}
		imshow("StandardVideo", frame);

		//等待键盘事件
		char key = waitKey(0);

		//若键盘事件为空格，则将该帧记录为上一帧记录的拷贝;案件Q为退出;其他按键则正常记录当前帧
		if (key == SPACEKEY && !eyesLocations.empty()) {
			cout << "the same as last frame's record" << endl;
			vector<Point> lastPoints(eyesLocations[eyesLocations.size() - 1]);
			eyesLocations.push_back(lastPoints);
			vector<Point> lastCenterPoints(eyesCenterLocations[eyesCenterLocations.size() - 1]);
			eyesCenterLocations.push_back(lastCenterPoints);
		}
		else if (key == QUITKEY) {
			cout << "Quit" << endl;
			break; 
		}
		else {
			//若漏点一只眼睛，则直接退出
			if (!mousePoints.empty() && (mousePoints.size() & 0x01) == 1) {
				cout << "single eye problem" << endl;
				break;
			}

			cout << "record new frame" << endl;

			//存入人眼坐标
			vector<Point> eyesPoints;
			while (!mousePoints.empty()) {
				eyesPoints.push_back(mousePoints.top());
				mousePoints.pop();
			}
			eyesLocations.push_back(eyesPoints);

			//存入眉心坐标
			vector<Point> eyesCenterPoints;
			if (!eyesPoints.empty()) {
				for (int i = 0; i < eyesPoints.size() - 1; i += 2) {
					Point eyecenter = Point((eyesPoints[i].x + eyesPoints[i + 1].x) >> 1, (eyesPoints[i].y + eyesPoints[i + 1].y) >> 1);
					eyesCenterPoints.push_back(eyecenter);
				}
			}
			eyesCenterLocations.push_back(eyesCenterPoints);
		}

	}

	Print2Txt();

	cout << "Mission Completed" << endl;
	system("pause");
	return 0;
}

//设置鼠标事件
void on_MouseHandle(int event, int x, int y, int flags, void* param)
{
	Mat img = *(Mat *)(param);
	//鼠标左键用于点击记录，在"points in img"中对点击位置进行反馈
	if (event == EVENT_LBUTTONDOWN) {
		Mat img_show = img.clone();
		Point point = Point(x, y);
		mousePoints.push(point);
		char temp[16];
		sprintf(temp, "(%d,%d)", x, y);
		cout << "x: " << x << " y: " << y << endl;
		circle(img_show, point, 3, cv::Scalar(0, 0, 255), -1);
		putText(img_show, temp, point, FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0, 255));
		imshow("points in img", img_show);
	}
	//鼠标右键用于重置点数 
	else if (event == EVENT_RBUTTONDOWN) {
		while (!mousePoints.empty()) {
			mousePoints.pop();
		}
		cout << "reset this frame" << endl;
		imshow("points in img", img);
	}

}

//根据纵坐标大小排列所有点
bool comparator_of_points(const Point& point1, const Point& point2) {
	if (point1.y == point2.y) {
		return point1.x < point2.x;
	}
	return point1.y < point2.y;
}

//将所有点记录进txt
void Print2Txt()
{
	string eyepath = eyes_output_path;
	string eyecenterpath = eyescenter_output_path;
	ofstream fout;
	ifstream fin;

	//写入人眼坐标
	fout.open(eyepath, ios::app);
	if (!fout) {
		cout << "can't open file" << endl;
	}
	for (int i = 0; i < eyesLocations.size(); i++) {
		fout << eyesLocations[i].size();
		sort(eyesLocations[i].begin(), eyesLocations[i].end(), comparator_of_points);
		for (int j = 0; j < eyesLocations[i].size(); j++) {
			fout << "," << eyesLocations[i][j].x << "," << eyesLocations[i][j].y;
		}
		fout << endl;
	}
	fout.close();

	//写入眉心坐标
	fout.open(eyecenterpath, ios::app);
	if (!fout) {
		cout << "can't open file" << endl;
	}
	for (int i = 0; i < eyesCenterLocations.size(); i++) {
		fout << eyesCenterLocations[i].size();
		sort(eyesCenterLocations[i].begin(), eyesCenterLocations[i].end(), comparator_of_points);
		for (int j = 0; j < eyesCenterLocations[i].size(); j++) {
			fout << "," << eyesCenterLocations[i][j].x << "," << eyesCenterLocations[i][j].y;
		}
		fout << endl;
	}
	fout.close();
}

//计算上一次记录的行数
int GetLastFramecount() {
	string eyepath = eyes_output_path;
	ifstream fin;
	fin.open(eyepath);
	if (!fin) {
		cout << "file isn't exist" << endl;
		return 0;
	}
	int linecount = 0;
	string str;
	while (getline(fin, str, '\n'))
		linecount++;
	cout << "linecount: " << linecount << endl;
	return linecount;
}
