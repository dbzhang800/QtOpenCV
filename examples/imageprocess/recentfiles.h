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
#ifndef RECENTFILES_H
#define RECENTFILES_H

#include <QObject>
#include <QStringList>
#include <QPointer>

class QMenu;

class RecentFiles : public QObject
{
    Q_OBJECT
public:
    explicit RecentFiles(QObject *parent = 0);
    ~RecentFiles();

    QMenu *menu() const;

    int maxFileCount() const;
    void setMaxFileCount(int maxCount);

    QStringList fileList() const;
    void setFileList(const QStringList &fileList);

    QString mostRecentFile() const;

signals:
    void activated(const QString &filePath);

public slots:
    void add(const QString &filePath);
    void clear();
    void remove(const QString &filePath);

private slots:
    void onActionTriggered();

private:
    bool eventFilter(QObject *obj, QEvent *evt);
    void updateMenu();
    int m_maxCount;
    int m_maxPathLength;
    QStringList m_fileList;
    bool m_dirty;
    QPointer<QMenu> m_menu;
};

#endif // RECENTFILES_H
