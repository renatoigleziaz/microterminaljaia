#include "vdalib.h"

namespace mmcore {

    vdaLib::vdaLib(QWidget *parent) : QWidget(parent)
    {
        // inicialize
        lst_members.clear();
    }

    bool vdaLib::insertitem_server(QString mesa, serverData *input, QTcpSocket *socket) {
        // lança um produto na base de dados do MM
        // pelo sistema de Micro-Terminal

        bool retsuccess = true;

        if (input->prodc.length() == 0) {
            // nenhum produto carregado
            qDebug() << "Nenhum produto está salvo na memoria local para ser inserido";
            return false;
        }

        // calcula o valor do produto
        QString qtd = input->qtd;
        QString tb1 = input->tb1;
        QString tb2 = input->tb2;
        QString tb3 = input->tb3;
        QString tb4 = input->tb4;

        float calc_qtd   = qtd.replace(",",".").toFloat();
        float calc_cus   = 0;
        float calc_total = 0;
        int   tabela     = 1;     /* ainda fixar isso aqui rs */

        if (tabela == 1) {
            calc_cus = tb1.replace(",",".").toFloat();
        }
        else if (tabela == 2) {
            calc_cus = tb2.replace(",",".").toFloat();
        }
        else if (tabela == 3) {
            calc_cus = tb3.replace(",",".").toFloat();
        }
        else if (tabela == 4) {
            calc_cus = tb4.replace(",",".").toFloat();
        }

        calc_total    = (calc_qtd * calc_cus);

        // constroi o objeto para transmitir os dados
        QString cus = QString::number( calc_cus, 'f', 2 );
        cus.replace(".", ",");
        QString custot = QString::number( calc_total, 'f', 2 );
        custot.replace(".", ",");

        QJsonObject object {

            {"index", this->randInt(1, 1000)},
            {"cod", input->prodc},
            {"ean", input->ean},
            {"des", input->prodn},
            {"uni", input->uni},
            {"qtd", input->qtd},
            {"cus", cus},
            {"custot", custot},
            {"ipi", input->ipi},
            {"icm", input->icm},
            {"tb1", input->tb1},
            {"tb2", input->tb2},
            {"tb3", input->tb3},
            {"tb4", input->tb4},
            {"bru", input->bru},
            {"liq", input->liq},
            {"ncm", input->ncm},
            {"tiplan", "0"},
            {"altval", "0"},
            {"cliente", "Outros"},
            {"detalhes", ""},
            {"usuario", mmcore::config::getusername()},
            {"rodizio", "0"},                                   /* ainda fixar isso aqui rs */
            {"hora", input->hor}
        };

        // gera o Json
        QString arrayitens = this->buildJSon(object);

        // envia para a API
        mmcore::database *data = new mmcore::database(NULL);
        // sinais de resposta
        connect(data, SIGNAL(ret_server(QString,QTcpSocket*)), this, SLOT(retlib_server(QString,QTcpSocket*)));
        connect(data, SIGNAL(error_server(QString,QTcpSocket*)), this, SLOT(errorlib_server(QString,QTcpSocket*)));
        // processa o AJAX
        data->socket = socket;
        data->open("command=vdalib&param=" + mesa + "&arrayitens=" + arrayitens);

        // envia o retorno do inicio do processo
        return retsuccess;

    }

    int vdaLib::get_countmember(QString input) {
        // retorna a quantidade de itens que um membro add pode ter

        int temp = 0;

        for (int x = 0; x < input.count(); x++) {

            if (input[x] == QLatin1Char('{')) {
                temp++;
            }
        }

        return temp;
    }

