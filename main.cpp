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

#define LAST_TIME_SCAN 1800

int timeOut, interval, lastimeScan, scanTimeDelay;
QString exceptVehicle;
QString hostName;
int port;
static const QString googleApiKey1 = "AIzaSyAssx3QfuwQQdIDy92yTYbbgL4Ap-9HGDw";
static const QString googleApiKey2 = "AIzaSyAFjSC-PZ4On8y8EjH3HsUs3THzGWxJyDc";
static const QString googleApiKey3 = "AIzaSyDLwz6OzaGZWMAFwv7e0E88POQElM1m8cc";
QSqlDatabase createDBConnection() {
    QSqlDatabase localDatabase;
    if(localDatabase.isDriverAvailable("QMYSQL") == false){
        qDebug() << "Open database error: driver is not available";
        ::exit(1);
    }
    localDatabase=QSqlDatabase::addDatabase("QMYSQL","CheckDatabase");
    QSettings iniFileSetings(qApp->applicationDirPath() + "/config.ini" , QSettings::IniFormat);
    qDebug()<<qApp->applicationDirPath();
    iniFileSetings.setIniCodec("UTF-8");

    iniFileSetings.beginGroup("DatabaseConfig");
    localDatabase.setHostName(iniFileSetings.value("HostName","localhost").toString());
    //localDatabase.setHostName(iniFileSetings.value("HostName","192.168.3.10").toString());
    localDatabase.setDatabaseName(iniFileSetings.value("DatabaseName","CadProVTS").toString());
    localDatabase.setUserName(iniFileSetings.value("UserName","cadpro").toString());
    localDatabase.setPassword( iniFileSetings.value("Password","cadprojsc").toString());
    iniFileSetings.endGroup();

    if(!localDatabase.open()){
        qDebug() << "Open database error"<<localDatabase.lastError().text();
        ::exit(1);
    } else {
        qDebug() << "Database connected!";
    }
    return localDatabase;
}

