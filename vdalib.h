#ifndef VDALIB_H
#define VDALIB_H

#include <QtCore>
#include <QString>
#include <QObject>
#include <QWidget>
#include <QDebug>
#include <QtNetwork>

#include "config.h"
#include "database.h"
#include "serverdata.h"
#include "vdalib_data.h"

#define LISTANUMSESSAO 100

namespace mmcore {

    class vdaLib : public QWidget
    {

        Q_OBJECT

    public:
        explicit vdaLib(QWidget *parent = 0);
        bool insertitem(QString mesa);
        bool insertitem_server(QString mesa, serverData *input, QTcpSocket *socket);
        QString buildJSon(QJsonObject input);
        int randInt(int low, int high);

        int ultimonumero = 0;
        unsigned int listanumeroSessao[LISTANUMSESSAO];

    signals:
        void response(bool result);
        void response_server(bool result, QTcpSocket *socket);

    public slots:
        void retlib(QString);
        void retlib_server(QString, QTcpSocket*);
        void errorlib(QString);
        void errorlib_server(QString, QTcpSocket*);

    private:
        int get_combinados();
        int get_countmember(QString input);
        void build_members(QString input, QString nivel, QString sep);
        QString build_detail(QString title, QString part1, QString part2, QString part3);
        void insertdet();
        QList<datamember*> lst_members;
        int pos_member   = 0;
        int pos_index    = 0;
        QString pos_code = "";
        QString pos_mesa = "";
    };
}

#endif // VDALIB_H
