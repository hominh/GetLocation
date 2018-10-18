#include <QCoreApplication>
#include <QSettings>
#include <QDebug>
#include <QTextStream>
#include <QTextCodec>
#include <iostream>
#include <QFile>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMutex>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <unistd.h>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QProcess>

#define LAST_TIME_SCAN 1800

int timeOut, interval, lastimeScan, scanTimeDelay;
QString exceptVehicle;
QString hostName;
int port;
QString readINI(QString oper)
{
    QSettings settings("CadProTFC.ini", QSettings::IniFormat);
    settings.beginGroup("Ip");
    QString var = settings.value(oper).toString();
    settings.endGroup();
    return var;
}

bool fileExists(QString path) {
    QFileInfo check_file(path);
    if (check_file.exists() && check_file.isFile()) {
        return true;
    } else {
        return false;
    }
}

int main(int argc, char *argv[])
{
    QString ip = readINI("Address");
    QString result;
    QProcess pingProc;
     while (true){
         QDateTime now = QDateTime::currentDateTime();
         QString timestamp = now.toString(QLatin1String("yyyyMMdd"));
         QString filename = timestamp+".log";
         bool checkLogFile = fileExists(filename);
         QFile file(filename);
         QTextStream out(&file);
         if(checkLogFile == true)
         {
             file.open(QFile::Append | QFile::Text);
         }
         else
         {
             file.open(QIODevice::WriteOnly | QIODevice::Text);
         }

        QDateTime now2 = QDateTime::currentDateTime();
        QString timestamp2 = now2.toString(QLatin1String("yyyy-MM-dd hh:mm:ss.zzz"));
        int exitCode=pingProc.execute("ping",QStringList() << "-c" << "1" << ip);

        pingProc.start("ping",QStringList() << "-c" << "1" << ip);

        pingProc.waitForFinished();

        QString line(pingProc.readAllStandardOutput());

        QString time;
        if(line.contains("time="))
        {
            int startofTime = line.indexOf("time=") + 5;
            int endofTime = line.indexOf(" ms");
            int middleTime = endofTime - startofTime;
            time = line.mid(startofTime,middleTime);

            /*qDebug() << "startofTime: "<<startofTime;
            qDebug() << "endofTime: "<<endofTime;
            qDebug() << "time: "<<time;
            qDebug() << "---endtime-----";*/
        }

        QString lane;
        if(ip == "10.10.125.50" || ip == "10.10.126.50" || ip == "10.10.72.50")
        {
            lane = "to lane 3";
        }
        else if(ip == "10.10.125.51" || ip == "10.10.126.51" || ip == "10.10.72.51")
        {
            lane = "to lane 4";
        }

        else if(ip == "10.10.125.40" || ip == "10.10.126.40" || ip == "10.10.72.40")
        {
            lane = "to switch .40";
        }

        else if(ip == "10.10.125.41" || ip == "10.10.126.41" || ip == "10.10.72.41")
        {
            lane = "to switch .41";
        }
		
		else if(ip == "10.10.126.10" || ip == "10.10.125.10" || ip == "10.10.72.10")
		{
			lane = "to antena .10";
		}
		
		else if(ip == "10.10.126.11" || ip == "10.10.125.11" || ip == "10.10.72.11")
		{
			lane = "to antena .11";
		}
		
		else if(ip == "10.10.126.12" || ip == "10.10.125.12" || ip == "10.10.72.12")
		{
			lane = "to antena .12";
		}
		
		else if(ip == "10.10.126.20" || ip == "10.10.125.20" || ip == "10.10.72.20")
		{
			lane = "to antena .20";
		}
		else if(ip == "10.10.126.21" || ip == "10.10.125.21" || ip == "10.10.72.21")
		{
			lane = "to antena .21";
		}
		
		else if(ip == "10.10.126.22" || ip == "10.10.125.22" || ip == "10.10.72.22")
		{
			lane = "to antena .22";
        }
        else if(ip == "192.168.3.10")
        {
            lane = "to local";
        }

        if(exitCode == 0)
        {
            result = timestamp2+ "\t" + "PING "+lane + " OK" + ". Time = " + time;
        }
        else
        {
            result = timestamp2+ "\t" + "PING "+lane + " FAILL";
        }
        out << result << endl;

        qDebug() << "Main thead sleep ... " << QString::number(1) <<"(s) ";
        sleep(1);
     }
     return 0;
}
