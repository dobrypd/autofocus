#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const char * windowOriginal = "Captured preview";
const int FOCUS_STEP = 512;
const int MAX_FOCUS_STEP = 32767;
const int FOCUS_DIRECTION_INFTY = 1;
const double epsylon = 0.001;

struct FocusState
{
    int step = 0;
    int direction = FOCUS_DIRECTION_INFTY;
    int minFocusStep = 10;
    double rate = 0;
    double rateDelta = 0;
    double maxRateFound = 0;
    double minRateFound = 1;
};

void focusDriveEnd(VideoCapture & cap, int direction)
{
    while (cap.set(CAP_PROP_ZOOM, (double)MAX_FOCUS_STEP * direction));
}

/**
 * Minimal focus step depends on lens
 * and I don't want to make any assumptions about it.
 *
 * Ends with lens with closest focal point.
 */
int findMinFocusStep(VideoCapture & cap, int startWith)
{
    int lStep = 0;
    int rStep = startWith;

    focusDriveEnd(cap, -FOCUS_DIRECTION_INFTY);
    while (lStep < rStep) {
        int mStep = (lStep + rStep) / 2;
        cap.set(CAP_PROP_ZOOM, -FOCUS_DIRECTION_INFTY * FOCUS_STEP); // Make space
        if (cap.set(CAP_PROP_ZOOM, mStep)) {
            rStep = mStep;
        } else {
            lStep = mStep + 1;
        }
    }
    cap.set(CAP_PROP_ZOOM, -FOCUS_DIRECTION_INFTY * MAX_FOCUS_STEP);
    return lStep;
}

/**
 * Rate frame from 0/blury/ to 1/sharp/.
 */
double rateFrame(Mat frame)
{
    unsigned long int sum = 0;
    unsigned long int size = frame.cols * frame.rows;
    Mat edges;
    cvtColor(frame, edges, CV_BGR2GRAY);
    GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
    Canny(edges, edges, 0, 30, 3);

    std::for_each(edges.begin<uchar>(),edges.end<uchar>(),[&](uchar v){
        sum += v != 0;
    });

    return (double) sum / (double) size;
}


int correctFocus(bool lastSuccess, FocusState & state, double rate)
{
    double rateDelta = state.direction * rate - state.rate;
    if (!lastSuccess) {
        state.direction *= -1;
        state.step = state.minFocusStep;
    } else {
        cout << rate << "   min=" << state.minRateFound <<  "   max=" << state.maxRateFound << "   delta=" << rateDelta << "   lastDelta" << state.rateDelta << endl;
        if (rate > state.maxRateFound - epsylon)
        {
            //
        } else {
            state.direction *= -1;
        }


        if (rateDelta < epsylon)
        {
            state.step *= 2;
        }
        else {
            state.step *= 2;
        }
    }

    // Update state.
    if (rate > state.maxRateFound)
        state.maxRateFound = rate;
    if (rate < state.minRateFound)
        state.minRateFound = rate;
    state.rateDelta = rateDelta;
    state.rate = rate;
    return state.step;
}

int main(int argc, char ** argv)
{
    VideoCapture cap("Nikon");
    if (!cap.isOpened())
        return -1;

    Mat frame;
    FocusState state;
    bool focus = true;
    bool lastSuccess = true;
    namedWindow(windowOriginal, 1);

    // Get settings:
    // cout << (const char *) (intptr_t) cap.get(CAP_PROP_GPHOTO2_WIDGET_ENUMERATE) << endl;

    cap.set(CAP_PROP_GPHOTO2_PREVIEW, true);
    cap.set(CAP_PROP_VIEWFINDER, true);
    state.minFocusStep = findMinFocusStep(cap, FOCUS_STEP / 16);

    char key = 0;
    while(key != 'q')
    {
        cap >> frame;

        if (focus)
            lastSuccess = cap.set(CAP_PROP_ZOOM,
                    correctFocus(lastSuccess, state, rateFrame(frame)) * state.direction);

        imshow(windowOriginal, frame);

        switch(key = waitKey(30))
        {
            case 'k': // focus out
                cap.set(CAP_PROP_ZOOM, 100);
                break;
            case 'j': // focus in
                cap.set(CAP_PROP_ZOOM, -100);
                break;
            case ',': // Drive to closest
                focusDriveEnd(cap, -FOCUS_DIRECTION_INFTY);
                break;
            case '.': // Drive to infinity
                focusDriveEnd(cap, FOCUS_DIRECTION_INFTY);
                break;
            case 'r': // reset focus state
                state = FocusState();
                break;
            case 'f': // focus switch on/off
                focus ^= true;
        }
    }

    return 0;
}

