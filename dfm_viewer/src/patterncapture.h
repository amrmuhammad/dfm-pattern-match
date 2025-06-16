#ifndef PATTERNCAPTURE_H
#define PATTERNCAPTURE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QProcess>

class PatternCapture : public QWidget {
    Q_OBJECT
public:
    PatternCapture(QWidget *parent = nullptr);

private slots:
    void runPatternCapture();
    void readProcessOutput();

private:
    QLineEdit *layoutFileEdit;
    QLineEdit *maskLayerEdit;
    QLineEdit *inputLayersEdit;
    QLineEdit *dbNameEdit;
    QPushButton *runButton;
    QTextEdit *logText;
    QProcess *process;
};

#endif
