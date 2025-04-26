#include "networkthread.h"

NetworkThread::NetworkThread(const std::string &ip, int port, QObject *parent)
    : QThread(parent), networkClient(new NetworkClient(ip, port)) {}

NetworkThread::~NetworkThread() {
    delete networkClient;
}

void NetworkThread::run() {
    if (!networkClient->initialize() || !networkClient->connect()) {
        emit errorOccurred("Failed to connect to server");
        return;
    }

    std::string data = networkClient->receive();
    if (data.empty()) {
        emit errorOccurred("Failed to receive data");
        return;
    }

    emit dataReceived(QString::fromStdString(data));
}