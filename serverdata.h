#ifndef SERVERDATA_H
#define SERVERDATA_H

#include <QString>

/*
 *  (H) Gerenciador de Micro-Terminal Jaia
 *
 *  Esffera Software
 *  por Renato Igleziaz
 *  08/04/2016
 *
 */

/*
 *
 *  Classe usada como memoria virtual do servidor,
 *  já que ele tem que lidar com vários terminais
 *  de serviço ele precisa de um QList<serverData*>
 *  Objeto para armazenar a sequencia de trabalho
 *  de cada terminal e também garantir a troca
 *  de dados entre eles.
 *
 *  Essa classe é a representação do objeto JSON
 *  usado em AJax para troca de dados. O nosso
 *  sistema recebe os dados por Ajax, então nossa
 *  transmissão de dados é toda por Http.
 *
 */

class serverData
{
public:
    int     op      = 0;  // nivel de operação
    QString ip      = ""; // ip do terminal
    QString mesa    = ""; // mesa/comanda
    QString prodc   = ""; // codigo do produto
    QString prodn   = ""; // descricao do produto
    QString coz     = ""; // cozinha
    QString img     = ""; // imagem
    QString cat     = ""; // cod categoria
    QString val     = ""; // valor
    QString qtd     = ""; // quantidade
    QString ean     = ""; // cod ean
    QString uni     = ""; // unidade
    QString ipi     = ""; // ipi
    QString icm     = ""; // icm
    QString tb1     = ""; // tabelas 1-4
    QString tb2     = "";
    QString tb3     = "";
    QString tb4     = "";
    QString bru     = ""; // peso
    QString liq     = "";
    QString ncm     = ""; // Geralmente guarda alicotas para ECF e SAT
    QString rod     = ""; // rodizio ainda fixar isso
    QString hor     = ""; // hora
};

#endif // SERVERDATA_H
