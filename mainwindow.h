#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractButton>
#include "client.h"
#include "card.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void inputNumberIntoLabel(QAbstractButton*);
    void deleteLabelString();
    void doneLabelString();
    void openMenu();
    void moveMenuChoose(QAbstractButton* but);
    void completeCommand();
    void startWindow();
	void chooseCard();
	void getCardToSave();
	void checkedSuccess();
	void gotBalance(QString);
	void startEverything();
	void badCardNumber();
	void badCardInfo();
	void sendRequestToPay();
	void deletePayInput();
	void payTransactionGood();
	void notEnoughMoney();
	void sumsUpAreOk();
	void sumsUpArentOk();
	void badRequestToPay();
	void moveRefundChoose(QAbstractButton *but);
	void chooseWhatRefund();
	void askHostForTransactions();
	void noTransactionsForThisCard();
	void gotTransactions(QString);
	void refundComplete();
	void somethingBadWithRefund();
	void showError();
	
	void keyPressEvent(QKeyEvent *event);
private:
	Client *client;
    QString inputtedData;
    QString MENU;
	int MENUSIZE;
    int menuChoose;
	
	int refundChoose;
	Card *winCard = nullptr;
	cardInfo* savedCard = nullptr;
	
	QTimer *timerToMenu;
	QTimer *timerToError;
	
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
