#include "lib_cv.h"

vector<int> compression_params;
int div_rgb = DIV;

/* 像素取值范围伸缩 */
int rangeConvert(int src, int hi, int lo)
{
    return int(src * 1.0 / 255 * (hi - lo) + lo);
}
int div2(int src, int lo)
{
    return src / 2; //+ lo / 2;
}
int halfRange(int hi, int lo)
{
    return 127;
    // return (hi + lo) / 2;
}
void init()
{
    if (compression_params.empty())
    {
        compression_params.push_back(IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(9);
    }
}
// 色阶提升
Mat ColorScaleUP(Mat img)
{
    int r = img.rows, c = img.cols;
    Mat res = Mat(r, c, CV_8UC3);
    for (int j = 0; j < r; j++)
    {
        uchar *src = img.ptr<uchar>(j);
        uchar *dst = res.ptr<uchar>(j);
        for (int i = 0; i < c; i++)
        {
            dst[0] = src[0] / 2 + 128;
            dst[1] = src[1] / 2 + 128;
            dst[2] = src[2] / 2 + 128;
            src += 3;
            dst += 3;
        }
    }
    return res;
}
// 色阶降低
Mat ColorScaleDOWN(Mat img)
{
    int r = img.rows, c = img.cols;
    Mat res = Mat(r, c, CV_8UC3);
    for (int j = 0; j < r; j++)
    {
        uchar *src = img.ptr<uchar>(j);
        uchar *dst = res.ptr<uchar>(j);
        for (int i = 0; i < c; i++)
        {
            dst[0] = src[0] / 2;
            dst[1] = src[1] / 2;
            dst[2] = src[2] / 2;
            src += 4;
            dst += 3;
        }
    }
    return res;
}
// 仅对不同的像素改变色阶
void ColorScaleConvert(Mat srcA, Mat srcB, Mat &dstA, Mat &dstB)
{
    int r1 = srcA.rows, c1 = srcA.cols,
        r2 = srcB.rows, c2 = srcB.cols;

    for (int j = 0; j < min(r1, r2); j++)
    {
        uchar *s1 = srcA.ptr<uchar>(j);
        uchar *d1 = dstA.ptr<uchar>(j);
        uchar *s2 = srcB.ptr<uchar>(j);
        uchar *d2 = dstB.ptr<uchar>(j);
        for (int i = 0; i < min(c1, c2); i++)
        {
            d1[0] = d2[0] = s2[0];
            d1[1] = d2[1] = s2[1];
            d1[2] = d2[2] = s2[2];

            // if (d1[0] >= 128 && d1[1] >= 128 && d1[2] >= 128) {
            // d1[0] = min(d1[0] + d1[0] / 10, 255);
            // d1[1] = min(d1[1] + d1[1] / 10, 255);
            // d1[2] = min(d1[2] + d1[2] / 10, 255);
            //}
            d1[0] = d1[1] = d1[2] = 128;

            d2[0] = d2[0] / 2;
            d2[1] = d2[1] / 2;
            d2[2] = d2[2] / 2;

            s1 += 3;
            s2 += 4;
            d1 += 3;
            d2 += 3;
        }
    }
    return;
}
// 反相
Mat invert(Mat img)
{
    int r = img.rows, c = img.cols;
    Mat res = Mat(r, c, CV_8UC3);
    for (int j = 0; j < r; j++)
    {
        uchar *src = img.ptr<uchar>(j);
        uchar *dst = res.ptr<uchar>(j);
        for (int i = 0; i < c; i++)
        {
            dst[0] = 255 - src[0];
            dst[1] = 255 - src[1];
            dst[2] = 255 - src[2];
            src += 3;
            dst += 3;
        }
    }
    return res;
}
// 线性减淡（叠加）
Mat MatPlus(Mat A, Mat B)
{
    int r = min(A.rows, B.rows), c = min(A.cols, B.cols);
    Mat res = Mat(r, c, CV_8UC3);
    for (int j = 0; j < r; j++)
    {
        uchar *src1 = A.ptr<uchar>(j);
        uchar *src2 = B.ptr<uchar>(j);
        uchar *dst = res.ptr<uchar>(j);
        for (int i = 0; i < c; i++)
        {
            dst[0] = min(src1[0] + src2[0], 255);
            dst[1] = min(src1[1] + src2[1], 255);
            dst[2] = min(src1[2] + src2[2], 255);
            src1 += 3;
            src2 += 3;
            dst += 3;
        }
    }
    return res;
}
// 划分
Mat divide(Mat B, Mat O)
{
    int r = min(B.rows, O.rows), c = min(B.cols, O.cols);
    Mat res = Mat(r, c, CV_8UC3);
    for (int j = 0; j < r; j++)
    {
        uchar *src1 = B.ptr<uchar>(j);
        uchar *src2 = O.ptr<uchar>(j);
        uchar *dst = res.ptr<uchar>(j);
        for (int i = 0; i < c; i++)
        {
            dst[0] = src2[0] == 0 ? 255 : 255 * src1[0] / src2[0];
            dst[1] = src2[1] == 0 ? 255 : 255 * src1[1] / src2[1];
            dst[2] = src2[2] == 0 ? 255 : 255 * src1[2] / src2[2];
            src1 += 3;
            src2 += 3;
            dst += 3;
        }
    }
    return res;
}
// 设置透明度
Mat AlphaOpt(Mat img, Mat orc)
{
    int r = min(img.rows, orc.rows), c = min(img.cols, orc.cols);
    Mat res = Mat(r, c, CV_8UC4);
    for (int j = 0; j < r; j++)
    {
        uchar *src = img.ptr<uchar>(j);
        uchar *src2 = orc.ptr<uchar>(j);
        uchar *dst = res.ptr<uchar>(j);
        for (int i = 0; i < c; i++)
        {
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
            dst[3] = (src2[0] + src2[1] + src2[2]) / 3;
            src += 3;
            src2 += 3;
            dst += 4;
        }
    }
    return res;
}
// 黑白转换
Mat GrayScale(Mat img)
{
    int r = img.rows, c = img.cols;
    Mat res = Mat(r, c, CV_8UC4);
    for (int j = 0; j < r; j++)
    {
        uchar *src = img.ptr<uchar>(j);
        uchar *dst = res.ptr<uchar>(j);
        for (int i = 0; i < c; i++)
        {
            dst[0] = (src[0] + src[1] + src[2]) / 3;
            dst[1] = (src[0] + src[1] + src[2]) / 3;
            dst[2] = (src[0] + src[1] + src[2]) / 3;
            dst[3] = src[3];
            src += 4;
            dst += 4;
        }
    }
    return res;
}
Mat getMirageTank(Mat src)
{
    init();
    int r = src.rows, c = src.cols;
    Mat A(r, c, CV_8UC3, Scalar(0, 0, 0));
    Mat B(r, c, CV_8UC4, src.data);
    Mat _A(A.rows, A.cols, CV_8UC3);
    Mat _B(B.rows, B.cols, CV_8UC3);
    ColorScaleConvert(A, B, _A, _B);
    // imshow("_A", _A);
    // imshow("_B", _B);
    // waitKey(0);
    Mat _1subA = invert(_A);
    Mat O = MatPlus(_1subA, _B);
    Mat R = divide(_B, O);
    Mat tank = AlphaOpt(R, O);
    // imshow("tank", tank);
    imwrite("./image/tank.png", tank, compression_params);
    return tank;
}