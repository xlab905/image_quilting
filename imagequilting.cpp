#include <imagequilting.h>
#include <valarray>
#include <algorithm>
#include <cstdlib>
// for loading parameters from xml
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
// for loading parameters from xml

using namespace cv;

ImageQuilting::ImageQuilting()
{
}

ImageQuilting::~ImageQuilting()
{
}

cv::Mat ImageQuilting::qimage_to_mat(QImage &imgin, bool inCloneImageData = true)
{
    switch ( imgin.format() )
     {
        // 8-bit, 4 channel
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
        {
           cv::Mat  mat( imgin.height(), imgin.width(),
                         CV_8UC4,
                         const_cast<uchar*>(imgin.bits()),
                         static_cast<size_t>(imgin.bytesPerLine())
                         );

           return (inCloneImageData ? mat.clone() : mat);
        }

        // 8-bit, 3 channel
        case QImage::Format_RGB32:
        case QImage::Format_RGB888:
        {
           if ( !inCloneImageData )
           {
              qWarning() << "ImageQuilting::qimage_to_mat() - Conversion requires cloning because we use a temporary QImage";
           }

           QImage   swapped = imgin;

           if ( imgin.format() == QImage::Format_RGB32 )
           {
              swapped = swapped.convertToFormat( QImage::Format_RGB888 );
           }

           swapped = swapped.rgbSwapped();

           return cv::Mat( swapped.height(), swapped.width(),
                           CV_8UC3,
                           const_cast<uchar*>(swapped.bits()),
                           static_cast<size_t>(swapped.bytesPerLine())
                           ).clone();
        }

        // 8-bit, 1 channel
        case QImage::Format_Indexed8:
        {
           cv::Mat  mat( imgin.height(), imgin.width(),
                         CV_8UC1,
                         const_cast<uchar*>(imgin.bits()),
                         static_cast<size_t>(imgin.bytesPerLine())
                         );

           return (inCloneImageData ? mat.clone() : mat);
        }

        default:
           qWarning() << "ImageQuilting::qimage_to_mat() - QImage format not handled in switch:" << imgin.format();
           break;
     }

     return cv::Mat();
}

QImage ImageQuilting::mat_to_qimage(cv::Mat &mat)
{
    // NOTE: This does not cover all cases - it should be easy to add new ones as required.
    switch ( mat.type() )
    {
        // 8-bit, 4 channel
        case CV_8UC4:
        {
            QImage image( mat.data,
                       mat.cols, mat.rows,
                       static_cast<int>(mat.step),
                       QImage::Format_ARGB32 );

            return image;
        }

        // 8-bit, 3 channel
        case CV_8UC3:
        {
            QImage image( mat.data,
                       mat.cols, mat.rows,
                       static_cast<int>(mat.step),
                       QImage::Format_RGB888 );

            return image.rgbSwapped();
        }

        // 8-bit, 1 channel
        case CV_8UC1:
        {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
            QImage image( mat.data,
                       mat.cols, mat.rows,
                       static_cast<int>(mat.step),
                       QImage::Format_Grayscale8 );
#else
            static QVector<QRgb>  sColorTable;

            // only create our color table the first time
            if ( sColorTable.isEmpty() )
            {
                sColorTable.resize( 256 );

                for ( int i = 0; i < 256; ++i )
                {
                    sColorTable[i] = qRgb( i, i, i );
                }
            }

            QImage image( mat.data,
                       mat.cols, mat.rows,
                       static_cast<int>(mat.step),
                       QImage::Format_Indexed8 );

            image.setColorTable( sColorTable );
#endif

            return image;
        }

        default:
            qWarning() << "ImageQuilting::mat_to_qimage() - cv::Mat image type not handled in switch:" << mat.type();
            break;
    }
    return QImage();
}

