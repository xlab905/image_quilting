/*
 * Created by Kevin Lai
 * Date: 2017.06.30
 *
 * This file defines the input output relations of the form
 *
 */

#include <QtWidgets>
#include <io.h>

// construct the form
IO::IO(QWidget *parent)
    : QWidget(parent)
{
    // define QPushButton objects
    loadButton = new QPushButton(tr("&Load Image"));
    loadButton->setToolTip(tr("Load image from file"));
    loadButton->setFixedSize(100,40);
    synButton = new QPushButton(tr("&Synthesize"));
    synButton->setToolTip(tr("Synthesizing image"));
    synButton->setFixedSize(100,40);
    synButton->setEnabled(false);
    saveButton = new QPushButton(tr("&Save Result"));
    saveButton->setToolTip(tr("Save result image to a file"));
    saveButton->setFixedSize(100,40);
    saveButton->setEnabled(false);
    resetButton = new QPushButton(tr("&Reset All"));
    resetButton->setToolTip(tr("Reset"));
    resetButton->setFixedSize(100,40);
    resetButton->setEnabled(false);

    // define labels
    srcImage = new QLabel(tr("Source Image"));
    srcImage->setAlignment(Qt::AlignCenter);
    srcImage->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    srcImage->setFixedSize(160,120);
    srcImage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    //srcImage->setGeometry(QRect(150,150,30,80));
    resImage = new QLabel(tr("Result Image"));
    resImage->setAlignment(Qt::AlignCenter);
    resImage->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    resImage->setFixedSize(516,387);
    resImage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    convCheckBox = new QCheckBox(tr("Use Convolution"));
    convCheckBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    convCheckBox->setChecked(true);
    complexCheckBox = new QCheckBox(tr("With Mincut"));
    complexCheckBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    complexCheckBox->setChecked(true);
    debugCheckBox = new QCheckBox(tr("Show Process"));
    debugCheckBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    debugCheckBox->setChecked(false);

    tileSize = new QLabel(tr("Tile Size"));
    tileSize->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    overlapRegion = new QLabel(tr("Overlap Region"));
    overlapRegion->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    numTile = new QLabel(tr("Tiles"));
    numTile->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    inSizeInfoText = new QLabel(tr("Input Size"));
    inSizeInfoText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    inSizeInfo = new QLabel(tr("null"));
    inSizeInfo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    outSizeInfoText = new QLabel(tr("Output Size"));
    outSizeInfoText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    outSizeInfo = new QLabel(tr("null"));
    outSizeInfo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    comInfoText = new QLabel(tr("Compute time"));
    comInfoText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    comInfo = new QLabel(tr("null"));
    comInfo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    // define slide bars
    tileSizeBar = new QSlider(Qt::Horizontal);
    tileSizeBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    tileSizeSpinBox = new QSpinBox;
    tileSizeSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    tileSizeSpinBox->setRange(1,90);
    tileSizeBar->setRange(1,90);

    overlapRegionBar = new QSlider(Qt::Horizontal);
    overlapRegionBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    overlapRegionSpinBox = new QSpinBox;
    overlapRegionSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    overlapRegionSpinBox->setRange(1,25);
    overlapRegionBar->setRange(1,25);

    numTileBar = new QSlider(Qt::Horizontal);
    numTileBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    numTileSpinBox = new QSpinBox;
    numTileSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    numTileSpinBox->setRange(3,15);
    numTileBar->setRange(3,15);

    // connect the click event to the buttons
    connect(loadButton, SIGNAL(clicked()), this, SLOT(loadImageButton()));
    connect(synButton, SIGNAL(clicked()), this, SLOT(synthesizeImageButton()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveImageButton()));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetAllButton()));
    connect(tileSizeSpinBox, SIGNAL(valueChanged(int)), tileSizeBar, SLOT(setValue(int)));
    connect(tileSizeBar, SIGNAL(valueChanged(int)), tileSizeSpinBox, SLOT(setValue(int)));
    tileSizeSpinBox->setValue(80);
    connect(overlapRegionSpinBox, SIGNAL(valueChanged(int)), overlapRegionBar, SLOT(setValue(int)));
    connect(overlapRegionBar, SIGNAL(valueChanged(int)), overlapRegionSpinBox, SLOT(setValue(int)));
    overlapRegionSpinBox->setValue(13);
    connect(numTileSpinBox, SIGNAL(valueChanged(int)), numTileBar, SLOT(setValue(int)));
    connect(numTileBar, SIGNAL(valueChanged(int)), numTileSpinBox, SLOT(setValue(int)));
    numTileSpinBox->setValue(5);

    // vertical button layout
    QVBoxLayout *buttonLayout1 = new QVBoxLayout;
    buttonLayout1->addWidget(loadButton);
    buttonLayout1->addWidget(synButton);
    buttonLayout1->addWidget(saveButton);
    buttonLayout1->addWidget(resetButton);
    buttonLayout1->setSpacing(15);

    QGridLayout *toolsLayout1 = new QGridLayout;
    toolsLayout1->addWidget(srcImage, 0, 0, Qt::AlignTop);
    toolsLayout1->addWidget(convCheckBox, 1, 0, Qt::AlignLeft);
    toolsLayout1->addWidget(complexCheckBox, 2, 0, Qt::AlignLeft);
    toolsLayout1->addWidget(debugCheckBox, 3, 0, Qt::AlignLeft);

    QGridLayout *toolsLayout2 = new QGridLayout;
    //toolsLayout->addWidget(convCheckBox, 0, 1);
    toolsLayout2->addWidget(tileSize, 0, 0);
    toolsLayout2->addWidget(tileSizeBar, 0, 1);
    toolsLayout2->addWidget(tileSizeSpinBox, 0, 2);
    toolsLayout2->addWidget(overlapRegion, 1, 0);
    toolsLayout2->addWidget(overlapRegionBar, 1, 1);
    toolsLayout2->addWidget(overlapRegionSpinBox, 1, 2);
    toolsLayout2->addWidget(numTile, 2, 0);
    toolsLayout2->addWidget(numTileBar, 2, 1);
    toolsLayout2->addWidget(numTileSpinBox, 2, 2);
    toolsLayout2->addWidget(inSizeInfoText, 3, 0);
    toolsLayout2->addWidget(inSizeInfo, 3, 1, 1, 2, Qt::AlignCenter);
    toolsLayout2->addWidget(outSizeInfoText, 4, 0);
    toolsLayout2->addWidget(outSizeInfo, 4, 1, 1, 2, Qt::AlignCenter);
    toolsLayout2->addWidget(comInfoText, 5, 0);
    toolsLayout2->addWidget(comInfo, 5, 1, 1, 2, Qt::AlignCenter);

    // arrange button layout
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addLayout(buttonLayout1, 0, 0, Qt::AlignCenter);
    mainLayout->addLayout(toolsLayout1, 0, 1, Qt::AlignCenter);
    mainLayout->addLayout(toolsLayout2, 1, 0, 1, 2, Qt::AlignLeft);
    mainLayout->addWidget(resImage, 0, 2, 2, 2, Qt::AlignCenter);
    //mainLayout->setSpacing(10);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(mainLayout);
    setWindowTitle(tr("Image Quilting"));
}

