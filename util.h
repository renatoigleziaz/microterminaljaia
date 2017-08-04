#ifndef UTIL_H
#define UTIL_H

#include <QtCore>
#include <QWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QtGui>
#include <QGraphicsBlurEffect>
#include <QLabel>
#include <QTableView>

#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#include <QtAndroid>
#endif

namespace mmcore {

    class Util
    {
    public:
        static void centerform(QWidget *parent);
        static void effectblur(QWidget *parent, bool blurred);
        static void effectfadein(QWidget *object);
        static QPropertyAnimation* effectfadeout(QWidget *object);
        static void effectvibrate();
        static void effectclicksound();
        static void fixOrientation();
        static void loader(bool state, QLabel* control);
        static void loaderw(bool state, QLabel* control);
        static void zebra(QStandardItemModel* model, QTableView* control);
    };

}

#endif // UTIL_H
