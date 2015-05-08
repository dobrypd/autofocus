#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const char * windowOriginal = "Captured preview";

int main(int argc, char ** argv)
{
    VideoCapture cap("Nikon");
    if (!cap.isOpened())
        return -1;

    Mat frame;
    char key = 0;

    cap.set(CAP_PROP_GPHOTO2_PREVIEW, true);

    namedWindow(windowOriginal, 1);

    while(key != 'q')
    {
        cap >> frame;

        imshow(windowOriginal, frame);
        switch(key = waitKey(30))
        {
            case 'j':
                cap.set(CAP_PROP_ZOOM, 100);
                break;
            case 'k':
                cap.set(CAP_PROP_ZOOM, -100);
                break;
        }
    }

    return 0;
}

