#include "newDBinputdialog.h"
#include <QVBoxLayout>
#include <QRegularExpression>

NewDBInputDialog::NewDBInputDialog(const QString &title, const QString &labelText, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(title);

    // Create layout
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Create prompt label
    QLabel *promptLabel = new QLabel(labelText, this);
    layout->addWidget(promptLabel);

    // Create input field
    lineEdit = new QLineEdit(this);
    layout->addWidget(lineEdit);

    // Create validation label
    validationLabel = new QLabel("", this);
    validationLabel->setStyleSheet("color: red;");
    layout->addWidget(validationLabel);

    // Create buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);

    // Connect signals
    connect(lineEdit, &QLineEdit::textChanged, this, &CustomInputDialog::validateInput);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Initial validation
    validateInput(lineEdit->text());
}

void NewDBInputDialog::validateInput(const QString &text)
{
    // Example validation: non-empty and only alphanumeric, underscore, or hyphen
    QRegularExpression re("^[a-zA-Z0-9_-]+$");
    bool isValid = !text.isEmpty() && re.match(text).hasMatch();

    if (isValid) {
        validationLabel->setText("Valid input");
        validationLabel->setStyleSheet("color: green;");
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    } else {
        validationLabel->setText("Invalid input: Use alphanumeric, underscore, or hyphen only");
        validationLabel->setStyleSheet("color: red;");
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

QString CustomInputDialog::getText() const
{
    return lineEdit->text();
}
