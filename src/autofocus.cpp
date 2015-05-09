/*
 * Copyright (c) 2015, Piotr Dobrowolski dobrypd[at]gmail[dot]com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const char * windowOriginal = "Captured preview";
const int FOCUS_STEP = 512;
const int MAX_FOCUS_STEP = 32767;
const int FOCUS_DIRECTION_INFTY = 1;
const int DEFAULT_BREAK_LIMIT = 5;
const double epsylon = 0.001; // compression, noice, etc.

struct Args_t
{
    const char * deviceName = NULL;
    const char * output = NULL;
    unsigned int fps = 20;
    unsigned int minimumFocusStep = 0;
    unsigned int breakLimit = DEFAULT_BREAK_LIMIT;
    bool measure = false;
    bool verbose = false;
} GlobalArgs;

struct FocusState
{
    int step = FOCUS_STEP;
    int direction = FOCUS_DIRECTION_INFTY;
    int minFocusStep = 10;
    int lastDirectionChange = 0;
    double rate = 0;
    double rateMax = 0;
    int stepToLastMax = 0;
};

ostream & operator<<(ostream & os, FocusState & state)
{
    os << "RATE=" << state.rate << "\tSTEP=" << state.step * state.direction
            << "\tLast change=" << state.lastDirectionChange;
}

void focusDriveEnd(VideoCapture & cap, int direction)
{
    while (cap.set(CAP_PROP_ZOOM, (double) MAX_FOCUS_STEP * direction))
        ;
}

/**
 * Minimal focus step depends on lens
 * and I don't want to make any assumptions about it.
 *
 */
int findMinFocusStep(VideoCapture & cap, unsigned int startWith, int direction)
{
    int lStep, rStep;
    lStep = 0;
    rStep = startWith;

    focusDriveEnd(cap, direction * FOCUS_DIRECTION_INFTY);
    while (lStep < rStep)
    {
        int mStep = (lStep + rStep) / 2;
        cap.set(CAP_PROP_ZOOM, direction * FOCUS_DIRECTION_INFTY * FOCUS_STEP);
        if (cap.set(CAP_PROP_ZOOM, -direction * mStep))
        {
            rStep = mStep;
        }
        else
        {
            lStep = mStep + 1;
        }
    }
    cap.set(CAP_PROP_ZOOM, direction * FOCUS_DIRECTION_INFTY * MAX_FOCUS_STEP);
    if (GlobalArgs.verbose)
    {
        cout << "Found minimal focus step = " << lStep << endl;
    }
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
    GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
    Canny(edges, edges, 0, 30, 3);

    std::for_each(edges.begin<uchar>(), edges.end<uchar>(), [&](uchar v)
    {
        sum += v != 0;
    });

    return (double) sum / (double) size;
}

int correctFocus(bool lastSucceeded, FocusState & state, double rate)
{
    state.lastDirectionChange++;
    if (GlobalArgs.verbose)
        cout << "RATE =" << rate << endl;
    double rateDelta = rate - state.rate;
    if (!lastSucceeded)
    {
        // Focus at limit or other problem, change the direction.
        state.direction *= -1;
        state.lastDirectionChange = 0;
    }
    else
    {
        if (rate < epsylon)
        { // It's hard to say anything
            state.step = FOCUS_STEP;
        }
        else if (rateDelta < epsylon)
        { // Wrong direction ?
            state.direction *= -1;
            state.step *= 0.75;
            state.lastDirectionChange = 0;
        }
        else
        { // Good direction.
            if ((state.rateMax > rate) && (state.lastDirectionChange > 3))
            { // I've done 3 steps without improvement, go back to max.
                state.direction = state.stepToLastMax >= 0 ? 1 : -1;
                int stepToMax = abs(state.stepToLastMax);
                state.stepToLastMax = 0;
                state.lastDirectionChange = 0; // Like reset.
                return stepToMax;
            }
        }
    }
    // Update state.
    state.rate = rate;
    state.stepToLastMax += state.direction * state.step;
    if (rate > state.rateMax)
    {
        state.stepToLastMax = 0;
        state.rateMax = rate;
    }
    return state.step;
}

void showHelp(const char * pName)
{
    cout << "usage " << pName << ": [OPTIONS] DEVICE_NAME\n\n"
            "OPTIONS:\n"
            "\t-h\t\treturns this help message,\n"
            "\t-o FILENAME\tsave output video in file (MJPEG only),\n"
            "\t-f FPS\t\tframes per second in output video,\n"
            "\t-m\t\tmeasure exposition\n"
            "\t\t\t(returns rates from closest focus to INTY\n"
            "\t\t\tfor every minimum step),\n"
            "\t-d INT\tset minimum focus step,\n"
            "\t-v\t\tverbose mode.\n\n\n"
            "DEVICE_NAME\t\tis your digital camera model substring.\n\n\n"
            "On runtime you can use keys to control:\n"
            "\tk:\t- focus out,\n"
            "\tj:\t- focus in,\n"
            "\t,:\t- focus to the closest point,\n"
            "\t.:\t- focus to infinity,\n"
            "\tr:\t- reset autofocus state,\n"
            "\tf:\t- switch autofocus on/off,\n"
            "\tq:\t- quit.\n";
}

