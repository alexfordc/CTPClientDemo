//
// Created by patrick on 1/3/17.
//

#include "initialize.h"
#include "GVAR.h"
#include <QDebug>
#include <QFile>
#include <QSettings>
#include <iostream>
#include <QtCore>

using namespace std;

//��ȡ�ĵ��ҵ�ǰ�û���ַ
void iniFrontAdress(){
    //QString path = QCoreApplication::applicationDirPath();
//    QString path = QDir::current().path();
//    std::cout << path.toStdString() << endl;
//    path.append("/ini/front.ini");
//    cout << path.toStdString() << endl;
//    QFile iniFile(path);
    QFile iniFile("./ini/front.ini");
    if (!iniFile.open(QIODevice::ReadOnly|QIODevice::Text)){
        qDebug() << "cannot find front.ini";
        cerr << "����,�Ҳ���front.ini" << endl;
        abort();
    }
    QTextStream in(&iniFile);
    FRONT_ADDRESS = in.readLine();
    iniFile.close();
}

////��ȡ�ĵ���ʼ����Լ��Ϣ
//void iniInstrument(){
//    QFile iniFile("ini/instrumentInfo.ini");
//    if (!iniFile.open(QIODevice::ReadOnly | QIODevice::Text)){
//        qDebug() << "cannot find instrumentInfo.ini";
//        cerr << "����, �Ҳ���instrumentInfo.ini" << endl;
//        abort();
//    }
//    QTextStream in(&iniFile);
//    QString line;
//    while (!in.atEnd()){
//        line = in.readLine();
//        QStringList list = line.split("|");
//        QString code = list.at(0).trimmed();
//        QString name = list.at(1).trimmed();
//        instruList.insert(make_pair(code, new Instrument(code, name)));
//    }
//    iniFile.close();
//}

//��ȡ�ļ���ʼ�����ݿ���Ϣ
void iniDB(){
    QSettings setting("./ini/db.ini", QSettings::IniFormat);
    DB_DRIVER_NAME = setting.value("DB_DRIVER_NAME").toString();
    DB_HOST_NAME = setting.value("DB_HOST_NAME").toString();
    DATABASE_NAME = setting.value("DATABASE_NAME").toString();
    USER_NAME = setting.value("USER_NAME").toString();
    PASSWORD = setting.value("PASSWORD").toString();
}