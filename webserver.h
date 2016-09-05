#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QWebSocketServer>

class WebServer : public QWebSocketServer
{
    Q_OBJECT
public:
    explicit WebServer(QObject *parent = 0);

    void _sendTextMessage(const QString &Message);
signals:
   void messageReceived(QString);
private slots:
   void wsNewConnection();
   void onMessageReceived(const QString &Value);

private:
    QWebSocket *mSocket;
};

#endif // WEBSERVER_H
