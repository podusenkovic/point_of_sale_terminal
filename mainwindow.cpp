#define DELAY 1000
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //--------------------------------------------------------
	//this->setStyleSheet("background-color: rgb(41,157,55)"); 
	//TODO cool color on background
	//--------------------------------------------------------
	MENU = "Pay\nRefund\nBalance\nClose\nCheck";
    MENUSIZE = MENU.split('\n').size();
    ui->mainLabel->setAlignment(Qt::AlignCenter);
	ui->mainLabel->setText("To start using thispost,\n connect to host!");
	client = new Client();
	client->show();
	connect(client, SIGNAL(connectionReady()),
			this, SLOT(startEverything()));
	for (int i = 0; i < ui->buttons_num->buttons().size(); i++)
        ui->buttons_num->buttons()[i]->setDisabled(true);
	
	timerToMenu = new QTimer(this);
	timerToMenu->setInterval(DELAY);
	timerToMenu->setSingleShot(true);
	connect(timerToMenu, SIGNAL(timeout()), SLOT(openMenu()));	
	
	timerToError = new QTimer(this);
	timerToError->setInterval(5000);
	timerToError->setSingleShot(true);
	connect(timerToError, SIGNAL(timeout()), SLOT(showError()));	
	
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startWindow(){
    ui->mainLabel->setText("Ready to work!");
	client->disconnect();
}


void MainWindow::inputNumberIntoLabel(QAbstractButton* but){
    ui->mainLabel->setText(ui->mainLabel->text() + but->text());
}


void MainWindow::deleteLabelString(){
    QString abc = ui->mainLabel->text();
    while(abc[abc.size() - 1] != "\n")
        abc.chop(1);
    ui->mainLabel->setText(abc);
}

void MainWindow::doneLabelString(){
    ui->mainLabel->setText(ui->mainLabel->text() + "\n");
}


void MainWindow::openMenu(){
	timerToMenu->stop();
	ui->mainLabel->setAlignment(Qt::AlignHCenter);
	ui->buttons_to_control->disconnect();
	connect(ui->buttons_to_control, SIGNAL(buttonClicked(QAbstractButton*)),
            SLOT(moveMenuChoose(QAbstractButton*)));
	
    ui->button_done->disconnect();
	for (int i = 0; i < ui->buttons_num->buttons().size(); i++)
		ui->buttons_num->buttons()[i]->setDisabled(true);
    connect(ui->button_done, SIGNAL(clicked()), 
            SLOT(completeCommand()));
    
    QString menu = MENU;
    menuChoose = 0;
    int index = 0;
    for(int i = 0; i < menuChoose + 1; i++){
        while(MENU[index]!='\n')
            index++;
    }
    menu.insert(index,' ');
    menu.insert(index + 1,'<');
    inputtedData = ui->mainLabel->text();
    ui->mainLabel->setAlignment(Qt::AlignCenter);
    ui->mainLabel->setText(menu);
}

void MainWindow::moveMenuChoose(QAbstractButton* but){
    QString menu = MENU;
    if (but->text() == "up" && but != nullptr){
        menuChoose = (menuChoose != 0) ? menuChoose - 1 : menuChoose;
    }
    else if (but->text() == "down" && but != nullptr){
        menuChoose = (menuChoose != MENUSIZE-1) ? menuChoose + 1 : menuChoose;
    }
    
    int index = 0;
    for(int i = 0; i < menuChoose + 1; i++){
        index += menu.split('\n')[i].size() + 1;
    }
    
    menu.insert(index - 1,' ');
    menu.insert(index,'<');
    ui->mainLabel->setText(menu);
}

void MainWindow::deletePayInput(){
	ui->mainLabel->setText("Input the price :\n");
}

void MainWindow::sendRequestToPay(){
	QString money = ui->mainLabel->text().split("\n")[1];
	ui->mainLabel->setText("Sending request!");
	timerToError->start();
	for (int i = 0; i < ui->buttons_num->buttons().size(); i++)
		ui->buttons_num->buttons()[i]->setDisabled(true);
	client->startPayTransaction(money, savedCard->cardToString());
	connect(client, SIGNAL(payTransactionGood()),
			this, SLOT(payTransactionGood()));
	connect(client, SIGNAL(notEnoughMoney()), 
			this, SLOT(notEnoughMoney()));
	connect(client, SIGNAL(badRequestToPay()),
			this, SLOT(badRequestToPay()));
	
	ui->button_done->disconnect();
	ui->button_delete->disconnect();
}

void MainWindow::payTransactionGood(){
	timerToError->stop();
	ui->mainLabel->setText("You paid successfully");
	client->disconnect();
	timerToMenu->start();
}

void MainWindow::notEnoughMoney(){
	timerToError->stop();
	ui->mainLabel->setText("You don't have\n enough money");
	client->disconnect();
	timerToMenu->start();
}

