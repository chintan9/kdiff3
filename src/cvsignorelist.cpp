/***************************************************************************
 * class CvsIgnoreList from Cervisia cvsdir.cpp                            *
 *    Copyright (C) 1999-2002 Bernd Gehrmann <bernd at mail.berlios.de>    *
 * with elements from class StringMatcher                                  *
 *    Copyright (c) 2003 Andre Woebbeking <Woebbeking at web.de>           *
 * Modifications for KDiff3 by Joachim Eibl                                *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "cvsignorelist.h"
#include "TypeUtils.h"

#include <QDir>
#include <QTextStream>

void CvsIgnoreList::init(FileAccess& dir, const t_DirectoryList* pDirList)
{
    static const QString ignorestr = QString::fromLatin1(". .. core RCSLOG tags TAGS RCS SCCS .make.state "
                                                         ".nse_depinfo #* .#* cvslog.* ,* CVS CVS.adm .del-* *.a *.olb *.o *.obj "
                                                         "*.so *.Z *~ *.old *.elc *.ln *.bak *.BAK *.orig *.rej *.exe _$* *$");

    constexpr char varname[] = "CVSIGNORE";

    addEntriesFromString(ignorestr);
    addEntriesFromFile(QDir::homePath() + "/.cvsignore");
    if(qEnvironmentVariableIsSet(varname) && !qEnvironmentVariableIsEmpty(varname))
    {
        addEntriesFromString(QString::fromLocal8Bit(qgetenv(varname)));
    }

    const bool bUseLocalCvsIgnore = cvsIgnoreExists(pDirList);
    if(bUseLocalCvsIgnore)
    {
        FileAccess file(dir);
        file.addPath(".cvsignore");
        if(file.exists() && file.isLocal())
            addEntriesFromFile(file.absoluteFilePath());
        else
        {
            file.createLocalCopy();
            addEntriesFromFile(file.getTempName());
        }
    }
}

void CvsIgnoreList::addEntriesFromString(const QString& str)
{
    QStringList patternList = str.split(' ');
    for(const QString& pattern : patternList)
    {
        addEntry(pattern);
    }
}

/*
    MocIgnoreFile is incapatiable with addEntriesFromFile so do nothing in AUTORUN mode
*/
void CvsIgnoreList::addEntriesFromFile(const QString& name)
{
#ifdef AUTORUN
    Q_UNUSED(name)
#else
    QFile file(name);

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        while(!stream.atEnd())
        {
            addEntry(stream.readLine());
        }
    }
#endif
}

void CvsIgnoreList::addEntry(const QString& pattern)
{
    if(pattern != QString("!"))
    {
        if(pattern.isEmpty()) return;

        // The general match is general but slow.
        // Special tests for '*' and '?' at the beginning or end of a pattern
        // allow fast checks.

        // Count number of '*' and '?'
        unsigned int nofMetaCharacters = 0;

        const QChar* pos;
        pos = pattern.unicode();
        const QChar* posEnd;
        posEnd = pos + pattern.length();
        while(pos < posEnd)
        {
            if(*pos == QChar('*') || *pos == QChar('?')) ++nofMetaCharacters;
            ++pos;
        }

        if(nofMetaCharacters == 0)
        {
            m_exactPatterns.append(pattern);
        }
        else if(nofMetaCharacters == 1)
        {
            if(pattern.at(0) == QChar('*'))
            {
                m_endPatterns.append(pattern.right(pattern.length() - 1));
            }
            else if(pattern.at(pattern.length() - 1) == QChar('*'))
            {
                m_startPatterns.append(pattern.left(pattern.length() - 1));
            }
            else
            {
                m_generalPatterns.append(pattern);
            }
        }
        else
        {
            m_generalPatterns.append(pattern);
        }
    }
    else
    {
        m_exactPatterns.clear();
        m_startPatterns.clear();
        m_endPatterns.clear();
        m_generalPatterns.clear();
    }
}

bool CvsIgnoreList::matches(const QString& text, bool bCaseSensitive) const
{
    QRegExp regexp(text, bCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
    if(m_exactPatterns.indexOf(regexp) >= 0)
    {
        return true;
    }

    for(const QString& startPattern : m_startPatterns)
    {
        if(text.startsWith(startPattern, bCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive))
        {
            return true;
        }
    }

    for(const QString& endPattern : m_endPatterns)
    {
        if(text.endsWith(endPattern, bCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive))
        {
            return true;
        }
    }

    for(const QString& globStr : m_generalPatterns)
    {
        QRegExp pattern(globStr, bCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::Wildcard);
        if(pattern.exactMatch(text))
            return true;
    }

    return false;
}

bool CvsIgnoreList::cvsIgnoreExists(const t_DirectoryList* pDirList)
{
    for(const FileAccess& dir : *pDirList)
    {
        if(dir.fileName() == ".cvsignore")
            return true;
    }
    return false;
}
