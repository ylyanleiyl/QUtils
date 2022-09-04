#include "hiscombobox.h"

#include <QMouseEvent>
#include <QLineEdit>
#include <QStyleOptionViewItem>
#include <QStyle>
#include <QScrollBar>
#include <QDebug>
#include <QSettings>
#include <QApplication>

HisComboItem::HisComboItem(const QString &text, QWidget *parent)
    : QWidget(parent),
      m_text(text),
      m_pBtn(new QToolButton(this))
{
    //m_pBtn->setIcon(QIcon(":/images/del.ico"));
    QStyle* style = QApplication::style();
    QIcon deleteIcon = style->standardIcon(QStyle::SP_DialogCloseButton);
    m_pBtn->setIcon(deleteIcon);
    m_pBtn->setStyleSheet("border:none;");//取消边框
    m_pBtn->setAutoRaise(true);

    QHBoxLayout *layout=new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(m_pBtn);
    setLayout(layout);

    connect(m_pBtn,&QToolButton::clicked,[this]{
        emit itemClicked(m_text);
    });
}

HisComboBox::HisComboBox(QWidget *parent)
    : QComboBox(parent)
    ,m_listWgt(new QListWidget(this))
{
    m_listWgt->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    setEditable(true);
    setModel(m_listWgt->model());
    setView(m_listWgt);

    //处理UI提升的情况下有预先设置item的记录到历史的问题
    QMetaObject::invokeMethod(this, "initUiItem",Qt::QueuedConnection);
}

HisComboBox::~HisComboBox() {
    QSettings iniFile(m_iniName, QSettings::IniFormat);
    //iniFile.setIniCodec("UTF-8");
    iniFile.beginGroup(m_context);
    iniFile.remove("");
    iniFile.setValue("Roles", m_roles);
    iniFile.endGroup();
    iniFile.beginWriteArray(m_context);
    for (int i = 0; i < m_listWgt->count(); i++) {
        iniFile.setArrayIndex(i);
        foreach (QVariant j, m_roles) {
            iniFile.setValue(QString::number(j.toInt()), m_listWgt->item(i)->data(j.toInt()));
        }
    }
    iniFile.endArray();
}

bool HisComboBox::event(QEvent *e)
{
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return)
        {
            QComboBox::InsertPolicy pl = insertPolicy();
            switch (pl) {
            case QComboBox::InsertAtTop:
                insertItem(0,currentText());
                break;
            default:
                addItem(currentText());
                break;
            }
            return true; //已被处理
        }
        return false;
    }
    return QComboBox::event(e);
}

void HisComboBox::setContext(QStringList &uiItems/*const QString &iniName, const QString &context*/)
{
    m_iniName = QString("%1_ComboBox.ini").arg(qAppName());
    m_context = objectName();

    //m_iniName=iniName;
    //m_context=context;
    QSettings iniFile(m_iniName, QSettings::IniFormat);

    int nCount = iniFile.beginReadArray(m_context);
    m_roles = iniFile.value("Roles").toList();
    for (int i = 0; i < nCount; i++) {
        iniFile.setArrayIndex(i);
        //if(uiItems.indexOf(iniFile.value(QString::number(Qt::DisplayRole)).toString())> -1 )
        //if(int idx{uiItems.indexOf(iniFile.value(QString::number(Qt::DisplayRole)).toString())}; idx > -1 )
        //if(int idx = uiItems.indexOf(iniFile.value(QString::number(Qt::DisplayRole)).toString()) ; idx > -1 )
        int idx = uiItems.indexOf(iniFile.value(QString::number(Qt::DisplayRole)).toString()) ;
        if(idx > -1 )
        {
            uiItems.takeAt(idx);
        }

        addItem(iniFile.value(QString::number(Qt::DisplayRole)).toString(),
                iniFile.value(QString::number(Qt::UserRole)).toString());

        foreach (QVariant j, m_roles) {
            m_listWgt->item(count()-1)->setData(j.toInt(),iniFile.value(QString::number(j.toInt())));
        }
    }

}

void HisComboBox::initUiItem()
{
    QStringList slItems;
    for(int i=0;i<count();i++)
    {
        slItems.append(itemText(i));
    }

    clear();

    setContext(slItems);

    insertItems(0,slItems);
}

void HisComboBox::addItem(const QString &text, const QVariant &userData)
{
    QListWidgetItem* pItm = new QListWidgetItem();
    pItm->setData(Qt::DisplayRole,text);
    pItm->setData(Qt::UserRole,userData);
    m_listWgt->addItem(pItm);

    HisComboItem *pWgt=new HisComboItem(text,m_listWgt);
    m_listWgt->setItemWidget(pItm,pWgt);

	if (!m_roles.contains(Qt::DisplayRole))
		m_roles.append(Qt::DisplayRole);

    connect(pWgt,&HisComboItem::itemClicked,this,[this,pWgt,pItm](){
        hidePopup();
        m_listWgt->takeItem(m_listWgt->row(pItm));
        emit itemRemoved(pWgt->text());
        delete pItm;
    });
}

void HisComboBox::addItem(const QIcon &icon, const QString &text, const QVariant &userData)
{
    Q_UNUSED(icon)
    addItem(text,userData);
}

void HisComboBox::addItems(const QStringList &texts)
{
    for(auto text : texts )
    {
        addItem(text);
    }
}

void HisComboBox::insertItem(int index, const QString &text, const QVariant &userData)
{
    QListWidgetItem* pItm = new QListWidgetItem();
    pItm->setData(Qt::DisplayRole,text);
    pItm->setData(Qt::UserRole,userData);
    m_listWgt->insertItem(index,pItm);

    HisComboItem *pWgt=new HisComboItem(text,m_listWgt);
    m_listWgt->setItemWidget(pItm,pWgt);

	if (!m_roles.contains(Qt::DisplayRole))
		m_roles.append(Qt::DisplayRole);

    connect(pWgt,&HisComboItem::itemClicked,this,[this,pWgt,pItm](){
        hidePopup();
        m_listWgt->takeItem(m_listWgt->row(pItm));
        emit itemRemoved(pWgt->text());
        delete pItm;
    });
}

void HisComboBox::insertItem(int index, const QIcon &icon, const QString &text, const QVariant &userData)
{
    Q_UNUSED(icon)
    insertItem(index,text,userData);
}

void HisComboBox::insertItems(int index, const QStringList &list)
{
    for(int i=0;i<list.size();i++)
    {
        insertItem(index+1,list[i]);
    }
}

void HisComboBox::setItemData(int index, const QVariant &value, int role)
{
    m_listWgt->item(index)->setData(role,value);
    if(!m_roles.contains(role))
        m_roles.append(role);
}

QVariant HisComboBox::itemData(int index, int role)
{
    return m_listWgt->item(index)->data(role);
}

void HisComboBox::showPopup()
{
    QComboBox::showPopup();

    int maxWidth=0;
    QFontMetrics fm(this->font());
    for(int idx=0;idx < this->count();idx++)
    {
        QRect rec = fm.boundingRect(this->itemText(idx));
        if(maxWidth < rec.width())
            maxWidth = rec.width();
    }

    if(view()->width() < maxWidth+50 )//+m_pBtn.width()
    {
        view()->setFixedWidth(maxWidth+50);
    }
}
