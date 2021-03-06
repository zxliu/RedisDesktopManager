#pragma once

#include <QTabWidget>
#include "basetab.h"

class ConsoleTabs : public QTabWidget
{
    Q_OBJECT

public:
    ConsoleTabs(QWidget * parent = nullptr);

    // Default QTabWidget interface, only for widget tabs
    int addTab(QWidget*w, const QIcon&i, const QString &l) { return QTabWidget::addTab(w, i, l); }
    int addTab(QWidget*w, const QString &l) { return QTabWidget::addTab(w, l); }

    // App interface, for functional tabs
    void addTab(QSharedPointer<BaseTab> tab);
    void replaceCurrentTab(QSharedPointer<BaseTab> tab);
    void closeTab(unsigned int index);    

protected:
    void closeCurrentTabWithValue();

    /** @return >=0 if exist **/
    int getTabIndex(QString&);    

protected slots:
    void OnError(const QString &);
    void OnTabClose(int i);    

private:
    QList<QSharedPointer<BaseTab>> m_tabs;
};