// load texture image button
void IO::loadImageButton()
{
    QString filename = QFileDialog::getOpenFileName(this,
                       tr("Select Image"), "/home/kevin/research/texture/image_quilting/srcImage",
                       tr("Image Files (*.png *.jpg *.bmp)"));
    //std::cout << filename.toUtf8().constData();

    if(filename.isEmpty())
        return;
    else
    {
        QFile file(filename);

        if(!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::information(this, tr("Unable to open file!"), file.errorString());
            return;
        }

        // display srcImage in the label
        QPixmap image(filename);
        srcImage->clear();
        srcImage->setPixmap(image);
        srcImage->setScaledContents(true);
        srcImage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        QString input_width = QString::number(image.width());
        QString input_height = QString::number(image.height());
        QString size_info = QString::fromStdString("Size = ") + input_height + QString::fromStdString(" X ") + input_width;
        inSizeInfo->setText(size_info);
        inSizeInfo->setScaledContents(true);
        inSizeInfo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        int min_size = std::min(image.width(), image.height());
        tileSizeSpinBox->setRange(3,std::round(min_size*0.7));
        tileSizeBar->setRange(3,std::round(min_size*0.7));
        overlapRegionSpinBox->setRange(1,std::round((min_size*0.7)*0.5));
        overlapRegionBar->setRange(1,std::round((min_size*0.7)*0.5));

        // load image in self-defined "Image" format
        input_image = image.toImage();
    }
    synButton->setEnabled(true);
    resetButton->setEnabled(true);
}

