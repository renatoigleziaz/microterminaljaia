#include "util.h"

namespace mmcore {

    void Util::centerform(QWidget *parent) {
        // centraliza uma form

        // geometria da área do dispositivo
        QRect screenGeometry = QApplication::desktop()->screenGeometry();
        int x = (screenGeometry.width()-parent->width()) / 2;
        int y = (screenGeometry.height()-parent->height()) / 2;

        // exibe a form
        parent->show();

        // centraliza
        parent->move(x, y);
    }

    void Util::effectblur(QWidget *parent, bool blurred) {
        // aplica blur na janela do Android e IOS enquanto carrega um DialogBox

        // cria objeto de efeito
        QGraphicsBlurEffect *effect = new QGraphicsBlurEffect(parent);

        // aplica o efeito para dispositivos mobile
    #ifdef Q_OS_ANDROID
        parent->setGraphicsEffect(blurred? effect : 0);
    #endif

    #ifdef Q_OS_IOS
        parent->setGraphicsEffect(blurred? effect : 0);
    #endif

        delete effect;
    }

    void Util::effectfadein(QWidget *object) {

        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(object);
        object->setGraphicsEffect(eff);
        QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
        a->setDuration(350);
        a->setStartValue(0);
        a->setEndValue(1);
        a->setEasingCurve(QEasingCurve::InBack);
        a->start(QPropertyAnimation::DeleteWhenStopped);
    }

    QPropertyAnimation* Util::effectfadeout(QWidget *object) {

        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(object);
        object->setGraphicsEffect(eff);
        QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
        a->setDuration(350);
        a->setStartValue(1);
        a->setEndValue(0);
        a->setEasingCurve(QEasingCurve::OutBack);
        a->start(QPropertyAnimation::DeleteWhenStopped);

        // connect(a,SIGNAL(finished()),this,SLOT(hideThisWidget()));
        return a;
    }

    void Util::effectclicksound() {
        // faz o android emitir uma notificação padrão
#ifdef Q_OS_ANDROID

        QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative", "activity", "()Landroid/app/Activity;");
        if ( activity.isValid() )
        {
            jint notifcationType = QAndroidJniObject::getStaticField<jint>("android/media/RingtoneManager", "TYPE_NOTIFICATION");
            QAndroidJniObject notification = QAndroidJniObject::callStaticObjectMethod("android/media/RingtoneManager", "getDefaultUri", "(I)Landroid/net/Uri;", notifcationType);
            if ( notification.isValid() )
            {
                QAndroidJniObject ring = QAndroidJniObject::callStaticObjectMethod("android/media/RingtoneManager",
                                                                               "getRingtone",
                                                                               "(Landroid/content/Context;Landroid/net/Uri;)Landroid/media/Ringtone;",
                                                                               activity.object<jobject>(),
                                                                               notification.object<jobject>());
                if ( ring.isValid() )
                {
                    ring.callMethod<void>("play", "()V");
                }
            }
        }
#endif
    }

    void Util::fixOrientation() {
        // fixa a orientação para dispositivos móveis

#ifdef Q_OS_ANDROID

        QScreen* screen = QGuiApplication::primaryScreen();
        screen->setOrientationUpdateMask(Qt::PortraitOrientation);
#endif

    }

    void Util::loader(bool state, QLabel *control)
    {
        // controle de carregamento
        // ANULADA -> Correções de Performance

        if (state) {

            control->setVisible(false);
        }
    }

    void Util::loaderw(bool state, QLabel *control)
    {
        // controle de carregamento

        if (!state) {

            QMovie* movie = new QMovie(":/default.gif");

            if (!movie->isValid())
            {
                qDebug() << "ERRO: Animação não foi carregada...";
                return;
            }

            control->setMovie(movie);
            movie->start();
            control->setVisible(true);
        }
        else {

            control->setVisible(false);
        }
    }

    void Util::zebra(QStandardItemModel* model, QTableView* control) {

        // faz o zebrado no QTableView

        QModelIndex index;
        bool bingo = false;

        for (int x = 0; x < control->model()->rowCount(); x++) {

            if (bingo) {

                for (int i = 0; i < control->model()->columnCount(); i++) {

                    index = model->index(x, i);
                    model->setData(index, QColor(200,200,200), Qt::BackgroundRole);
                }
            }

            if (!bingo) {

                bingo = true;
            }
            else {
                bingo = false;
            }
        }
    }

    void Util::effectvibrate() {
        // faz o android vibrar rs
#ifdef Q_OS_ANDROID

        QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative", "activity", "()Landroid/app/Activity;");
        //QAndroidJniObject activity = QtAndroid::androidActivity(); // Req. Qt 5.3
        if ( activity.isValid() )
        {
            qDebug() << "Vibrate-Start()";

            QAndroidJniObject serviceField = QAndroidJniObject::getStaticObjectField<jstring>("android/content/Context", "VIBRATOR_SERVICE");
            if ( serviceField.isValid() )
            {
                qDebug() << "Vibrate-Service()";
                QAndroidJniObject vibrator = activity.callObjectMethod("getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;",serviceField.object<jobject>());

                if ( vibrator.isValid() )
                {
                    qDebug() << vibrator.isValid();
                    vibrator.callMethod<void>("vibrate", "(J)V", 1, 100);
                }
            }
        }

#endif
    }
}
