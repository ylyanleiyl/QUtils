#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QComboBox>
#include <QListView>
#include <QListWidget>
#include <QToolButton>
#include <QLayout>
#include <QStyledItemDelegate>

class HisComboItem : public QWidget
{
    Q_OBJECT
public:
    explicit HisComboItem(const QString &text,QWidget *parent = nullptr);
    QString text() const{return m_text;}

signals:
    void itemClicked(const QString &text);

private:
    QString m_text;
    QToolButton *m_pBtn;
};

class HisComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit HisComboBox(QWidget *parent = nullptr);
    ~HisComboBox();


    //同名覆盖
    void addItem(const QString &text, const QVariant &userData = QVariant());
    void addItem(const QIcon &icon, const QString &text, const QVariant &userData = QVariant());
    void addItems(const QStringList &texts);
    void insertItem(int index, const QString &text, const QVariant &userData = QVariant());
    void insertItem(int index, const QIcon &icon, const QString &text, const QVariant &userData = QVariant());
    void insertItems(int index, const QStringList &list);
    void setItemData(int index, const QVariant &value, int role = Qt::UserRole);
    QVariant itemData(int index, int role = Qt::UserRole);
protected:
    virtual void showPopup();
    virtual bool event(QEvent *e);
public slots:
    void setContext(QStringList &uiItems);
    void initUiItem();

signals:
    void itemRemoved(const QString &text);

private:
    QListWidget *m_listWgt;
    QString m_iniName{"ComboBox.ini"};
    QString m_context{"ComboBox"};
    QList<QVariant> m_roles;
};

#endif // COMBOBOX_H
