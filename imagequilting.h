/*
 * Created by Kevin Lai
 * Date: 2017.07.01
 *
 * Image quilting functions
 *
 */

#ifndef IMAGEQUILTING_H
#define IMAGEQUILTING_H

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <queue>
#include <time.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <QImage>
#include <QPixmap>
#include <QDebug>
#include <vector>

using array2D = std::vector< std::vector< int > >;

using namespace std;

class ImageQuilting
{
public:
    ImageQuilting();
    ~ImageQuilting();

    void initParams(bool _useconv, bool _simple, bool _show_every_pic);
    void synthesize(QImage &imgin, QImage &imgout, int _tilesize, int _num_tiles, int _overlap, bool _useconv);
    cv::Mat getxcorr2(cv::Mat &imgA, cv::Mat &imgB);

    cv::Mat qimage_to_mat(QImage &imgin, bool inCloneImageData);
    QImage mat_to_qimage(cv::Mat &mat);


    // utility functions
    cv::Mat find_candidates(cv::Mat &X, double _best);
    cv::Mat ssd(cv::Mat &X, cv::Mat &Y);
    cv::Mat mincut(cv::Mat &X, int _direction);
    cv::Mat filtered_write(cv::Mat &A, cv::Mat &B, cv::Mat &M);

    void find_min_max(cv::Mat &X, double &_minVal, double &_maxVal);
    void ind2sub(cv::Mat &X, int _idx, int &_sub1, int &_sub2);
    void _initParameters( const std::string &_filename );

    double find_min(cv::Mat X);
    double myssd(cv::Mat &X);

private:
    cv::Mat input_image;
    cv::Mat output_image;
    int tilesize;
    int overlap;
    double err;

    int startI;
    int startJ;
    int endI;
    int endJ;

    //double distances[][];
    int input_height;
    int input_width;

    int num_tiles;
    int output_width;
    int output_height;

    cv::Rect roi;
    cv::Mat output_image_roi;
    cv::Mat input_image_roi;

    int m_useconv;
    int m_complex;
    int m_show_every_pic;
};

#endif // IMAGEQUILTING_H