void ImageQuilting::synthesize(QImage &imgin, QImage &imgout, int _tilesize, int _num_tiles, int _overlap, bool _useconv)
{
    // initialize the variables
    input_image = qimage_to_mat(imgin);
    tilesize = _tilesize;
    overlap = _overlap;
    num_tiles = _num_tiles;
    m_useconv = _useconv;
    err = 0.002;

    input_height = input_image.rows;
    input_width = input_image.cols;
    cv::Mat distances = cv::Mat::zeros(input_height-tilesize, input_width-tilesize, CV_64F);
    cv::Mat distances_tmp = distances.clone();
    cv::Mat Z, Z_tmp;
    cv::Mat candidates;
    cv::Mat M;
    cv::Mat E, C;

    int destsize = num_tiles*tilesize - (num_tiles-1)*overlap;
    output_image = cv::Mat::zeros(destsize, destsize, CV_8UC3);
    //std::cout << "output size = [" << output_image.rows << "," << output_image.cols << "]" << std::endl;

    double best;

    for(int i=0; i<num_tiles; i++)
    {
        for(int j=0; j<num_tiles; j++)
        {
            std::cout << "[i,j] = [" << i << "," << j << "]" << std::endl;
            startI = (i)*tilesize - (i)*overlap;
            startJ = (j)*tilesize - (j)*overlap;
            endI = startI + tilesize - 1;
            endJ = startJ + tilesize - 1;

            if( m_useconv==0 )
            {
                // compute the distances from the template to target for all i and j
                for(int a=0; a<distances.rows; a++)
                {
                    cv::Mat v1;
                    output_image(Rect(startJ,startI,endJ-startJ+1,endI-startI+1)).copyTo(v1);
                    cv::Mat v1_flatten = v1.reshape(1,v1.rows*v1.cols*v1.channels());
                    for(int b=0; b<distances.cols; b++)
                    {
                        cv::Mat v2;
                        input_image(Rect(b,a,tilesize,tilesize)).copyTo(v2);
                        cv::Mat v2_flatten = v2.reshape(1,v2.rows*v2.cols*v2.channels());

                        v1_flatten.convertTo(v1_flatten, CV_64F, 1, 0);
                        v2_flatten.convertTo(v2_flatten, CV_64F, 1, 0);
                        cv::Mat v1_flatten_thresh = ((v1_flatten>0)/255);
                        v1_flatten_thresh.convertTo(v1_flatten_thresh, CV_64F, 1, 0);
                        cv::Mat myssd_input = v1_flatten_thresh.mul((v1_flatten - v2_flatten));

                        distances.at<double>(a,b) = myssd( myssd_input );
                    }
                }
            }
            else
            {
                // compute the distances from the source to the left overlap region
                if(j>0)
                {
                    output_image(Rect(startJ,startI,overlap,endI-startI+1)).copyTo(output_image_roi);
                    distances_tmp = ssd(input_image,output_image_roi);

                    // crop distances
                    distances_tmp(cv::Rect(0,0,distances_tmp.cols-tilesize+overlap,distances_tmp.rows)).copyTo(distances);
                }

                // compute the distances from the source to the top overlap region
                if(i>0)
                {
                    output_image(Rect(startJ,startI,endJ-startJ+1,overlap)).copyTo(output_image_roi);
                    Z_tmp = ssd(input_image,output_image_roi);

                    // crop Z
                    Z_tmp(cv::Rect(0,0,Z_tmp.cols,Z_tmp.rows-tilesize+overlap)).copyTo(Z);

                    if(j>0)
                    {
                        distances = distances + Z;
                    }
                    else
                    {
                        distances = Z;
                    }
                }

                // if both are greater, compute the distance of the overlap
                if((i>0) && (j>0))
                {
                    output_image(Rect(startJ,startI,overlap,overlap)).copyTo(output_image_roi);
                    Z_tmp = ssd(input_image,output_image_roi);

                    // crop Z
                    Z_tmp(cv::Rect(0,0,Z_tmp.cols-tilesize+overlap,Z_tmp.rows-tilesize+overlap)).copyTo(Z);
                    distances = distances - Z;
                }

                output_image_roi.release();
                Z_tmp.release();

            }
            //std::cout << "distances = [" << distances.rows << ", " << distances.cols << "]" << std::endl;
            //std::cout << "distances = "<< std::endl << " "  << distances << std::endl << std::endl;
            // find the best candidates for the match
            best = find_min(distances);
            //std::cout << "best = " << best << std::endl;
            candidates = find_candidates(distances, best);

            //std::cout << "candidates = "<< std::endl << " "  << candidates << std::endl << std::endl;
            //std::cout << "candidates = [" << candidates.rows << ", " << candidates.cols << "]" << std::endl;

            // generate uniformly random number
            double random_number = ((double) rand() / (RAND_MAX));
            //std::cout << "random_number = " << random_number << std::endl;

            //std::cout << std::floor(random_number*candidates.cols) << std::endl;
            //std::cout << candidates.at<double>(0,0) << std::endl;
            int idx = candidates.at<double>(0,(std::floor(random_number*candidates.cols)));
            std::cout << "idx = " << idx << std::endl;

            int sub1, sub2;
            ind2sub(distances, idx, sub1, sub2);
            //std::cout << "sub = [" << sub1 << "," << sub2 << "]" << std::endl;
            std::cout << "pick tile [" << sub1 << "," << sub2 << "] out of " << candidates.cols << " candidates.";
            std::cout << " best error = " << best << std::endl;

            if(!m_complex)
            {
                // simple synthesize, random copy paste from the sample texture
                input_image(Rect(sub2,sub1,tilesize,tilesize)).copyTo(output_image(Rect(startJ,startI,tilesize,tilesize)));
            }
            else
            {
                // initialize the mask to all ones
                M = Mat::ones(tilesize, tilesize, CV_64F);

                // if we have a left overlap
                if(j>0)
                {
                    // compute the ssd in the border region
                    // extract the first channel of the input and output, and convert them to CV_64F
                    cv::Mat input_split[3], output_split[3];
                    cv::split(input_image(Rect(sub2,sub1,overlap,tilesize)), input_split);
                    cv::split(output_image(Rect(startJ,startI,overlap,endI-startI+1)), output_split);
                    // need to convert to double, so that we can calculate
                    input_split[0].convertTo(input_split[0], CV_64F, 1, 0);
                    output_split[0].convertTo(output_split[0], CV_64F, 1, 0);
                    E =  input_split[0] - output_split[0];
                    cv::Mat E_2 = E;
                    cv::pow(E, 2, E_2);

                    // compute the mincut array
                    C = mincut(E_2, 0);
                    //std::cout << "C = [" << C.rows << ", " << C.cols << "]" << std::endl;
                    //std::cout << "C = "<< std::endl << " "  << C << std::endl << std::endl;

                    // compute the mask and write to the destination
                    cv::Mat C_thresh_vertical = ((C>=0)/255);
                    C_thresh_vertical.convertTo(C_thresh_vertical, CV_64F, 1, 0);
                    //std::cout << "C_thresh_vertical = "<< std::endl << " "  << C_thresh_vertical << std::endl << std::endl;
                    C_thresh_vertical.copyTo(M(Rect(0,0,overlap,M.cols)));
                    //std::cout << "M = "<< std::endl << " "  << M << std::endl << std::endl;
                }

                if(i>0)
                {
                    // compute the ssd in the border region
                    cv::Mat input_split[3], output_split[3];
                    cv::split(input_image(Rect(sub2,sub1,tilesize,overlap)), input_split);
                    cv::split(output_image(Rect(startJ,startI,endJ-startJ+1,overlap)), output_split);
                    input_split[0].convertTo(input_split[0], CV_64F, 1, 0);
                    output_split[0].convertTo(output_split[0], CV_64F, 1, 0);
                    E =  input_split[0] - output_split[0];
                    cv::Mat E_2 = E;
                    cv::pow(E, 2, E_2);

                    // compute the mincut array
                    C = mincut(E_2,1);
                    //std::cout << "C = "<< std::endl << " "  << C << std::endl << std::endl;

                    // compute the mask and write to the destination
                    cv::Mat C_thresh_horizontal = ((C>=0)/255);
                    C_thresh_horizontal.convertTo(C_thresh_horizontal, CV_64F, 1, 0);
                    M(Rect(0,0,M.rows,overlap)) = M(Rect(0,0,M.rows,overlap)).mul(C_thresh_horizontal);
                }
                //std::cout << "M = "<< std::endl << " "  << M << std::endl << std::endl;
                if((i==0)&&(j==0)) // synthesizing the first one, just copy paste
                {
                    input_image(Rect(sub2,sub1,tilesize,tilesize)).copyTo(output_image(Rect(startJ,startI,tilesize,tilesize)));
                }
                else
                {
                    // write to the destination using the mask
                    cv::Mat A = output_image(Rect(startJ,startI,endJ-startJ+1,endI-startI+1));
                    cv::Mat B = input_image(Rect(sub2,sub1,tilesize,tilesize));

                    output_image(Rect(startJ,startI,endJ-startJ+1,endI-startI+1)) = filtered_write(A, B, M);
                }


            }
            if(!m_useconv)
            {

            }
            else
            {
                distances.release();
            }

            candidates.release();
            Z.release();
            E.release();
            C.release();
            M.release();
            if(m_show_every_pic)
            {
                cv::imshow("output_image", output_image);
                cv::waitKey();
                cv::destroyAllWindows();
            }
        }
    }
    std::cout << "DONE!" << std::endl;

    // convert cv::Mat to QImage
    imgout = mat_to_qimage(output_image);
}