void MainWindow::badRequestToPay(){
	timerToError->stop();
	ui->mainLabel->setText("You inputted bad\namount of money");
	client->disconnect();
	timerToMenu->start();	
}

void MainWindow::completeCommand(){
	ui->buttons_to_control->disconnect();
	if (savedCard == nullptr && menuChoose != 4){
		ui->mainLabel->setText("Please, choose card\nto use!");
		timerToMenu->start();
		return;
	}
    //-------------Pay--------------------------------------------
    if (menuChoose == 0){ 
        ui->mainLabel->setText("Input the price :\n");
        for (int i = 0; i < ui->buttons_num->buttons().size(); i++)
            ui->buttons_num->buttons()[i]->setEnabled(true);
		ui->button_done->disconnect();
		connect(ui->button_done, SIGNAL(clicked()), 
				this, SLOT(sendRequestToPay()));
		connect(ui->button_delete, SIGNAL(clicked(bool)),
				this, SLOT(deletePayInput()));
    }
    //-------------Refund-----------------------------------------
    if (menuChoose == 1){ 
		ui->mainLabel->setAlignment(Qt::AlignTop);
		ui->button_done->disconnect();
		this->askHostForTransactions();
    }
    //-------------Balance----------------------------------------
    if (menuChoose == 2){ 
		timerToError->start();
        ui->mainLabel->setText("Connecting to host\nto get a balance!");
		client->checkBalance(savedCard->cardToString());
		connect(client, SIGNAL(gotBalance(QString)),
				this, SLOT(gotBalance(QString)));
		connect(client, SIGNAL(badCardNumber()),
				this, SLOT(badCardNumber()));
		connect(client, SIGNAL(badCardInfo()),
				this, SLOT(badCardInfo()));
    }    
    //-------------Close------------------------------------------
    if (menuChoose == 3){
		timerToError->start();
        ui->mainLabel->setText("Connecting to close\nmy time!");
		client->saveAllTransactions();
		connect(client, SIGNAL(sumsUpAreOk()),
				this, SLOT(sumsUpAreOk()));
		connect(client, SIGNAL(sumsUpArentOk()),
				this, SLOT(sumsUpArentOk()));
    }
    //-------------Check------------------------------------------
    if (menuChoose == 4){
		timerToError->start();
        ui->mainLabel->setText("Checking connection\nTo host");
		client->createTestRequest();
		connect(client, SIGNAL(checkedSuccess()), 
				this, SLOT(checkedSuccess()));		
    }
}


void MainWindow::chooseCard(){
	if (winCard != nullptr){
		winCard->close();
		delete winCard;
	}
	winCard = new Card();
	winCard->show();
	connect(winCard, SIGNAL(accepted()), 
			this, SLOT(getCardToSave()));
}

void MainWindow::getCardToSave(){
	Card* temp = (Card*)sender();
	savedCard = temp->getSavedCard();
	//temp->close(); TODO create nice card view
}


void MainWindow::checkedSuccess(){
	timerToError->stop();
	ui->mainLabel->setText("Checked success");
	ui->button_done->disconnect();
	client->disconnect();
	timerToMenu->start();
}

void MainWindow::gotBalance(QString bal){
	timerToError->stop();
	ui->mainLabel->setText("Balance : \n" + bal);
	ui->button_done->disconnect();
	client->disconnect();
	timerToMenu->start();
}

void MainWindow::badCardNumber(){
	timerToError->stop();
	ui->mainLabel->setText("There is no such card");
	ui->button_done->disconnect();
	client->disconnect();
	timerToMenu->start();
}

void MainWindow::badCardInfo(){
	timerToError->stop();
	ui->mainLabel->setText("Bad data with a card");
	ui->button_done->disconnect();
	client->disconnect();
	timerToMenu->start();
}




void MainWindow::startEverything(){
	connect(ui->buttons_num, SIGNAL(buttonClicked(QAbstractButton*)), 
            SLOT(inputNumberIntoLabel(QAbstractButton*)));
	
    connect(ui->button_menu, SIGNAL(clicked(bool)),
            SLOT(openMenu()));
    connect(ui->button_to_card, SIGNAL(clicked(bool)),
			this, SLOT(chooseCard()));
    startWindow();
}


