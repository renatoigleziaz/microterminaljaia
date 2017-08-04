#include "server.h"

/*
 *  (H) Gerenciador de Micro-Terminal Jaia
 *
 *  Esffera Software
 *  por Renato Igleziaz
 *  08/04/2016
 *
 */

namespace mmcore {

    Server::Server(QObject *parent) : QObject(parent)
    {
        // controla o endereço de acesso do servidor

        // banco de dados
        data_valid = new mmcore::database(NULL);
        // sinais de resposta
        connect(data_valid, SIGNAL(ret_server(QString,QTcpSocket*)), this, SLOT(ret_server(QString,QTcpSocket*)));
        connect(data_valid, SIGNAL(error_server(QString,QTcpSocket*)), this, SLOT(error_server(QString,QTcpSocket*)));

        // inicia as funções principais para adicionar um item ao MM
        this->vda_ins = new mmcore::vdaLib(NULL);
        connect(this->vda_ins, SIGNAL(response_server(bool,QTcpSocket*)), this, SLOT(response_server(bool,QTcpSocket*)));

        // controle de porta
        QSettings settings("esffera", "mmFastFood");
        settings.beginGroup("connect");
        QString ret = settings.value("port").toString();
        settings.endGroup();

        if (ret.length()==0) {
            ret = "1024";
        }

        QTextStream ts(&ret);
        quint16 port = 1024;
        ts >> port;

        // instancia do servidor
        server = new QTcpServer(this);
        connect(server, SIGNAL(newConnection()), SLOT(newConnection()));

        // inicia o método de escuta
        bool result = server->listen( QHostAddress::Any, port );

        if (!result) {
            // servidor não está pronto

            QMessageBox::critical(NULL,
                                  mmcore::config::nameapp(),
                                  "Servidor não está pronto, verifique seu Anti-Virus, Firewall e configuração de portas e tente novamente.");

            qDebug() << "mmServerTerminal: Erro não pode iniciar o serviço";
        }

        // sinal de conexão
        qDebug() << "mmServerTerminal: Conectado? = " << result;
    }

    void Server::newConnection()
    {
        // Novos terminais

        bool check = false;
        serverData *newdata;

        while (server->hasPendingConnections())
        {
            check = false;
            QTcpSocket *socket = server->nextPendingConnection();
            connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
            connect(socket, SIGNAL(disconnected()), SLOT(disconnected()));

            qDebug() << "mmServerTerminal: Login de " << socket->peerAddress().toString();

            // verifica se o IP já existe
            for (int x = 0; x < clients.count(); x++) {

                if (clients[x]->ip == socket->peerAddress().toString()) {
                    check = true;
                    break;
                }
            }

            if (!check) {
                // adiciona se não tiver
                newdata = new serverData();
                newdata->ip = socket->peerAddress().toString();
                clients.append(newdata);

                // emite o aviso de novo cliente para UI
                emit this->clientsupdate(clients);
            }

            // primeira checagem
            this->terminal_op(socket, 0);
        }
    }

    void Server::disconnected()
    {
        // quando um terminal desconecta do servidor

        qDebug() << "Terminal Encerrado.";

        QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
        socket->deleteLater();
    }

