#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtCore>
#include <QtNetwork>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QChar>
#include <QList>
#include <QTimer>

#include "config.h"
#include "serverdata.h"
#include "database.h"
#include "vdalib.h"
#include "mythread.h"

/*
 *  (H) Gerenciador de Micro-Terminal Jaia
 *
 *  Esffera Software
 *  por Renato Igleziaz
 *  08/04/2016
 *
 */

namespace mmcore {

    class Server : public QObject
    {
        Q_OBJECT
    public:
        explicit Server(QObject *parent = 0);

    signals:
        // quando um client é conectado no server, é enviado um sinal para GUI
        void clientsupdate(QList<serverData*>);
        // quando um cliente é desconectado a GUI é sinalizada
        void clientfinished(serverData *item);

    public slots:
        // quando recebe um novo cliente
        void newConnection();
        // quando um cliente é desconectado
        void disconnected();
        // quando uma mensagem do terminal é recebida
        void readyRead();
        // quando uma pesquisa AJAX é feita
        void ret_server(QString buffer, QTcpSocket *socket);
        void error_server(QString buffer, QTcpSocket *socket);
        void response_server(bool result, QTcpSocket *socket);
        void alert(QTcpSocket *terminal);

    private:
        // objeto principal socket
        QTcpServer *server;
        // usado como memoria virtual amarrada ao IP do terminal
        QList<serverData*> clients;
        // objeto AJAX de conexão
        mmcore::database *data_valid;
        // classe de inserção de itens
        mmcore::vdaLib *vda_ins;

        // limpa a tela do terminal
        void terminal_cls(QTcpSocket *terminal);
        // envia um texto ao terminal
        void terminal_settext(QTcpSocket *terminal, QString text);
        // envia o cursor para uma posição especifica
        void terminal_setcursor(QTcpSocket *terminal);
        // controla os eventos como (Comanda, Codigo do Produto e Quantidade)
        void terminal_op(QTcpSocket *terminal, int op);
        // envia uma mensagem ao terminal e aguarda qualquer tecla para continuar
        void terminal_msg(QTcpSocket *terminal, QString msg);
    };

}

#endif // SERVER_H