void ImageQuilting::ind2sub(cv::Mat &X, int _idx, int &_sub1, int &_sub2)
{
    _sub1 = std::floor(_idx/((double)X.cols));
    _sub2 = _idx - X.cols*std::floor(_idx/((double)X.cols));
}

cv::Mat ImageQuilting::find_candidates(cv::Mat &X, double _best)
{
    //std::cout << "find candidates" << std::endl;
    int count = 0;
    cv::Mat X_flatten = X.reshape(1,X.rows*X.cols*X.channels()).t();

    // first calculate how many candidates are within the range
    for(int i=0; i<X_flatten.cols; i++)
    {        
        if(X.at<double>(0,i)<=(_best*(err+1)))
        {
            count++;
        }
    }

    // define candidates size as the number calculated above
    cv::Mat _candidates = cv::Mat::zeros(1, count, CV_64F);
    count = 0;

    // assign the index of candidate to the candidate matrix
    for(int i=0; i<X_flatten.cols; i++)
    {
        if(X.at<double>(0,i)<=(_best*(err+1)))
        {
            _candidates.at<double>(0,count) = i;
            count++;
        }
    }
    //std::cout << "candidates = "<< std::endl << " "  << _candidates << std::endl << std::endl;
    return _candidates;
}

cv::Mat ImageQuilting::filtered_write(cv::Mat &A, cv::Mat &B, cv::Mat &M)
{
    cv::Mat R = A;

    cv::Mat A_split[3], B_split[3], R_split[3];
    cv::split(A,A_split);
    cv::split(B,B_split);
    cv::split(R,R_split);

    for(int k=0; k<A.channels(); k++)
    {
        R_split[k] = A_split[k].mul((M==0)/255) + B_split[k].mul(((M==1)/255));
    }

    cv::merge(R_split, 3, R);
    return R;
}