void closeDBConnection(QSqlDatabase dbConnection){
    dbConnection.close();
}
QString GetLocationGoogleAPI(QString latitude, QString longitude, QString googleApiKey){
    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QEventLoop eventLoop;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    QString location;
     //the HTTP request
    //1s <=10 request
    //24h <= 2500 request
    QNetworkRequest req( QUrl( QString("https://maps.googleapis.com/maps/api/geocode/json?latlng=%2,%3&key=%1")
                               .arg(googleApiKey)
                               .arg(latitude)
                               .arg(longitude)));
    QNetworkReply *reply = mgr.get(req);
    eventLoop.exec(); // blocks stack until "finished()" has been called
    if (reply->error() == QNetworkReply::NoError) {
        //success
        QString data = (QString) reply->readAll();
        location = data.mid(data.indexOf("formatted_address") + 22);
        location = location.left(location.indexOf("geometry") - 13);
        //qDebug() << location;
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
    }
    delete reply;
    return location;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QDir logDir(qApp->applicationDirPath()+"/log");
    logDir.mkpath("./");
    QSqlDatabase localDatabase = createDBConnection();
     while (true){
         QSqlQuery query(localDatabase);
         //select tất cả  bảng cần lấy đđ
         // Kiểm tra thòii gian <=hiện tại - delta
         //--------- và dia diem = ""
        // lấy ra latlong và tra về đia điểm
         // update ngược lại vào csdl
         //xong
         lastimeScan=1800;
         QDateTime scanTime = QDateTime::currentDateTime().addSecs(-lastimeScan);
         //tbl_carlog and tbl_vipham_vantoc-------------------------------------------------------------------------
         QString str="SELECT `carlog_id` as id ,`carlog_thoigian` AS thoigian,'tbl_carlog' as bang, `carlog_kinhdo` as log, `carlog_vido` as lat,'carlog_diadiem' as coldd,'carlog_id' as colid, `carlog_diadiem` as dd  FROM `tbl_carlog`  WHERE `carlog_thoigian`>='"+scanTime.toString("yyyy-MM-dd hh:mm:ss")+"'  AND carlog_diadiem ='1' UNION SELECT `vipham_vantoc_id` as id ,`vipham_vantoc_thoigianbatdau` AS thoigian,'tbl_vipham_vantoc' as bang, `vipham_vantoc_kinhdo` as log, `vipham_vantoc_vido` as lat,'vipham_vantoc_diadiem' as coldd,'vipham_vantoc_id' as colid, `vipham_vantoc_diadiem` as dd FROM tbl_vipham_vantoc WHERE `vipham_vantoc_thoigianbatdau`>='"+scanTime.toString("yyyy-MM-dd hh:mm:ss")+"' AND vipham_vantoc_diadiem ='1' ";
        // query.prepare(str);
        // query.bindValue(":date", scanTime.toString("yyyy-MM-dd hh:mm:ss"));
        query.exec(str);
        qDebug()<<scanTime.toString("yyyy-MM-dd hh:mm:ss") <<str<< query.size();
        QString id, thoigian, bang, carlat, carlong,coldiadiem,diadiem,colid;
        while (query.next()){
             id= query.value(0).toString();
             thoigian = query.value(1).toString();
             bang= query.value(2).toString();
             carlong =query.value(3).toString();
             carlat =query.value(4).toString();
             coldiadiem =query.value(5).toString();
             colid =query.value(6).toString();
            diadiem=GetLocationGoogleAPI(carlat, carlong,googleApiKey1);
           // QString vitritim = diadiem.s

            qDebug()<<"toado:"<< carlat<< carlong<<diadiem;
           // sleep(1);
            QSqlQuery updateDiadiem(localDatabase);
            QString updateDD="UPDATE "+bang + " SET "+coldiadiem + "= '"+diadiem + "' WHERE "+colid + "='"+id + "'";
            if(updateDiadiem.exec(updateDD)){
                qDebug()<< "update carlog ";
            }else{
                qDebug()<< "error carlog";
            }
        }
        //tbl_laixelog-------------------------------------------------------------------------
        QSqlQuery queryLaixelog(localDatabase);
       QString strlaixelog="SELECT `laixelog_id`,`laixelog_thoigianbatdau`, `laixelog_kinhdobatdau`, `laixelog_vidobatdau`, `laixelog_diadiembatdau`, `laixelog_thoigianketthuc`, `laixelog_kinhdoketthuc`, `laixelog_vidoketthuc`, `laixelog_diadiemketthuc` FROM `tbl_laixelog` WHERE `laixelog_thoigianbatdau`>='"+scanTime.toString("yyyy-MM-dd hh:mm:ss")+"'  AND (`laixelog_diadiembatdau`='' OR `laixelog_diadiemketthuc`='') ";
        queryLaixelog.exec(strlaixelog);
        qDebug()<<strlaixelog<< queryLaixelog.size();
       QString laixelog_id, laixelog_kinhdobatdau, laixelog_vidobatdau,laixelog_diadiembatdau,laixelog_kinhdoketthuc,laixelog_vidoketthuc,laixelog_diadiemketthuc;
       while (queryLaixelog.next()){
            laixelog_id= queryLaixelog.value(0).toString();
            laixelog_kinhdobatdau= queryLaixelog.value(2).toString();
            laixelog_vidobatdau= queryLaixelog.value(3).toString();
            laixelog_diadiembatdau= queryLaixelog.value(4).toString();
            laixelog_kinhdoketthuc= queryLaixelog.value(6).toString();
            laixelog_vidoketthuc= queryLaixelog.value(7).toString();
            laixelog_diadiemketthuc= queryLaixelog.value(8).toString();
            qDebug()<< laixelog_id<<laixelog_diadiembatdau<<laixelog_diadiemketthuc;
            if(laixelog_diadiembatdau=="" && laixelog_kinhdobatdau!= "0" && laixelog_vidobatdau != "0"){
                laixelog_diadiembatdau=GetLocationGoogleAPI(laixelog_vidobatdau, laixelog_kinhdobatdau,googleApiKey2);
                qDebug()<<"ddbd:"<< laixelog_vidobatdau<< laixelog_kinhdobatdau<<laixelog_diadiembatdau;
            }
            if(laixelog_diadiemketthuc=="" && laixelog_kinhdoketthuc!= "0" && laixelog_vidoketthuc != "0"){
                laixelog_diadiemketthuc=GetLocationGoogleAPI(laixelog_vidoketthuc, laixelog_kinhdoketthuc,googleApiKey3);
                qDebug()<<"ddkt:"<< laixelog_vidoketthuc<< laixelog_kinhdoketthuc<<laixelog_diadiemketthuc;
            }

           QSqlQuery updateLaixelog(localDatabase);
          QString updateLXstr="UPDATE tbl_laixelog SET laixelog_diadiembatdau= '"+laixelog_diadiembatdau + "' , laixelog_diadiemketthuc=  '"+laixelog_diadiemketthuc + "'  WHERE laixelog_id='"+laixelog_id + "'";
            qDebug()<< updateLXstr ;
           if(updateLaixelog.exec(updateLXstr)){
               qDebug()<< "update laixelog ";
           }else{
               qDebug()<< "error laixelog";
           }
       }
        qDebug() << "Main thead sleep ... " << QString::number(10) <<"(s) ";
        sleep(10);
     }
     return 0;
}
