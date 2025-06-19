#ifndef BATCHPATTERNCAPTURE_H
#define BATCHPATTERNCAPTURE_H

#include <QMainWindow>
#include <QSettings>
#include <QProcess>

class QLineEdit;
class QPushButton;
class QTextEdit;

class BatchPatternCapture : public QMainWindow {
    Q_OBJECT
public:
    explicit BatchPatternCapture(QWidget *parent = nullptr);

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

    QSettings *settings;
    QLineEdit *layoutFileEdit;
    QPushButton *browseButton;
    QLineEdit *maskLayerEdit;
    QLineEdit *inputLayersEdit;
    QLineEdit *dbNameEdit;
    QPushButton *runButton;
    QPushButton *cancelButton;
    QTextEdit *logText;
    QProcess *process;
    QString programPath;
};

#endif // BATCHPATTERNCAPTURE_H