void ImageQuilting::find_min_max(cv::Mat &X, double &_minVal, double &_maxVal)
{
    cv::Point p1, p2;
    cv::minMaxLoc(X, &_minVal, &_maxVal, &p1, &p2);
}

cv::Mat ImageQuilting::ssd(cv::Mat &X, cv::Mat &Y)
{
    std::cout << "calculating ssd" << std::endl;

    // create filter K with all values set to 1
    cv::Mat K = cv::Mat::ones(Y.rows, Y.cols, CV_64F);
    Point Kanchor(-1,-1);
    //Point Kanchor(K.cols - K.cols/2 - 1, K.rows - K.rows/2 - 1);
    //cv::flip(K, K, -1);

    std::vector<cv::Mat> X_split, Y_split;
    cv::split(X,X_split);
    cv::split(Y,Y_split);

    cv::Mat A, B, Z, a2, ab, ab_tmp, a2_tmp, Z_tmp;
    double b2;

    // compute for each channel, then sum them up
    for(int k=0; k<X.channels(); k++)
    {
        A = X_split[k];
        B = Y_split[k];
        //std::cout << "size of B = [" << B.rows << ", " << B.cols << "]" << std::endl;
        //std::cout << "B = "<< std::endl << " "  << B << std::endl << std::endl;

        // convert to CV_64F(double) for better calculation
        A.convertTo(A, CV_64F, 1, 0);
        B.convertTo(B, CV_64F, 1, 0);

        cv::Mat A_2 = A;
        cv::pow(A, 2, A_2);

        // calculate sum of squares of A
        cv::filter2D(A_2, a2_tmp, -1, K, Kanchor, 0, cv::BORDER_CONSTANT);

        // extract region of interest
        a2_tmp(Rect(std::ceil(K.cols/2), std::ceil(K.rows/2), A.cols-K.cols+1, A.rows-K.rows+1)).copyTo(a2);
        //std::cout << "size of a2 = [" << a2.rows << ", " << a2.cols << "]" << std::endl;
        //std::cout << "a2 = "<< std::endl << " "  << a2 << std::endl << std::endl;

        // calculate sum of squares of B
        cv::Mat B_2 = B;
        cv::pow(B, 2, B_2);
        b2 = cv::sum(B_2)[0];

        // normalize B, because the sum of the filter must be 1
        cv::normalize(B, B, 0, 1, cv::NORM_MINMAX, CV_64F);
        Point Banchor(-1,-1);
        //Point Banchor(B.cols-B.cols/2-1, B.rows-B.rows/2-1);
        //cv::flip(B, B, -1);
        //std::cout << "B = "<< std::endl << " "  << B << std::endl << std::endl;

        // calculate sum of squares of AB
        cv::filter2D(A, ab_tmp, -1, B, Banchor, 0, cv::BORDER_CONSTANT);

        // extract region of interest
        ab_tmp(Rect(std::ceil(B.cols/2), std::ceil(B.rows/2), A.cols-B.cols+1, A.rows-B.rows+1)).copyTo(ab);
        ab = ab*2;
        //std::cout << "ab[0][0] = " << ab.at<double>(0,0) << std::endl;
        //std::cout << "size of ab = [" << ab.rows << ", " << ab.cols << "]" << std::endl;
        //std::cout << "ab = "<< std::endl << " "  << ab << std::endl << std::endl;

        if(k==0)
        {
            Z_tmp = ((a2 - ab) + b2);
            Z_tmp.copyTo(Z);
            //std::cout << "a2[0][0] = " << a2.at<double>(0,0) << std::endl;
            //std::cout << "ab[0][0] = " << ab.at<double>(0,0) << std::endl;
            //std::cout << "b2 = "<< b2 << std::endl;
            //std::cout << "Z[0][0] = " << Z.at<double>(0,0) << std::endl;
        }
        else
        {
            Z_tmp = Z + ((a2 - ab) + b2);
            Z_tmp.copyTo(Z);
            //std::cout << "a2[0][0] = " << a2.at<double>(0,0) << std::endl;
            //std::cout << "ab[0][0] = " << ab.at<double>(0,0) << std::endl;
            //std::cout << "b2 = "<< b2 << std::endl;
        }


    }

    return Z;
}

