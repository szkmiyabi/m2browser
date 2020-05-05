#ifndef BROWSER_H
#define BROWSER_H

#include <QMainWindow>

class browser : public QMainWindow
{
    Q_OBJECT

public:
    browser(QWidget *parent = nullptr);
    ~browser();
};
#endif // BROWSER_H
