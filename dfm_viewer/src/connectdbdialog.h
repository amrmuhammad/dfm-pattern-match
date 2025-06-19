#ifndef CONNECTDBDIALOG_H
#define CONNECTDBDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "databaseviewer.h"

class ConnectDbDialog : public QDialog {
    Q_OBJECT
public:
    explicit ConnectDbDialog(QWidget *parent = nullptr, DatabaseViewer *viewer = nullptr);

private slots:
    void tryConnect();

private:
    QLineEdit *dbNameEdit;
    QPushButton *connectButton;
    QLabel *errorLabel;
    DatabaseViewer *dbViewer;
};

#endif // CONNECTDBDIALOG_H
