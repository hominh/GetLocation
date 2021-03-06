#include <QString>
#include <QDebug>
#include "pingmodel.h"

PingModel::PingModel(QObject *parent) : QObject(parent), running(false)
{
    ping = new QProcess(this);
    connect(ping, SIGNAL(started()), this, SLOT(verifyStatus()));
    connect(ping, SIGNAL(finished(int)), this, SLOT(readResult()));
}
PingModel ::~PingModel()
{

}

void PingModel::verifyStatus()
{
    if(ping->isReadable())
    {
        qDebug() << "read on ...";
        connect(ping, SIGNAL(readyRead()), this, SLOT(readResult()));
        if(ping->canReadLine()){
            qDebug() << "LINE read on ...";
        }
        else {
             qDebug() << "not able to read ...";
        }
    }
}

void PingModel::readResult()
{
    qDebug() << "Result: ";
    running = false;
    qDebug() << "Result: " << ping->readLine();
}

void PingModel::start_command()
{
    if(ping)
    {

        QString command = "ping";
        QStringList args;
        args << "-w" <<  "3" <<  "192.178.3.10";
        ping->start(command, args);
        ping->waitForStarted(7000);
        running = true;
        ping->waitForFinished(5000);

    }
}

bool PingModel::is_running(){
    return running;
}

bool PingModel::finished(){
    return ping->atEnd();
}
