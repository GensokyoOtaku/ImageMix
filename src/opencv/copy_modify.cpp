#include "lib_cv.h"

int MAXN_MIX = DEFAULT_MAXN_MIX;
int MIX_OFFSET = DEFAULT_MIX_OFFSET;
int R_OffsetFactor = DEFAULT_OFFSET;
int G_OffsetFactor = DEFAULT_OFFSET;
int B_OffsetFactor = DEFAULT_OFFSET;

int MIX_MODE = DEFAULT_MIX_MODE;
int output_mode = OUTPUT_PNG;
int R_CODE[1000];
int angle = 0;

Mat img;
HBITMAP bmp;
bool clip_lock = true;

bool Hbitmap2Mat(HBITMAP &hBmp, Mat &mat)
{
    BITMAP Bmp;

    GetObject(hBmp, sizeof(BITMAP), &Bmp);
    int nChannels = Bmp.bmBitsPixel == 1 ? 1 : Bmp.bmBitsPixel / 8;
    // int depth = Bmp.bmBitsPixel == 1 ? IPL_DEPTH_1U : IPL_DEPTH_8U;
    Mat res_mat;
    res_mat.create(cv::Size(Bmp.bmWidth, Bmp.bmHeight), CV_MAKETYPE(CV_8U, nChannels));
    GetBitmapBits(hBmp, Bmp.bmHeight * Bmp.bmWidth * nChannels, res_mat.data);
    mat = res_mat;
    return true;
}

bool Mat2HBitmap(HBITMAP &hBmp, Mat &mat)
{
    int nChannels = (mat.type() >> 3) - CV_8U + 1;
    // int iSize = mat.cols * mat.rows * nChannels;
    hBmp = CreateBitmap(mat.cols, mat.rows, 1, nChannels * 8, mat.data);

    return true;
}

uchar pixelOffset(uchar origin, int offsetFactor)
{
    if (origin == 0)
        return MIX_OFFSET;
    if (origin == 255)
        return 255 - MIX_OFFSET;
    return max(min(origin + MIX_OFFSET * offsetFactor, 255), 0);
}

void ImageMix(Mat &mat)
{
    int tot = 0;
    size_t r = mat.rows, c = mat.cols;
    for (size_t i = 0; i < r; i++)
    {
        uchar *data = mat.ptr<uchar>(i);
        for (size_t j = 0; j < c; j++)
        {
            if ((i + j + rand()) % 3 == 0)
            {
                tot++;
                if (MIX_OFFSET == 0)
                {
                    int r = rand() % 256;
                    int g = rand() % 256;
                    int b = rand() % 256;
                    (*data) = ((r & 1) ? r : 255 - r);
                    data++;
                    (*data) = ((g & 1) ? g : 255 - g);
                    data++;
                    (*data) = ((b & 1) ? b : 255 - b);
                    data++;
                }
                else
                {
                    *data = pixelOffset(*data, R_OffsetFactor);
                    data++;
                    *data = pixelOffset(*data, G_OffsetFactor);
                    data++;
                    *data = pixelOffset(*data, B_OffsetFactor);
                    data++;
                }
            }
            if (tot >= MAXN_MIX)
                break;
        }
        if (tot >= MAXN_MIX)
            break;
    }
    LOG("changed pixels: ")
    LOG(tot);
    // cout << tot << "\n";
}

void getImgFormClipboard(HWND hwnd)
{
    if (clip_lock)
    {
        clip_lock = false;
        return;
    }
    if (!IsClipboardFormatAvailable(CF_BITMAP))
    {
        LOG("Not Bitmap");
        return;
    }
    if (!OpenClipboard(hwnd))
    {
        LOG("Clipboard not available");
        return;
    }

    bmp = (HBITMAP)GetClipboardData(CF_BITMAP);
    if (bmp != NULL)
    {
        LOG("---------------------------------");
        LOG("Get clipboard data");
        Hbitmap2Mat(bmp, img);
        EmptyClipboard();
        switch (MIX_MODE)
        {
        case DEFAULT_MIX_MODE:
            ImageMix(img);
            break;
        case COCKROACH_MODE:
            img = imread("./image/cockroach.png", CV_8UC4);
            // imshow("test", img);
            break;
        case TANK_MODE:
            img = getMirageTank(img);
            break;
        default:
            break;
        }
        if (angle)
            rotate(img, img, ROTATE_ANGLE);

        Mat temp = Mat();
        if (output_mode == OUTPUT_JPG)
            cvtColor(img, temp, COLOR_BGRA2BGR);

        if (output_mode == OUTPUT_PNG)
            Mat2HBitmap(bmp, img);
        else
            Mat2HBitmap(bmp, temp);

        // imshow("test", img);
        clip_lock = true;
        SetClipboardData(CF_BITMAP, bmp);
        bmp = NULL;
        img = Mat::zeros(cv::Size(1, 1), CV_8UC4);
        LOG("Set clipboard data");
        LOG("---------------------------------");
    }
    else
    {
        LOG("Not get clipboard data");
    }

    CloseClipboard();
}