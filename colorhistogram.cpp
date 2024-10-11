#include "colorhistogram.h"
#include "imageviewer.h"
#include <QtWidgets>

/* deriving from qwidget, setlayout, add stuff; setcentralwidget only happens in qmainwindow */

ColorHistogram::ColorHistogram(const QImage &_image):image(_image) { //initializing image member to value of image passed to constructor as reference

    /* UI Stuff */
    mainLayout = new QHBoxLayout(this);

    /* add imageViewer to mainLayout - make it stretch to fit space */
    imageViewer = new ImageViewer(image);
    mainLayout->addWidget(imageViewer, 1); //stretch of 1 to make imageViewer stretch
    //connect(imageViewer, &ImageViewer::mouseMovedSignal, this, &ColorHistogram::mouseMovedSlot); //TODO: when hovering, give RGB value


    /* construct side layout and add to mainLayout */
    sideLayout = new QVBoxLayout();
    mainLayout->addLayout(sideLayout);

    // colorHistogram = new QPixmap(256, 256); /* assign colorHistogram = the one we want for slider, combo-box combination */
    colorHistogramDisplay = new QLabel();

    /* Color Slider Value Display */
    sliderValDisplay = new QLabel();

    /* Color Slider for r/g/b */
    colorSlider = new QSlider(Qt::Orientation::Horizontal);
    colorSlider->setRange(0, 255);

    /* Combo Box for color options */
    QStringList colorOptions = {"Red", "Green", "Blue"};
    colorSelector = new QComboBox();
    colorSelector->addItems(colorOptions);

    /* Combo Box for freq threshold */
    QStringList thresholdOptions = {"1", "2", "4", "8", "16", "32", "64", "128"};
    freqThresholdSelector = new QComboBox();
    freqThresholdSelector->addItems(thresholdOptions);

    /* Add UI Stuff to Layout - repetition of -> (so this could, in future be refactored to its own class) */
    sideLayout->addWidget(colorHistogramDisplay);
    sideLayout->addWidget(sliderValDisplay);
    sideLayout->addWidget(colorSlider);
    sideLayout->addWidget(colorSelector);
    sideLayout->addWidget(freqThresholdSelector);

    /* connections for slider, combo boxes */
    connect(colorSlider, &QSlider::valueChanged, this, &ColorHistogram::sliderValueChanged);
    connect(colorSelector, &QComboBox::activated, this, &ColorHistogram::colorComboBoxToggled);
    connect(freqThresholdSelector, &QComboBox::activated, this, &ColorHistogram::thresholdComboBoxToggled);

    /* Logic stuff */
    freq = QVector<int>(1 << 24, 0);
    buildFreq(image);

    /* build pixmaps for red since this is our default color, using default threshold of 1 */
    freqThreshold = 1;
    sliderVal = 1;
    buildHistSlices(0);

    /* display pixmap for slice of red w/ (0, pixmap, pixmap) , since this is our default value of our default color */
    colorHistogramDisplay->setPixmap(histSlices[0].at(0));
}

void ColorHistogram::buildFreq(const QImage img) {

    //loop thru image to build freq vector
    for (int y = 0; y < img.height(); ++y) {
        const QRgb *line = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            /* RGB is stored as 4 bytes: alpha, red, green, blue; bit-mask alpha to extract RGB; Online Help: https://stackoverflow.com/questions/6126439/what-does-0xff-do */
            const QRgb &rgb = line[x];
            ++freq[rgb & 0xffffff]; //bit-wise AND to remove alpha bits; similar stuff when determining red, green, blue etc.
            }
    }
}

void ColorHistogram::sliderValueChanged(int value) {
    /* cosmetics */
    sliderValDisplay->setText(QString::number(value));

    /* set sliderVal member to slider's value, access corresponding pixmap */
    sliderVal = value;
    //acccess value for Qmap[color_key];
    colorHistogramDisplay->setPixmap(histSlices[selectedColor].at(value));

}

void ColorHistogram::colorComboBoxToggled(int index) {
    /* set key for QMap, so we look for slices corresponding to selected color */
    selectedColor = index;
    int currentSliderValue = colorSlider->value();

    /* reubuild pixmaps for new color */
    buildHistSlices(selectedColor);

    /* display updated pixmap for new color */
    colorHistogramDisplay->setPixmap(histSlices[index].at(currentSliderValue));
}

void ColorHistogram::thresholdComboBoxToggled(int index) {
    freqThreshold = freqThresholdSelector->currentText().toInt();
    int currentColor = colorSelector->currentIndex();
    int currentSliderValue = colorSlider->value();

    /* rebuild pixmaps for new threshold */
    histSlices.clear(); //so that our function re-builds pixmaps for all colors
    buildHistSlices(currentColor);

    /* display updated pixmap for new threshold */
    colorHistogramDisplay->setPixmap(histSlices[currentColor].at(currentSliderValue));
}


void ColorHistogram::buildHistSlices(int _selectedColor) {
    /* if we already constructed the 256 Pixmaps for the selected color, no need to build again */
    if (!histSlices[_selectedColor].isEmpty()) {
        return;
    }

    /* Loop thru 256 slices for given color slice */
    for (int slice = 0; slice < 256; ++slice) {

        /* Need a temp image to modify specific pixels */
        QPixmap sliceView(256, 256);
        QImage temp(sliceView.toImage());


        int freqIndex = 0;
        QRgb freqColor(0);

        /* 2-D loop thru image, for each pixel check: if its freq exceeds threshold -> then, color it appropriately */
        for(int x = 0; x < temp.width(); ++x)
        {
            for(int y = 0; y < temp.height(); ++y)
            {
                switch(_selectedColor) {
                    case 0:
                        freqIndex = qRgb(slice, x, y) & 0xffffff;
                        freqColor = qRgb(slice, x, y);
                        break;
                    case 1:
                        freqIndex = qRgb(x, slice, y) & 0xffffff;
                        freqColor = qRgb(x, slice, y);
                        break;
                    case 2:
                        freqIndex = qRgb(x, y, slice) & 0xffffff;
                        freqColor = qRgb(x, y, slice);
                        break;
                }


                /* If color is present, show it on pixmap; ohterwise show black */
                if (freq[freqIndex] >= freqThreshold) {
                    temp.setPixelColor(x, y, freqColor);
                } else {
                    temp.setPixelColor(x, y, QColorConstants::Black);
                }
            }
        }

        /* convert image back to pixmap, append that pixmap to corresponding vector of pixmaps */
        sliceView = QPixmap::fromImage(temp);
        histSlices[_selectedColor].append(sliceView);
    }
}
