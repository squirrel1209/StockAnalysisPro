#ifndef NETWORKTHREAD_H
#define NETWORKTHREAD_H

#include <QThread>
#include "NetworkClient.h"

class NetworkThread : public QThread {
    Q_OBJECT
public:
    NetworkThread(const std::string &ip, int port, QObject *parent = nullptr);
    ~NetworkThread();

protected:
    void run() override;

signals:
    void dataReceived(const QString &jsonData);
    void errorOccurred(const QString &error);

private:
    NetworkClient *networkClient;
};

#endif // NETWORKTHREAD_H