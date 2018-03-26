#ifndef CLIENT_H
#define CLIENT_H

#include "ui_client.h"

#include <QtCore>
#include <QtGui>
#include <QWidget>
#include <QTcpSocket>
#include <QNetworkSession>
#include <QNetworkConfiguration>
#include <QNetworkConfigurationManager>
#include <QTime>
#include <QHostInfo>

namespace Ui {
class Client;
}

struct myTransaction{
	int idTran;
	QString card;
	QString amountTran;
};

class Client : public QWidget
{
	Q_OBJECT
public:
	explicit Client(QWidget *parent = nullptr);
	
signals:
	void checkedSuccess();
	void connectionReady();
	void gotBalance(QString);
	void badCardInfo();
	void badCardNumber();
	void payTransactionGood();
	void notEnoughMoney();
	void sumsUpAreOk();
	void sumsUpArentOk();
	void badRequestToPay();
	void gotTransactions(QString);
	void noTransactionsForThisCard();
	void refundComplete();
	void somethingBadWithRefund();
public slots:
	void createTestRequest();
	void readData();
	void sessionOpened();
	void openConnection();
	void startPayTransaction(QString, QString);
	void checkBalance(QString card);
	void saveAllTransactions();
	void requestTransactions(QString cardNum);
	void requestToRefund(int);
		
	QString getTransactions(QString cardNum);
private:
	QTcpSocket* tcpSocket = nullptr;
	int id;
	Ui::Client *ui;
	QNetworkSession *networkSession = nullptr;
	int idTran = 0;
	QVector<myTransaction> allTran;
	
	QString hostAddress;
	int hostPort;

};

#endif // CLIENT_H