void IO::synthesizeImageButton()
{
    QElapsedTimer timer;
    srand(time(NULL)); // must call srand() first
    std::cout << "synthesizing" << std::endl;

    // get the tilesize and overlap region value
    int tilesize = tileSizeSpinBox->value(); // get tile size
    int overlap = overlapRegionSpinBox->value(); // get overlap region
    int num_tiles = numTileSpinBox->value();
    bool useconv = convCheckBox->isChecked();
    bool complex = complexCheckBox->isChecked();
    bool debug = debugCheckBox->isChecked();

    // initialize variables in imagequilting
    imagequilting.initParams(useconv, complex, debug);

    // start synthesizing
    timer.start();
    imagequilting.synthesize(input_image, output_image, tilesize, num_tiles, overlap, useconv);

    // print the information
    QString str = QString::fromStdString("  ms");
    QString compute_info = QString::number(timer.elapsed()) + str;
    comInfo->setText(compute_info);
    comInfo->setScaledContents(true);
    comInfo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // display resImage in the label
    QPixmap image = QPixmap::fromImage(output_image);
    resImage->clear();
    resImage->setPixmap(image);
    resImage->setScaledContents(true);

    QString output_width = QString::number(image.width());
    QString output_height = QString::number(image.height());
    QString size_info = QString::fromStdString("Size = ") + output_height + QString::fromStdString(" X ") + output_width;
    outSizeInfo->setText(size_info);
    outSizeInfo->setScaledContents(true);
    outSizeInfo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    resImage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    saveButton->setEnabled(true);
}

void IO::saveImageButton()
{
    std::cout << "saving" << std::endl;

    QString filename = QFileDialog::getSaveFileName(this,
                       tr("Save result image"), "/home/kevin/research/texture/image_quilting/resImage",
                       tr("Image Files (*.png *.jpg *.bmp)"));

    if(filename.isEmpty())
        return;
    if(!filename.endsWith(".png"))
    {
        filename.append(".png");
    }
    output_image.save(filename, "PNG");
}

void IO::resetAllButton()
{
    std::cout << "reset all" << std::endl;

    srcImage->setText("Source Image");
    resImage->setText("Result Image");

    synButton->setEnabled(false);
    saveButton->setEnabled(false);
    resetButton->setEnabled(false);

    tileSizeSpinBox->setRange(1,100);
    tileSizeBar->setRange(1,100);
    overlapRegionSpinBox->setRange(1,25);
    overlapRegionBar->setRange(1,25);
    numTileSpinBox->setRange(3,15);
    numTileBar->setRange(3,15);

    tileSizeSpinBox->setValue(80);
    overlapRegionSpinBox->setValue(13);
    numTileSpinBox->setValue(5);

    inSizeInfo->setText("null");
    outSizeInfo->setText("null");
    comInfo->setText("null");
}