void MainWindow::keyPressEvent(QKeyEvent *event){
	if (event->key() == Qt::Key_Up)
		ui->button_to_up->animateClick();
	if (event->key() == Qt::Key_Down)
		ui->button_to_down->animateClick();
	if (event->key() == Qt::Key_0)
		ui->button_num_0->animateClick();
	if (event->key() == Qt::Key_1)
		ui->button_num_1->animateClick();
	if (event->key() == Qt::Key_2)
		ui->button_num_2->animateClick();
	if (event->key() == Qt::Key_3)
		ui->button_num_3->animateClick();
	if (event->key() == Qt::Key_4)
		ui->button_num_4->animateClick();
	if (event->key() == Qt::Key_5)
		ui->button_num_5->animateClick();
	if (event->key() == Qt::Key_6)
		ui->button_num_6->animateClick();
	if (event->key() == Qt::Key_7)
		ui->button_num_7->animateClick();
	if (event->key() == Qt::Key_8)
		ui->button_num_8->animateClick();
	if (event->key() == Qt::Key_9)
		ui->button_num_9->animateClick();
	if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)
		ui->button_delete->animateClick();
	if (event->key() == Qt::Key_Return)
		ui->button_done->animateClick();
	if (event->key() == Qt::Key_M)
		ui->button_menu->animateClick();
	if (event->key() == Qt::Key_C)
		ui->button_to_card->animateClick();	
}


void MainWindow::sumsUpAreOk(){
	timerToError->stop();
	ui->mainLabel->setText("Everything is ok!");
	client->disconnect();
	timerToMenu->start();
}

void MainWindow::sumsUpArentOk(){
	timerToError->stop();
	ui->mainLabel->setText("There's something\ndifferent in lists!");
	client->disconnect();
	timerToMenu->start();
}

void MainWindow::moveRefundChoose(QAbstractButton *but){
    //QString myText = "You can get refund this:\n" + client->getTransactions(savedCard->getNumber());
	QString myText = ui->mainLabel->text().split(" <")[0]+ 
					 ui->mainLabel->text().split(" <")[1];
	int refundSize = myText.split("\n").size() - 2;
	qDebug() << refundSize;
    if (but->text() == "up" && but != nullptr){
        refundChoose = (refundChoose != 0) ? refundChoose - 1 : refundChoose;
    }
    else if (but->text() == "down" && but != nullptr){
        refundChoose = (refundChoose != refundSize - 1) ? refundChoose + 1 : refundChoose;
    }
    
    int index = 0;
    for(int i = 0; i < refundChoose + 2; i++){
        index += myText.split('\n')[i].size() + 1;
    }
    
    myText.insert(index - 1,' ');
    myText.insert(index,'<');
    ui->mainLabel->setText(myText);
}

void MainWindow::chooseWhatRefund(){
	ui->button_done->disconnect();
	timerToError->start();
	ui->mainLabel->setText("Asking to refund");
	client->requestToRefund(refundChoose);
	ui->mainLabel->setAlignment(Qt::AlignCenter);
	connect(client, SIGNAL(refundComplete()),
			this, SLOT(refundComplete()));
	connect(client, SIGNAL(somethingBadWithRefund()),
			SLOT(somethingBadWithRefund()));
		
}

void MainWindow::askHostForTransactions(){
	timerToError->start();
	ui->mainLabel->setText("Asking host for transactions");
	
	client->requestTransactions(savedCard->getNumber());
	connect(client, SIGNAL(noTransactionsForThisCard()),
			this, SLOT(noTransactionsForThisCard()));
	connect(client, SIGNAL(gotTransactions(QString)),
			this, SLOT(gotTransactions(QString)));
}

void MainWindow::noTransactionsForThisCard(){
	timerToError->stop();
	ui->mainLabel->setText("No transactions for\nthis card!");
	client->disconnect();
	timerToMenu->start();
}

void MainWindow::gotTransactions(QString tran){
	timerToError->stop();
	QString myText = "You can get refund from:\n" + tran;
	refundChoose = 0;	
	
	int index = 0;
	for(int i = 0; i < refundChoose + 2; i++){
		index += myText.split('\n')[i].size() + 1;
	}
		
	myText.insert(index - 1,' ');
	myText.insert(index,'<');
	ui->mainLabel->setText(myText);
	ui->button_done->disconnect();
	connect(ui->buttons_to_control, SIGNAL(buttonClicked(QAbstractButton*)),
			SLOT(moveRefundChoose(QAbstractButton*)));
	connect(ui->button_done, SIGNAL(clicked(bool)),
			this, SLOT(chooseWhatRefund()));
	client->disconnect();
}


void MainWindow::refundComplete(){
	ui->buttons_to_control->disconnect();
	timerToError->stop();
	ui->mainLabel->setText("Refund complete!");
}

void MainWindow::somethingBadWithRefund(){
	ui->buttons_to_control->disconnect();
	timerToError->stop();
	ui->mainLabel->setText("Refund wasn't completed!");
}

void MainWindow::showError(){
	ui->mainLabel->setText("External host hasn't\nbeen found!\nTry again!");
	client->show();
	connect(client, SIGNAL(connectionReady()),
			this, SLOT(startEverything()));
	for (int i = 0; i < ui->buttons_num->buttons().size(); i++)
        ui->buttons_num->buttons()[i]->setDisabled(true);
	
}
