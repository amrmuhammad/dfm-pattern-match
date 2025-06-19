#ifndef PATTERNCAPTURE_H
#define PATTERNCAPTURE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QProcess>
#include <QSettings>

class PatternCapture : public QWidget {
    Q_OBJECT
public:
    PatternCapture(QWidget *parent = nullptr);

private slots:
    void browseLayoutFile();
    void runPatternCapture();
    void cancelProcess();
    void readProcessOutput();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    bool validateInputs();
    void loadSettings();
    void saveSettings();

    QLineEdit *layoutFileEdit;
    QLineEdit *maskLayerEdit;
    QLineEdit *inputLayersEdit;
    QLineEdit *dbNameEdit;
    QPushButton *runButton;
    QPushButton *cancelButton;
    QPushButton *browseButton;
    QTextEdit *logText;
    QProcess *process;
    QSettings *settings;
};

#endif
