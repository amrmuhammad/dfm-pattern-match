#ifndef NEWDBINPUTDIALOG_H
#define NEWDBINPUTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>

class NewDBInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewDBInputDialog(const QString &title, const QString &labelText, QWidget *parent = nullptr);
    QString getText() const;

    void setValidationMessage(const QString &message, bool isValid);
    
private slots:
    void validateInput(const QString &text);

private:
    QLineEdit *lineEdit;
    QLabel *validationLabel;
    QDialogButtonBox *buttonBox;
};

#endif // NEWDBINPUTDIALOG_H