    void Server::readyRead()
    {
        // controla o retorno de informação do servidor

        QTcpSocket *socket = static_cast<QTcpSocket*>(sender());

        if (socket->bytesAvailable() > 0) {

            QByteArray buffer = socket->readAll();
            QString s_data = QString::fromUtf8(buffer.data());

            qDebug() << socket->peerAddress().toString();
            qDebug() << s_data;

            // acha o terminal
            int terminal_int = -1;

            for (int x = 0; x < clients.count(); x++) {

                if (clients[x]->ip == socket->peerAddress().toString()) {
                    terminal_int = x;
                    break;
                }
            }

            if (terminal_int == -1) {
                return;
            }

            if (clients[terminal_int]->op == 3) {
                // verifica se o terminal está em modo Busy
                return;
            }
            else if (clients[terminal_int]->op == 6) {
                // em modo de mensagem, qualquer tecla vai mandar para o inicio
                this->terminal_op(socket, 0);
                return;
            }

            if (buffer == "\r") {
                // enter

                if (clients[terminal_int]->op == 0) {
                    // mesa

                    if (clients[terminal_int]->mesa.length() == 0) {
                        this->terminal_op(socket, 0);
                        return;
                    }

                    // se tudo ocorreu bem pede o código do produto
                    this->terminal_op(socket, 1);
                }
                else if (clients[terminal_int]->op == 1) {
                    // produtos

                    if (clients[terminal_int]->prodc.length() == 0) {
                        this->terminal_op(socket, 1);
                        return;
                    }

                    // pesquisa o produto e avança para a quantidade
                    this->terminal_cls(socket);
                    this->terminal_settext(socket, "Trabalhando...");
                    data_valid->open_server("command=search-code&param=" + clients[terminal_int]->prodc, socket);
                }
                else if (clients[terminal_int]->op == 2) {
                    // quantidade

                    if (clients[terminal_int]->qtd.length() == 0) {
                        this->terminal_op(socket, 2);
                        return;
                    }

                    bool validate;
                    QString validStr = clients[terminal_int]->qtd;
                    validStr.replace(",", ".").toFloat(&validate);

                    if (!validate)
                    {
                        this->terminal_op(socket, 0);
                        return;
                    }

                    // coloca o terminal em modo de espera até adicionar o produto
                    this->terminal_op(socket, 3);
                }
            }
            else if (buffer == "\b") {
                // backspace

                if (clients[terminal_int]->op == 0) {
                    // mesa

                    if (clients[terminal_int]->mesa.length() == 0) {
                        return;
                    }

                    clients[terminal_int]->mesa = clients[terminal_int]->mesa.mid(0, clients[terminal_int]->mesa.length()-1);
                }
                else if (clients[terminal_int]->op == 1) {
                    // produto

                    if (clients[terminal_int]->prodc.length() == 0) {
                        return;
                    }

                    clients[terminal_int]->prodc = clients[terminal_int]->prodc.mid(0, clients[terminal_int]->prodc.length()-1);
                }
                else if (clients[terminal_int]->op == 2) {
                    // quantidade

                    if (clients[terminal_int]->qtd.length() == 0) {
                        return;
                    }

                    clients[terminal_int]->qtd = clients[terminal_int]->qtd.mid(0, clients[terminal_int]->qtd.length()-1);
                }

                socket->write(buffer);
            }
            else if (buffer == "\u007F") {
                // delete

                // reseta toda a operação
                this->terminal_op(socket, 0);
            }
            else if (buffer == "." || buffer == ",") {
                // "." ou "," usado para quantidade somente

                buffer = ",";

                if (clients[terminal_int]->op == 2) {

                    if (clients[terminal_int]->qtd.length() == 0) {
                        return;
                    }

                    if (clients[terminal_int]->qtd.indexOf(",") > 0) {
                        return;
                    }

                    clients[terminal_int]->qtd += buffer;
                }
                else {
                    return;
                }

                socket->write(buffer);
            }
            else {
                // qualquer numero

                if (clients[terminal_int]->op == 0) {
                    // mesa

                    if (clients[terminal_int]->mesa.length() == 3) {
                        return;
                    }

                    clients[terminal_int]->mesa += buffer;
                }
                else if (clients[terminal_int]->op == 1) {
                    // produto

                    if (clients[terminal_int]->prodc.length() == 20) {
                        return;
                    }

                    clients[terminal_int]->prodc += buffer;
                }
                else if (clients[terminal_int]->op == 2) {
                    // quantidade

                    if (clients[terminal_int]->qtd.length() == 10) {
                        return;
                    }

                    clients[terminal_int]->qtd += buffer;
                }

                socket->write(buffer);
            }
        }
    }

    void Server::terminal_cls(QTcpSocket *terminal) {
        // limpa a tela do terminal

        terminal->write("\x1B[H\x1B[J");
    }

    void Server::terminal_settext(QTcpSocket *terminal, QString text) {
        // envia um texto para o terminal

        QByteArray ba = text.toLatin1();
        const char *c_str2 = ba.data();
        terminal->write(c_str2);
    }

    void Server::terminal_setcursor(QTcpSocket *terminal) {
        // fixa o cursor para digitação

        terminal->write("\x1B[02;01H");
    }

    void Server::terminal_op(QTcpSocket *terminal, int op) {
        // controle de sessão dos terminais

        this->terminal_cls(terminal);

        if (op == 0) {
            // modo de mesa/comanda

            this->terminal_settext(terminal, "Mesa/Comanda:");

            for (int x = 0; x < clients.count(); x++) {

                if (clients[x]->ip == terminal->peerAddress().toString()) {
                    clients[x]->op    = 0;
                    clients[x]->mesa  = "";
                    clients[x]->prodc = "";
                    clients[x]->prodn = "";
                    clients[x]->qtd   = "";
                    this->terminal_setcursor(terminal);
                    break;
                }
            }
        }
        else if (op == 1) {
            // modo de produto

            this->terminal_settext(terminal, "Codigo do Produto:");

            for (int x = 0; x < clients.count(); x++) {

                if (clients[x]->ip == terminal->peerAddress().toString()) {
                    clients[x]->op = 1;
                    this->terminal_setcursor(terminal);
                    break;
                }
            }
        }
        else if (op == 2) {
            // modo de quantidade

            this->terminal_settext(terminal, "Quantidade:");

            for (int x = 0; x < clients.count(); x++) {

                if (clients[x]->ip == terminal->peerAddress().toString()) {
                    clients[x]->op = 2;
                    this->terminal_setcursor(terminal);
                    this->terminal_settext(terminal, clients[x]->qtd);
                    break;
                }
            }
        }
        else if (op == 3) {
            // lança o item

            this->terminal_settext(terminal, "UM MOMENTO...");

            for (int x = 0; x < clients.count(); x++) {

                if (clients[x]->ip == terminal->peerAddress().toString()) {
                    clients[x]->op = 3;
                    data_valid->open("command=valid-code&param=" + clients[x]->prodc);
                    break;
                }
            }
        }

    }