double ImageQuilting::myssd(cv::Mat &X)
{
    cv::Mat X_2 = X;
    cv::pow(X, 2, X_2);
    double y = cv::sum(X_2)[0];
    return y;
}

cv::Mat ImageQuilting::mincut(cv::Mat &X, int _direction)
{
    //std::cout << "X = "<< std::endl << " "  << X << std::endl << std::endl;
    if(_direction == 1)
    {
        X = X.t();
    }

    // allocate the current cost array, and set first row to first row of X
    cv::Mat E = X;//.clone();
    //cv::Mat E = cv::Mat::zeros(X.rows, X.cols, CV_64F);
    //X.copyTo(E);
    //std::cout << "E = "<< std::endl << " "  << E << std::endl << std::endl;
    int endE = E.cols-1;

    // starting with the second array, compute the path costs until the end(cumulative error)
    for(int i=1; i<E.rows; i++)
    {
        E.at<double>(i,0) = X.at<double>(i,0) + std::min(E.at<double>(i-1,0),E.at<double>(i-1,1));
        for(int j=1; j<E.cols-1; j++)
        {
            E.at<double>(i,j) = X.at<double>(i,j) + std::min({E.at<double>(i-1,j-1),E.at<double>(i-1,j),E.at<double>(i-1,j+1)});
        }
        E.at<double>(i,endE) = X.at<double>(i,endE) + std::min(E.at<double>(i-1,endE-1),E.at<double>(i-1,endE));
    }
    //std::cout << "E = "<< std::endl << " "  << E << std::endl << std::endl;
    //std::cout << "E.row(E.rows-1) = "<< std::endl << " "  << E.row(E.rows-1) << std::endl << std::endl;

    // back trace to find the cut
    cv::Mat C = cv::Mat::zeros(X.rows, X.cols, CV_64F);
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(E.row(E.rows-1), &minVal, &maxVal, &minLoc, &maxLoc);
    int idx = minLoc.x;
    //std::cout << "idx = " << idx << std::endl;

    int k = E.rows-1;
    for(int l=0; l<E.cols; l++)
    {
        if(l<idx){ C.at<double>(k,l) = -1; }
        else if(l==idx){ C.at<double>(k,l) = 0; }
        else{ C.at<double>(k,l) = +1; }
    }

    for(int i=E.rows-2; i>=0; --i)
    {
        for(int j=0; j<E.cols; j++)
        {
            if((idx>0)&&(E.at<double>(i,idx-1)==find_min(E(Rect(idx-1,i,std::min(idx+1,E.cols-1)-(idx-1)+1,1)))))
            {
                idx = idx-1;
            }
            else if((idx<E.cols-1)&&(E.at<double>(i,idx+1)==find_min(E(Rect(std::max(idx-1,0),i,idx+1-std::max(idx-1,0)+1,1)))))
            {

                idx = idx+1;
            }
            //std::cout << "idx = " << idx << std::endl;
            for(int l=0; l<E.cols; l++)
            {
                if(l<idx){ C.at<double>(i,l) = -1; }
                else if(l==idx){ C.at<double>(i,l) = 0; }
                else{ C.at<double>(i,l) = +1; }
            }
        }
    }
    //std::cout << "C = "<< std::endl << " "  << C << std::endl << std::endl;
    //std::cout << "C = [" << C.rows << ", " << C.cols << "]" << std::endl;
    if(_direction == 1)
    {
        C = C.t();
    }
    //std::cout << "C = [" << C.rows << ", " << C.cols << "]" << std::endl;
    return C;
}

double ImageQuilting::find_min(cv::Mat X)
{
    double _minVal, _maxVal;
    cv::minMaxIdx(X, &_minVal, &_maxVal);
    return _minVal;
}

void ImageQuilting::initParams( bool _useconv, bool _complex, bool _show_every_pic)
{
    m_useconv = _useconv;
    m_complex = _complex;
    m_show_every_pic = _show_every_pic;
}