    void vdaLib::build_members(QString input, QString nivel, QString sep) {
        // add membros opcionais a coleção

        if (input.count() == 0) {
            return;
        }

        datamember *member;
        QString coluna1 = "";
        QString coluna2 = "";
        int fase = 1;

        for (int x = 0; x < input.count(); x++) {

            if (input[x] == QLatin1Char('{')) {
                // inicio de registro
                coluna1 = "";
                coluna2 = "";
                fase = 1;
            }
            else if (input[x] == QLatin1Char('}')) {
                // final de registro

                member = new datamember();
                member->des = coluna1;
                member->val = coluna2;
                member->tip = nivel;
                member->sep = sep;
                this->lst_members.append(member);
            }
            else if (input[x] == QLatin1Char('|')) {
                // divisão de coluna
                fase = 2;
            }
            else {
                // dados
                if (fase == 1) {

                    coluna1 = coluna1 + QChar(input[x]);
                }
                else if (fase == 2) {

                    coluna2 = coluna2 + QChar(input[x]);
                }
            }
        }
    }

    QString vdaLib::build_detail(QString title, QString part1, QString part2, QString part3) {
        // construi os detalhes

        if (this->lst_members.count() == 0) {
            return "";
        }

        // caracter Enter dentro de um QSTRING
        QChar ret1 = '\r';
        QChar ret2 = '\n';
        QString newline = "";
        newline = newline + ret1;
        newline = newline + ret2;

        QString temp;
        bool f11 = false;
        bool f12 = false;
        bool f21 = false;
        bool f22 = false;
        bool f31 = false;
        bool f32 = false;
        bool f4 = false;
        bool cabec1 = false;
        bool cabec2 = false;
        bool cabec3 = false;

        temp = temp + "Pedido: " + newline;
        temp = temp + title + newline;
        temp = temp + "Opção de Tamanho: " + newline;

        for (int x = 0; x < this->lst_members.count(); x++) {

            if (this->lst_members[x]->tip == "0" && this->lst_members[x]->sep == "-") {

                temp = temp + this->lst_members[x]->des + newline;
            }
            else if (this->lst_members[x]->tip == "1" && this->lst_members[x]->sep == "1" ) {

                if (!cabec1) {
                    temp = temp + newline;
                    temp = temp + "Item 1: " + part1 + newline;
                    cabec1 = true;
                }

                if (!f11) {
                    temp = temp + "Adicionar: " + newline;
                    f11 = true;
                }

                temp = temp + this->lst_members[x]->des + newline;
            }
            else if (this->lst_members[x]->tip == "2" && this->lst_members[x]->sep == "1" ) {

                if (!cabec1) {
                    temp = temp + newline;
                    temp = temp + "Item 1: " + part1 + newline;
                    cabec1 = true;
                }

                if (!f12) {
                    temp = temp + "Remover: " + newline;
                    f12 = true;
                }

                temp = temp + this->lst_members[x]->des + newline;
            }
            else if (this->lst_members[x]->tip == "1" && this->lst_members[x]->sep == "2" ) {

                if (!cabec2) {
                    temp = temp + newline;
                    temp = temp + "Item 2: " + part2 + newline;
                    cabec2 = true;
                }

                if (!f21) {
                    temp = temp + "Adicionar: " + newline;
                    f21 = true;
                }

                temp = temp + this->lst_members[x]->des + newline;
            }
            else if (this->lst_members[x]->tip == "2" && this->lst_members[x]->sep == "2" ) {

                if (!cabec2) {
                    temp = temp + newline;
                    temp = temp + "Item 2: " + part2 + newline;
                    cabec2 = true;
                }

                if (!f22) {
                    temp = temp + "Remover: " + newline;
                    f22 = true;
                }

                temp = temp + this->lst_members[x]->des + newline;
            }
            else if (this->lst_members[x]->tip == "1" && this->lst_members[x]->sep == "3" ) {

                if (!cabec3) {
                    temp = temp + newline;
                    temp = temp + "Item 3: " + part3 + newline;
                    cabec3 = true;
                }

                if (!f31) {
                    temp = temp + "Adicionar: " + newline;
                    f31 = true;
                }

                temp = temp + this->lst_members[x]->des + newline;
            }
            else if (this->lst_members[x]->tip == "2" && this->lst_members[x]->sep == "3" ) {

                if (!cabec3) {
                    temp = temp + newline;
                    temp = temp + "Item 3: " + part3 + newline;
                    cabec3 = true;
                }

                if (!f32) {
                    temp = temp + "Remover: " + newline;
                    f32 = true;
                }

                temp = temp + this->lst_members[x]->des + newline;
            }
            else if (this->lst_members[x]->tip == "3" && this->lst_members[x]->sep == "-" ) {

                if (!f4) {
                    temp = temp + newline;
                    temp = temp + "Tipo de Recheio: " + newline;
                    f4 = true;
                }

                temp = temp + this->lst_members[x]->des + newline;
            }
        }

        return temp;
    }

