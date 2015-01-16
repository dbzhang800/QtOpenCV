#include "cvmatandqimage.h"
#include "cvimagewidget.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QApplication>
#include <QFileDialog>
#include <QTabWidget>
#include <QDebug>

//We don't defined any custom classes in this example.

QtOcv::ImageWidget *createImageWindow(const QImage &img, const QString &title=QString());

void process1()
{
    //Get an image file path.
    QString dir = "";
    QString filePath = QFileDialog::getOpenFileName(0, "Open", dir, "Images(*.png *.bmp *.jpg *.gif)");
    if (filePath.isEmpty())
        return;

    //Show the original image in a window.
    QImage sourceImage = QImage(filePath);
    createImageWindow(sourceImage, filePath);

    //Get CV_8UC3(RGB) or CV_8UC1 mats
    //no matter which format the QImage is actully used.
    cv::Mat mat_8UC3_rgb = QtOcv::image2Mat(sourceImage, CV_8UC3, QtOcv::MCO_RGB);
#if 0
    cv::Mat mat_8UC1_gray = QtOcv::image2Mat(sourceImage, CV_8UC1);
#else
    cv::Mat mat_8UC1_gray(mat_8UC3_rgb.rows, mat_8UC3_rgb.cols, CV_8UC1);
    int from_to[] = {1, 0};
    cv::mixChannels(&mat_8UC3_rgb, 1, &mat_8UC1_gray, 1, from_to, 1);
#endif
    //Show the gray image in a new window if needed.
    createImageWindow(QtOcv::mat2Image_shared(mat_8UC1_gray), "Gray Image");

    //Create mono chrome mat.
    cv::Mat mat_mono;

#if 1
#  if 1
    double minVal, maxVal;
    cv::minMaxIdx(mat_8UC1_gray, &maxVal, &minVal);
    double v = (maxVal - minVal) * 0.3 + minVal;
    //v = 255 * 0.4;
    cv::threshold(mat_8UC1_gray, mat_mono, v, 255, cv::THRESH_BINARY);
#  else
    cv::adaptiveThreshold(mat_8UC1_gray, mat_mono, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 3, -1);
#  endif
    //Show the mono image in a new window if needed.
    createImageWindow(QtOcv::mat2Image_shared(mat_mono), "mono Image");
#else
    mat_mono = mat_8UC1_gray;
#endif

//    cv::floodFill()

    // Apply the dilation operation
    cv::Mat mat_dilation;
#if 0
    int dilation_size = 3;
    cv::Mat element = cv::getStructuringElement(2, cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
                  cv::Point(dilation_size, dilation_size));     // dilation_type = MORPH_ELLIPSE
    cv::dilate(mat_mono, mat_dilation, element);
    //Show the mono image in a new window if needed.
    createImageWindow(QtOcv::mat2Image_shared(mat_dilation), "Dilation Image");
#else
    mat_dilation = mat_mono;
#endif

    //Try detected edges of the mono image.
    int kernel_size = 3;
    cv::Mat detected_edges;
    // Reduce noise with a kernel 3x3
    cv::blur(mat_dilation, detected_edges, cv::Size(5,5));
    // Canny detector
    cv::Canny(detected_edges, detected_edges
              , 20
              , 60
              , kernel_size);
    createImageWindow(QtOcv::mat2Image_shared(detected_edges), "Edge Image");

    // Apply the dilation operation again?
    cv::Mat mat_dilation2;
    int dilation_size2 = 3;
    cv::Mat elements = cv::getStructuringElement(2, cv::Size(2 * dilation_size2 + 1, 2 * dilation_size2 + 1),
                  cv::Point(dilation_size2, dilation_size2));     // dilation_type = MORPH_ELLIPSE
    cv::dilate(detected_edges, mat_dilation2, elements);
    //Show the mono image in a new window if needed.
    createImageWindow(QtOcv::mat2Image_shared(mat_dilation2), "Dilation Image - 2");

    //Find countours
    std::vector<std::vector<cv::Point> > contours;
    int find_contours_mode = CV_CHAIN_APPROX_TC89_L1; //CV_RETR_LIST
    cv::findContours(mat_dilation2, contours, find_contours_mode, CV_CHAIN_APPROX_NONE);
    std::sort(contours.begin(), contours.end(), [](std::vector<cv::Point> a, std::vector<cv::Point> b) {
        return b.size() < a.size();
    });

    size_t maxEllipseCount = 4;
    cv::Mat ellipseImage = mat_8UC3_rgb.clone();
    for(size_t i = 0, foundCount = 0; (i < contours.size()) && (foundCount < maxEllipseCount); i++) {
        size_t count = contours[i].size();

        if( count < 6 )
            continue;

        cv::Mat pointsf;
        cv::Mat(contours[i]).convertTo(pointsf, CV_32F);
        cv::RotatedRect box = cv::fitEllipse(pointsf);

        if( qMax(box.size.width, box.size.height) > qMin(box.size.width, box.size.height)*30 )
            continue;

        foundCount++;

        cv::ellipse(ellipseImage, box, cv::Scalar(0,0,255), 1, CV_AA);
        cv::ellipse(ellipseImage, box.center, box.size*0.5f, box.angle, 0, 360, cv::Scalar(0,255,255), 1, CV_AA);

        if (foundCount == 2) {
            //high light the second one.
            cv::Point2f vtx[4];
            box.points(vtx);
            for( int j = 0; j < 4; j++ )
                cv::line(ellipseImage, vtx[j], vtx[(j+1)%4], cv::Scalar(0,255,0), 1, CV_AA);
        }

        QString ellipseInfo = QString("Center(%1, %2) Size(%3, %4) Angle %5")
                .arg(box.center.x).arg(box.center.y)
                .arg(box.size.width).arg(box.size.height)
                .arg(box.angle);
        qDebug() << foundCount << ellipseInfo;
    }

    //Show the ellipse window
    createImageWindow(QtOcv::mat2Image_shared(ellipseImage), "Ellipse Image");
}

static QTabWidget *topWidget = 0;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTabWidget mw;
    mw.setWindowTitle("MainWindow of the QtOpenCV demo.");
    topWidget = &mw;

    //vvvvvvvvvvvvvv
    process1();
    //^^^^^^^^^^^^^^

    if (mw.count())
        mw.showMaximized();
    return a.exec();
}

QtOcv::ImageWidget *createImageWindow(const QImage &img, const QString &title)
{
    QtOcv::ImageWidget *w = new QtOcv::ImageWidget;
    w->setImage(img);
    w->setCurrentScale(0);//auto fit

    //Add to top tabWidget or shown as an top window.
#if 1
    topWidget->addTab(w, title.isEmpty() ? QString("Tab%1").arg(topWidget->count()) : title);
#else
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->resize(800, 600);
    if (!title.isEmpty())
        w->setWindowTitle(title);
    w->show();
#endif

    return w;
}


