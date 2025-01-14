#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QGraphicsScene;

class MainWindow : public QMainWindow
{
    Q_OBJECT

/* members */
private:
    QString lastDir;

/* constructro & destructor */
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

/* slots */
public slots:
    void openFileSlot();
};

#endif // MAINWINDOW_H