    bool vdaLib::insertitem(QString mesa) {
        // lança um produto na base de dados do MM
        // quando uma configuração de venda foi feita

        // reset padrão
        bool retsuccess = true;
        this->lst_members.clear();
        this->pos_member = 0;
        this->pos_mesa   = mesa;

        // obtem um numero de sessão (index)
        this->pos_index  = this->randInt(1, 1000);

        // determina qual item usar como padrão no caso do combo
        QString load_descritivo     = "";
        QString load_configsettings = "vda_api";
        QString altval              = "0";
        QString data_tamanho        = "";
        QString data_recheio        = "";
        QString data_add1           = "";
        QString data_rem1           = "";
        QString data_add2           = "";
        QString data_rem2           = "";
        QString data_add3           = "";
        QString data_rem3           = "";
        QString title1              = "";
        QString title2              = "";
        QString title3              = "";
        QString detalhes            = "";

        float item1_val = 0;
        float item2_val = 0;
        float item3_val = 0;
        float fixvalue  = 0;
        float op_val    = 0;

        // verifica se temos um combo
        if (this->get_combinados() > 1) {

            QSettings item1("esffera", "mmFastFood");
            item1.beginGroup("vda_api");
            title1 = item1.value("des").toString();
            load_descritivo = load_descritivo + "1/1 " + item1.value("des").toString();
            item1_val = item1.value("tb1").toString().replace(",",".").toFloat();
            op_val += item1.value("total_op").toString().replace(",",".").toFloat();
            data_tamanho = item1.value("tamanho").toString();
            data_recheio = item1.value("recheios").toString();
            data_add1 = item1.value("adicionais").toString();
            data_rem1 = item1.value("remover").toString();
            item1.endGroup();

            this->build_members(data_tamanho, "0", "-");
            this->build_members(data_add1, "1", "1");
            this->build_members(data_rem1, "2", "1");

            QSettings item2("esffera", "mmFastFood");
            item2.beginGroup("vda_api_v2");
            title2 = item2.value("des").toString();
            load_descritivo = load_descritivo + " 1/2 " + item2.value("des").toString();
            item2_val = item2.value("tb1").toString().replace(",",".").toFloat();
            op_val += item2.value("total_op").toString().replace(",",".").toFloat();
            data_add2 = item2.value("adicionais").toString();
            data_rem2 = item2.value("remover").toString();
            item2.endGroup();

            this->build_members(data_add2, "1", "2");
            this->build_members(data_rem2, "2", "2");

            if (this->get_combinados() == 3) {

                QSettings item3("esffera", "mmFastFood");
                item3.beginGroup("vda_api_v3");
                title3 = item3.value("des").toString();
                load_descritivo = load_descritivo + " 1/3 " + item3.value("des").toString();
                item3_val = item3.value("tb1").toString().replace(",",".").toFloat();
                op_val += item3.value("total_op").toString().replace(",",".").toFloat();
                data_add3 = item3.value("adicionais").toString();
                data_rem3 = item3.value("remover").toString();
                item3.endGroup();

                this->build_members(data_add3, "1", "3");
                this->build_members(data_rem3, "2", "3");
            }

            if (item1_val > item2_val) {

                load_configsettings = "vda_api";
                fixvalue = item1_val;
            }
            else {

                load_configsettings = "vda_api_v2";
                fixvalue = item2_val;
            }

            if (fixvalue < item3_val) {

                load_configsettings = "vda_api_v3";
                fixvalue = item3_val;
            }

            // no caso dos combos o valor já vai definido = 1
            altval   = "1";
            fixvalue = (fixvalue + op_val);

            this->build_members(data_recheio, "3", "-");
        }
        else if (this->get_combinados() == 1) {
            // sem combo

            QSettings itemU("esffera", "mmFastFood");
            itemU.beginGroup("vda_api");
            title1 = itemU.value("des").toString();
            load_descritivo = itemU.value("des").toString();
            fixvalue = itemU.value("tb1").toString().replace(",",".").toFloat();
            op_val += itemU.value("total_op").toString().replace(",",".").toFloat();
            data_tamanho = itemU.value("tamanho").toString();
            data_recheio = itemU.value("recheios").toString();
            data_add1 = itemU.value("adicionais").toString();
            data_rem1 = itemU.value("remover").toString();
            itemU.endGroup();

            if (op_val > 0) {
                // no caso de opcionais com valor o vai definido = 1
                altval = "1";
            }

            fixvalue = (fixvalue + op_val);

            this->build_members(data_tamanho, "0", "-");
            this->build_members(data_add1, "1", "1");
            this->build_members(data_rem1, "2", "1");
            this->build_members(data_recheio, "3", "-");
        }

        // verifica se tem algum item para ser inserido
        QSettings settings("esffera", "mmFastFood");
        settings.beginGroup(load_configsettings);

        QString cod    = settings.value("cod").toString();
        QString coz    = settings.value("coz").toString();
        this->pos_code = cod;

        qDebug() << "Codigo: " << cod << " Cozinha: " << coz;

        if (cod.length() == 0) {
            // nenhum produto carregado
            qDebug() << "Nenhum produto está salvo na memoria local para ser inserido";
            return false;
        }

        if (load_descritivo.count() == 0) {

            load_descritivo = settings.value("des").toString();
        }

        // calcula o valor do produto
        float calc_qtd   = settings.value("qtd").toString().replace(",",".").toFloat();
        float calc_cus   = fixvalue;
        float calc_total = 0;

        calc_total = (calc_qtd * calc_cus);

        // item de cozinha ?
        if (coz != "0") {
            // busca as informações dos adicionais
            // gera os detalhes
            detalhes = this->build_detail(load_descritivo, title1, title2, title3);
        }

        // constroi o objeto para transmitir os dados
        QString cus = QString::number( calc_cus, 'f', 2 );
        cus.replace(".", ",");
        QString custot = QString::number( calc_total, 'f', 2 );
        custot.replace(".", ",");

        QJsonObject object {

            {"index", this->pos_index},
            {"cod", settings.value("cod").toString()},
            {"ean", settings.value("ean").toString()},
            {"des", load_descritivo},
            {"uni", settings.value("uni").toString()},
            {"qtd", settings.value("qtd").toString()},
            {"cus", cus},
            {"custot", custot},
            {"ipi", settings.value("ipi").toString()},
            {"icm", settings.value("icm").toString()},
            {"tb1", settings.value("tb1").toString()},
            {"tb2", settings.value("tb2").toString()},
            {"tb3", settings.value("tb3").toString()},
            {"tb4", settings.value("tb4").toString()},
            {"bru", settings.value("bru").toString()},
            {"liq", settings.value("liq").toString()},
            {"ncm", settings.value("ncm").toString()},
            {"tiplan", "0"},
            {"altval", altval},
            {"cliente", "Outros"},
            {"detalhes", detalhes},
            {"usuario", mmcore::config::getusername()},
            {"rodizio", "0"},                                   /* ainda fixar isso aqui rs */
            {"hora", settings.value("hor").toString()}
        };

        settings.endGroup();

        // gera o Json
        QString arrayitens = this->buildJSon(object);

        // envia para a API
        mmcore::database *data = new mmcore::database(this);
        // sinais de resposta
        connect(data, SIGNAL(ret(QString)), this, SLOT(retlib(QString)));
        connect(data, SIGNAL(error(QString)), this, SLOT(errorlib(QString)));
        // processa o AJAX
        data->open("command=vdalib&param=" + mesa + "&arrayitens=" + arrayitens);

        // envia o retorno do inicio do processo
        return retsuccess;
    }

