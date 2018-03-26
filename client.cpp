#include "client.h"
#include <QMap>

Client::Client(QWidget *parent) : 
	QWidget(parent),
	ui(new Ui::Client),
	tcpSocket(new QTcpSocket(this))
{
	ui->setupUi(this);
	srand(QTime::currentTime().msec() + QTime::currentTime().second());
	id = rand()%999 + 1;
	
	QString name = QHostInfo::localHostName();
	if (!name.isEmpty()) {
		ui->box_addresses->addItem(name);
		QString domain = QHostInfo::localDomainName();
		if (!domain.isEmpty())
			ui->box_addresses->addItem(name + QChar('.') + domain);
	}
		
	if (name != QLatin1String("localhost"))
		ui->box_addresses->addItem(QString("localhost"));
	
	QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
	for (int i = 0; i < ipAddressesList.size(); i++){
		if(!ipAddressesList.at(i).isLoopback() && 
		   ipAddressesList.at(i).toIPv4Address())
			ui->box_addresses->addItem(ipAddressesList.at(i).toString());
	}
	for (int i = 0; i < ipAddressesList.size(); i++){
		if(ipAddressesList.at(i).isLoopback()&& 
		   ipAddressesList.at(i).toIPv4Address())    
			ui->box_addresses->addItem(ipAddressesList.at(i).toString());
	}
	
	ui->line_edit_port->setValidator(new QIntValidator(1, 65535, this));
	
	connect(ui->button_connect, SIGNAL(clicked(bool)),
			this, SLOT(openConnection()));
	connect(ui->button_quit, SIGNAL(clicked(bool)),
			this, SLOT(close()));
	connect(tcpSocket, SIGNAL(readyRead()),
			this, SLOT(readData()));
	connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError()));
	
	
	ui->box_addresses->setFocus();
	
	QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        
        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();
        
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
            QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }
        networkSession = new QNetworkSession(config, this);
        connect(networkSession, SIGNAL(opened()),
				this, SLOT(sessionOpened()));
        ui->statusLabel->setText(tr("Opening network session."));
        networkSession->open();
    }
		
}

void Client::sessionOpened(){
    QNetworkConfiguration config = networkSession->configuration();
    QString id;
    if(config.type() == QNetworkConfiguration::UserChoice)
        id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
    else id = config.identifier();
    
    QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
    settings.beginGroup(QLatin1String("QtNetwork"));
    settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
    settings.endGroup();

    ui->statusLabel->setText("Just relax and chill! Have a good day ^_^");
}

void Client::openConnection(){
	hostAddress = ui->box_addresses->currentText();
	hostPort = ui->line_edit_port->text().toInt();
	
	this->close();
	
	tcpSocket->abort();
	tcpSocket->connectToHost(hostAddress,
							 hostPort);
	
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	QString data = QString::number(id) + ":0000";
	
	out << data;
	tcpSocket->write(block);
}

void Client::createTestRequest(){
	hostAddress = ui->box_addresses->currentText();
	hostPort = ui->line_edit_port->text().toInt();
	
	this->close();
	
	tcpSocket->abort();
	tcpSocket->connectToHost(hostAddress,
							 hostPort);
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	
	QString data = QString::number(id) + ":0010";
	out << data;
	tcpSocket->write(block);
}


void Client::readData(){
	QDataStream in;
	in.setDevice(tcpSocket);
	
	QString data;
	in >> data;
	qDebug() << data;
	QString code = data.split(":")[0];
	
	if (code == "0020")
		emit checkedSuccess();
	if (code == "0001")
		emit connectionReady();
	if (code == "0220")
		emit gotBalance(data.split(":")[1]);
	if (code == "0123"){
		emit badCardInfo();
		allTran.pop_back();
	}
	if (code == "0124"){
		emit badCardNumber();
		allTran.pop_back();
	}
	if (code == "0120")
		emit payTransactionGood();
	if (code == "0125"){
		emit notEnoughMoney();
		allTran.pop_back();
	}
	if (code == "0540")
		emit sumsUpAreOk();
	if (code == "0127")
		emit sumsUpArentOk();
	if (code == "0126"){
		emit badRequestToPay();
		allTran.pop_back();
	}
	if (code == "0740")
		emit noTransactionsForThisCard();
	if (code == "0720")
		emit gotTransactions(data.split(":")[1]);
	if (code == "0620"){
		emit refundComplete();
		for (int i = 0; i < allTran.size(); i++){
			if (allTran[i].idTran == data.split(":")[1].toInt()){
				allTran.remove(i);
				break;
			}
		}
	}
	if (code == "0650")
		emit somethingBadWithRefund();
}

void Client::checkBalance(QString card){
	tcpSocket->abort();
	tcpSocket->connectToHost(hostAddress,
							 hostPort);
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	
	QString data = QString::number(id) + ":0210:" + 
				   card;
	out << data;
	tcpSocket->write(block);
}

void Client::startPayTransaction(QString mon, QString card){
	tcpSocket->abort();
	tcpSocket->connectToHost(hostAddress,
							 hostPort);
	idTran++;
	allTran.push_back(myTransaction{idTran, card.split(":")[0], mon});
	
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	
	QString data = QString::number(id) + ":0110:" + mon + ":" + 
				   card;
	out << data;
	tcpSocket->write(block);
}

void Client::saveAllTransactions(){
	QFile file("transactions.txt");
	file.open(QIODevice::WriteOnly);
	QTextStream outFile(&file);
	
	outFile << QString::number(allTran.size()) + "\r\n";
	
	QMap<QString, int> summary;
	for (int i = 0; i < allTran.size(); i++){
		if(!summary.keys().contains(allTran[i].card))
			summary[allTran[i].card] = 0;
		
		outFile << QString::number(allTran[i].idTran) + " "
			   + allTran[i].card + " "
			   + allTran[i].amountTran + "\r\n";
		summary[allTran[i].card] = summary[allTran[i].card] + allTran[i].amountTran.toInt();
	}	
	file.close();
	
	tcpSocket->abort();
	tcpSocket->connectToHost(hostAddress,
							 hostPort);
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	QString sumUp;
	for (int i = 0; i < summary.keys().size(); i++){
		sumUp = summary.keys()[i] + " " + QString::number(summary[summary.keys()[i]]) + "\r\n";
	}
	QString data = QString::number(id) + ":0530:" + sumUp;
	out << data;
	tcpSocket->write(block);
	
}


QString Client::getTransactions(QString cardNum){
	QString toReturn = "";
	for (int i = 0; i < allTran.size(); i++)
		if (allTran[i].card == cardNum)
			toReturn += " id:" + QString::number(allTran[i].idTran) + " "
						+ allTran[i].amountTran + "$" + "\n";
	return toReturn;
}


void Client::requestTransactions(QString cardNum){
	tcpSocket->abort();
	tcpSocket->connectToHost(hostAddress,
							 hostPort);
	
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	
	
	QString data = QString::number(id) + ":0710:" + cardNum; // transactions request
	out << data;
	tcpSocket->write(block);
}

void Client::requestToRefund(int num){
	tcpSocket->abort();
	tcpSocket->connectToHost(hostAddress,
							 hostPort);
	
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	
	qDebug() << num << allTran[num].idTran;
	
	QString data = QString::number(id) + ":0610:" + allTran[num].idTran; // request to refund
	out << data;
	tcpSocket->write(block);	
}
