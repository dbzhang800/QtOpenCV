/****************************************************************************
** Copyright (c) 2012-2015 Debao Zhang <hello@debao.me>
** All right reserved.
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
** LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
** OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
** WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
****************************************************************************/
#include "recentfiles.h"
#include <QMenu>
#include <QEvent>

RecentFiles::RecentFiles(QObject *parent) :
    QObject(parent), m_maxCount(50), m_maxPathLength(400), m_dirty(true), m_menu(new QMenu("Recent Files"))
{

    m_menu->installEventFilter(this);
}

RecentFiles::~RecentFiles()
{
    if (!m_menu.isNull())
        delete m_menu;
}

QMenu *RecentFiles::menu() const
{
    return m_menu;
}

int RecentFiles::maxFileCount() const
{
    return m_maxCount;
}

void RecentFiles::setMaxFileCount(int maxCount)
{
    m_maxCount = maxCount;
    if (m_maxCount > m_fileList.count())
        m_fileList = m_fileList.mid(0, m_maxCount);
}

QStringList RecentFiles::fileList() const
{
    return m_fileList;
}

void RecentFiles::setFileList(const QStringList &fileList)
{
    m_fileList = fileList;
    m_dirty = true;
}

QString RecentFiles::mostRecentFile() const
{
    return m_fileList.isEmpty() ? QString() : m_fileList.at(0);
}

void RecentFiles::add(const QString &filePath)
{
    if (m_fileList.count() > 0 && m_fileList[0] == filePath)
        return;

    m_fileList.removeAll(filePath);
    m_fileList.insert(0, filePath);
    while (m_fileList.count() > m_maxCount)
        m_fileList.removeLast();

   m_dirty = true;
}

void RecentFiles::clear()
{
    if (m_fileList.isEmpty())
        return;
    m_fileList.clear();
    m_dirty = true;
}

void RecentFiles::remove(const QString &filePath)
{
    if (m_fileList.removeAll(filePath))
        m_dirty = true;
}

void RecentFiles::onActionTriggered()
{
    QAction *act = qobject_cast<QAction *>(sender());
    if (!act)
        return;

    emit activated(act->data().toString());
}

bool RecentFiles::eventFilter(QObject *obj, QEvent *evt)
{
    if (obj == m_menu && evt->type() == QEvent::Show && m_dirty) {
        m_menu->clear();
        foreach (QString filePath, m_fileList) {
            QString title = QFontMetrics(m_menu->font()).elidedText(filePath, Qt::ElideMiddle, m_maxPathLength);
            QAction *act = new QAction(title, m_menu.data());
            act->setData(filePath);
            m_menu->addAction(act);

            connect(act, SIGNAL(triggered()), SLOT(onActionTriggered()));
        }
        m_menu->addSeparator();
        QAction *clearAction = m_menu->addAction(tr("Clear Menu"), this, SLOT(clear()));
        clearAction->setDisabled(m_fileList.isEmpty());
        m_dirty = false;
    }
    return QObject::eventFilter(obj, evt);
}