bool parseArguments(int argc, char ** argv)
{
    int index;
    int c;

    opterr = 0;
    while ((c = getopt(argc, argv, "ho:f:mvd:")) != -1)
        switch (c)
        {
        case 'h':
            showHelp(argv[0]);
            exit(0);
        case 'o':
            GlobalArgs.output = optarg;
            break;
        case 'f':
            GlobalArgs.fps = atoi(optarg);
            break;
        case 'v':
            GlobalArgs.verbose = true;
            break;
        case 'm':
            GlobalArgs.measure = true;
            break;
        case 'd':
            GlobalArgs.minimumFocusStep = atoi(optarg);
            break;
        case '?':
            if (optopt == 'o' || optopt == 'f' || optopt == 'd')
                cerr << "Option `-" << ((char) optopt)
                        << "` requires an argument." << endl;
            cerr << "Unknown option `-" << ((char) optopt) << "`." << endl;
            return false;
        default:
            abort();
        }

    if (optind < argc)
    {
        GlobalArgs.deviceName = argv[optind];
    }

    for (index = optind + 1; index < argc; index++)
        cerr << "Omitting argument " << argv[index] << endl;
    return true;
}

int main(int argc, char ** argv)
{
    if (!parseArguments(argc, argv))
    {
        showHelp(argv[0]);
        return -1;
    }
    VideoCapture cap(GlobalArgs.deviceName);
    if (!cap.isOpened())
    {
        cout << "Cannot find device " << GlobalArgs.deviceName << endl;
        showHelp(argv[0]);
        return -1;
    }
    VideoWriter videoWriter;

    Mat frame;
    FocusState state;
    bool focus = true;
    bool lastSucceeded = true;
    namedWindow(windowOriginal, 1);

    // Get settings:
    if (GlobalArgs.verbose)
    {
        cout << "List of camera settings: " << endl
                << (const char *) (intptr_t) cap.get(
                        CAP_PROP_GPHOTO2_WIDGET_ENUMERATE) << endl;
        cap.set(CAP_PROP_GPHOTO2_COLLECT_MSGS, true);
    }

    cap.set(CAP_PROP_GPHOTO2_PREVIEW, true);
    cap.set(CAP_PROP_VIEWFINDER, true);
    cap >> frame; // To check PREVIEW output Size.
    if (GlobalArgs.output != NULL)
    {
        Size S = Size((int) cap.get(CAP_PROP_FRAME_WIDTH),
                (int) cap.get(CAP_PROP_FRAME_HEIGHT));
        videoWriter.open(GlobalArgs.output, CV_FOURCC('M', 'J', 'P', 'G'),
                GlobalArgs.fps, S, true);
        if (!videoWriter.isOpened())
        {
            cerr << "Cannot open output file " << GlobalArgs.output << endl;
            showHelp(argv[0]);
            return -1;
        }
    }

    if (GlobalArgs.minimumFocusStep == 0)
        state.minFocusStep = findMinFocusStep(cap, FOCUS_STEP / 16,
                -FOCUS_DIRECTION_INFTY);
    else
        state.minFocusStep = GlobalArgs.minimumFocusStep;
    focusDriveEnd(cap, -FOCUS_DIRECTION_INFTY); // Start with closest

    char key = 0;
    while (key != 'q' && key != 27 /*ESC*/)
    {
        cap >> frame;
        if (GlobalArgs.output != NULL)
            videoWriter << frame;

        if (focus && !GlobalArgs.measure)
        {
            int stepToCorrect = 0;
            if (!(lastSucceeded = cap.set(CAP_PROP_ZOOM,
                    max(
                            stepToCorrect = correctFocus(lastSucceeded, state,
                                    rateFrame(frame)), state.minFocusStep)
                            * state.direction))
                    || (stepToCorrect < state.minFocusStep))
            {
                if (--GlobalArgs.breakLimit <= 0)
                {
                    focus = false;
                    state.step = state.minFocusStep * 4;
                    cout
                            << "Focused, you can press 'f' to improve with small step, "
                                    "or 'r' to reset." << endl;
                }
            }
            else
            {
                GlobalArgs.breakLimit = DEFAULT_BREAK_LIMIT;
            }
        }
        else if (GlobalArgs.measure)
        {
            double rate = rateFrame(frame);
            if (!cap.set(CAP_PROP_ZOOM, state.minFocusStep))
            {
                if (--GlobalArgs.breakLimit <= 0)
                    break;
            }
            else
            {
                cout << rate << endl;
            }
        }

        if ((focus || GlobalArgs.measure) && GlobalArgs.verbose)
        {
            cout << "STATE=\t" << state << endl;
            cout << "Output from camera: " << endl
                    << (const char *) (intptr_t) cap.get(
                            CAP_PROP_GPHOTO2_FLUSH_MSGS) << endl;
        }

        imshow(windowOriginal, frame);
        switch (key = waitKey(30))
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
            focus = true;
            state = FocusState();
            break;
        case 'f': // focus switch on/off
            focus ^= true;
        }
    }

    if (GlobalArgs.verbose)
    {
        cout << "Captured " << (int) cap.get(CAP_PROP_FRAME_COUNT) << " frames"
                << endl << "in " << (int) (cap.get(CAP_PROP_POS_MSEC) / 1e2)
                << " seconds," << endl << "at avg speed "
                << (cap.get(CAP_PROP_FPS)) << " fps." << endl;
    }

    return 0;
}
