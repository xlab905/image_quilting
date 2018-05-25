/*
 * Created by Kevin Lai
 * Date: 2017.06.30
 *
 * This file defines the input output relations of the form
 *
 */

#ifndef IO_H
#define IO_H

#include <QWidget>
#include <QMap>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <time.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <imagequilting.h>


class QPushButton;
class QTextEdit;
class QLabel;
class QSlider;
class QSpinBox;
class QCheckBox;

class IO : public QWidget
{
    Q_OBJECT

public:
    IO(QWidget *parent = 0);

public slots:
    void loadImageButton();
    void saveImageButton();
    void synthesizeImageButton();
    void resetAllButton();

private:
    ImageQuilting imagequilting;

    QPushButton *loadButton;
    QPushButton *saveButton;
    QPushButton *synButton;
    QPushButton *resetButton;

    QCheckBox *convCheckBox;
    QCheckBox *complexCheckBox;
    QCheckBox *debugCheckBox;

    QLabel *srcImage;
    QLabel *resImage;
    QLabel *tileSize;
    QLabel *overlapRegion;
    QLabel *numTile;
    QLabel *comInfoText;
    QLabel *comInfo;
    QLabel *inSizeInfoText;
    QLabel *outSizeInfoText;
    QLabel *inSizeInfo;
    QLabel *outSizeInfo;

    QSlider *tileSizeBar;
    QSpinBox *tileSizeSpinBox;
    QSlider *overlapRegionBar;
    QSpinBox *overlapRegionSpinBox;
    QSlider *numTileBar;
    QSpinBox *numTileSpinBox;

    QImage input_image;
    QImage output_image;

};


#endif // IO_H