    QString vdaLib::buildJSon(QJsonObject input) {
        // obtem um QJsonObject do produto para gerar o comando de inserção

        /*
         * Formato requerido
         *
            QJsonObject object {

                {"property1", 1},
                {"property2", 2}
            };
        */

        // pega object
        QJsonDocument doc(input);
        QString strJson(doc.toJson(QJsonDocument::Compact));

        return "[" + strJson + "]";
    }

    int vdaLib::randInt(int low, int high) {
        // gera um número randomico para o index do produto

        bool flag = true;
        unsigned int num, i;

        /* garante que o numero de sessão não se repete nas ultimas 100 comunicacoes */
        while (flag) {

            num = qrand() % ((high + 1) - low) + low;
            flag = false;

            for (i = 0; i < LISTANUMSESSAO; i++) {

                if(num == listanumeroSessao[i]) {
                    /* numero de sessao repetido, necessario gerar outro */
                    flag = true;
                }
            }
        }

        listanumeroSessao[ultimonumero] = num;
        ultimonumero = (ultimonumero + 1) % LISTANUMSESSAO;
        return num;
    }

    void vdaLib::insertdet() {
        // insere os adicionais de produtos de cozinha

        // sincroniza o item da vez
        int x = this->pos_member;

        // objeto com os dados AJAX
        QJsonObject object {

            {"des", this->lst_members[x]->des},
            {"val", this->lst_members[x]->val},
            {"tip", this->lst_members[x]->tip},
            {"sep", this->lst_members[x]->sep}

        };

        // prepara o proximo item da sequencia
        this->pos_member = (x + 1);
        QString index = QString::number(this->pos_index);

        // gera o Json
        QString arrayitens = this->buildJSon(object);

        // envia para a API
        mmcore::database *data = new mmcore::database(this);
        // sinais de resposta
        connect(data, SIGNAL(ret(QString)), this, SLOT(retlib(QString)));
        connect(data, SIGNAL(error(QString)), this, SLOT(errorlib(QString)));

        // processa o AJAX
        data->open("command=vdalib_det&param=" + this->pos_mesa
                   + "&codigoitem=" + this->pos_code
                   + "&index=" + index
                   + "&arrayopdet=" + arrayitens);
    }

    void vdaLib::retlib(QString buffer) {
        // sucesso no comando

        qDebug() << buffer;

        // verifica se ainda tem itens de cozinha para enviar
        if (this->lst_members.count() > 0) {

            if (this->pos_member < this->lst_members.count()) {

                // ainda tem itens para enviar
                this->insertdet();
                return;
            }
        }

        // se chegou aqui é porque terminou todos os processos
        emit this->response(true);
    }

    void vdaLib::errorlib(QString buffer) {
        // erro

        qDebug() << buffer;
        emit this->response(false);
    }

    void vdaLib::retlib_server(QString buffer, QTcpSocket *socket) {
        // sucesso no comando no caso de terminal

        qDebug() << buffer;
        emit this->response_server(true, socket);
    }

    void vdaLib::errorlib_server(QString buffer, QTcpSocket *socket) {
        // erro no caso de terminal

        qDebug() << buffer;
        emit this->response_server(false, socket);
    }

    int vdaLib::get_combinados() {
        // obtem a lista de combinados

        QSettings settings("esffera", "mmFastFood");
        settings.beginGroup("pedido_cozinha");

            int qtd = settings.value("qtdcombinados").toInt();

        settings.endGroup();

        return qtd;
    }
}