    void Server::ret_server(QString buffer, QTcpSocket *socket) {
        // retorno de pesquisa a API

        qDebug() << buffer;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(buffer.toUtf8());
        QJsonObject jsonObject = jsonResponse.object();
        QJsonArray resp = jsonResponse.array();

        int x = 0;

        for (x = 0; x < clients.count(); x++) {

            if (clients[x]->ip == socket->peerAddress().toString()) {
                // achou o terminal

                if (clients[x]->op == 1) {
                    // retorno da escolha do produto

                    if (resp[0].toString() == "erro") {

                        // verifica se deu erro
                        // erro
                        clients[x]->prodc = "";
                        this->terminal_msg(socket, "Nao Encontrado!");
                        return;
                    }
                    else if (resp[0].toString() == "alerta") {

                        // nenhum registro encontrado
                        // erro
                        clients[x]->prodc = "";
                        this->terminal_msg(socket, "Nao Encontrado!");
                        return;
                    }

                    clients[x]->prodc = resp[0].toString();
                    clients[x]->prodn = resp[1].toString();
                    clients[x]->coz   = resp[2].toString();
                    clients[x]->img   = resp[3].toString();
                    clients[x]->cat   = resp[4].toString();
                    clients[x]->val   = resp[5].toString();
                    clients[x]->qtd   = resp[6].toString();

                    if (clients[x]->qtd == "0") {
                        // não é item de balança
                        clients[x]->qtd = "1";
                    }

                    // modo quantidade
                    this->terminal_op(socket, 2);
                    break;
                }
                else if (clients[x]->op == 3) {
                    // lança o produto

                    if (resp[0].toString() == "erro") {

                        // verifica se deu erro
                        // erro
                        clients[x]->prodc = "";
                        this->terminal_msg(socket, "Nao Encontrado!");
                        return;
                    }
                    else if (resp[0].toString() == "alerta") {

                        // nenhum registro encontrado
                        // erro
                        clients[x]->prodc = "";
                        this->terminal_msg(socket, "Nao Encontrado!");
                        return;
                    }

                    QJsonObject obj = resp[0].toObject();

                    clients[x]->ean = obj["ean"].toString();
                    clients[x]->uni = obj["uni"].toString();
                    clients[x]->ipi = obj["ipi"].toString();
                    clients[x]->icm = obj["icm"].toString();
                    clients[x]->tb1 = obj["tb1"].toString();
                    clients[x]->tb2 = obj["tb2"].toString();
                    clients[x]->tb3 = obj["tb3"].toString();
                    clients[x]->tb4 = obj["tb4"].toString();
                    clients[x]->bru = obj["bru"].toString();
                    clients[x]->liq = obj["liq"].toString();
                    clients[x]->ncm = obj["ncm"].toString();
                    clients[x]->rod = obj["rodizio"].toString();
                    clients[x]->hor = obj["hora"].toString();

                    // adiciona diretamente
                    if (!this->vda_ins->insertitem_server(clients[x]->mesa, clients[x], socket)) {

                        // erro
                        this->terminal_msg(socket, "Erro!");
                    }

                    break;
                }
            }
        }
    }

    void Server::error_server(QString buffer, QTcpSocket *socket) {
        // retorno de pesquisa a API em caso de erro

        qDebug() << buffer;
        this->terminal_msg(socket, "Erro!");
    }

    void Server::response_server(bool result, QTcpSocket *socket) {
        // resposta da tentativa de adicionar um produto ao MM

        qDebug() << result;

        if (result) {
            this->terminal_msg(socket, "Adicionado!");

            for (int x = 0; x < clients.count(); x++) {

                if (clients[x]->ip == socket->peerAddress().toString()) {
                    // emite o sinal
                    emit this->clientfinished(clients[x]);
                    break;
                }
            }
        }
        else {
            this->terminal_msg(socket, "Erro!");
        }
    }

    void Server::terminal_msg(QTcpSocket *terminal, QString msg) {
        // envia uma mensagem para o terminal

        this->terminal_cls(terminal);
        this->terminal_settext(terminal, msg);

        for (int x = 0; x < clients.count(); x++) {

            if (clients[x]->ip == terminal->peerAddress().toString()) {
                clients[x]->op = 6;
                break;
            }
        }

        // serviços de mensagem para o terminal
        qDebug() << "Thread start...";
    }

    void Server::alert(QTcpSocket *terminal) {
        // ok fim da mensagem

        //delete sleeping;
        qDebug() << "Reset Terminal";
        this->terminal_op(terminal, 0);
    }
}
