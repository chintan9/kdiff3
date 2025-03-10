// clang-format off
/**
 * KDiff3 - Text Diff And Merge Tool
 *
 * SPDX-FileCopyrightText: 2021 David Hallas <david@davidhallas.dk>
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
// clang-format on

#include "GitIgnoreList.h"

#include "Logging.h"
#include "fileaccess.h"

#include <algorithm>
#include <utility>

#include <QFile>
#include <QStringList>
#include <QTextStream>

namespace {

bool isComment(const QString& line)
{
    return line.startsWith(QChar(u'#'));
}

} // namespace

GitIgnoreList::GitIgnoreList() = default;

GitIgnoreList::~GitIgnoreList() = default;

void GitIgnoreList::enterDir(const QString& dir, const DirectoryList& directoryList)
{
    const auto directoryListIt = std::find_if(directoryList.begin(), directoryList.end(), [](const FileAccess& file) {
        return file.fileName() == ".gitignore";
    });
    if(directoryListIt != directoryList.end())
    {
        addEntries(dir, readFile(directoryListIt->absoluteFilePath()));
    }
}

bool GitIgnoreList::matches(const QString& dir, const QString& text, bool bCaseSensitive) const
{
    /*
        Unknown compiler/library bug is triggering a warning from valgring about an unitialized with no useful stack trace.
        Isolated to:
            for(auto& dirPattern: m_patterns).

            gcc14, clang18 tested. valgrind-3.24.0, distro arch linux.
    */
    for(auto& dirPattern: m_patterns)
    {
        if(!dir.startsWith(dirPattern.first))
        {
            continue;
        }
        for(QRegularExpression& pattern: dirPattern.second)
        {
            if(!bCaseSensitive)
            {
                pattern.setPatternOptions(QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
            }
            else
            {
                pattern.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);
            }
            const QRegularExpressionMatch match = pattern.match(text);
            if(match.hasMatch())
            {
                qCDebug(kdiffGitIgnoreList) << "Matched entry" << text;
                return true;
            }
        }
    }
    return false;
}

QString GitIgnoreList::readFile(const QString& fileName) const
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        return QString();
    }
    QTextStream stream(&file);
    return stream.readAll();
}

void GitIgnoreList::addEntries(const QString& dir, const QString& lines)
{
    static const QRegularExpression newLineReg = QRegularExpression("[\r\n]");
    const QStringList lineList = lines.split(newLineReg, Qt::SkipEmptyParts);
    for(const QString& line: lineList)
    {
        if(isComment(line))
        {
            continue;
        }
        QRegularExpression expression(QRegularExpression::wildcardToRegularExpression(line));
        if(!expression.isValid())
        {
            qCDebug(kdiffGitIgnoreList) << "Expression" << line << "is not valid - skipping ...";
            continue;
        }
        qCDebug(kdiffGitIgnoreList) << "Adding entry [" << dir << "]" << line;
        m_patterns[dir].push_back(expression);
    }
}
