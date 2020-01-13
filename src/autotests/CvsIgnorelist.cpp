// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * Copyright (C) 2019 Michael Reeves <reeves.87@gmail.com>
 *
 * This file is part of KDiff3.
 *
 * KDiff3 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * KDiff3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KDiff3.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QTest>
#include <qglobal.h>

#include "../cvsignorelist.h"

class CvsIgnoreListTest : public QObject
{
    const QString defaultPatterns = QString::fromLatin1(". .. core RCSLOG tags TAGS RCS SCCS .make.state "
                                                        ".nse_depinfo #* .#* cvslog.* ,* CVS CVS.adm .del-* *.a *.olb *.o *.obj "
                                                        "*.so *.Z *~ *.old *.elc *.ln *.bak *.BAK *.orig *.rej *.exe _$* *$");
    Q_OBJECT
  private Q_SLOTS:
    void init()
    {
        CvsIgnoreList test;
        //sanity check defaults
        QVERIFY(test.m_exactPatterns.isEmpty());
        QVERIFY(test.m_endPatterns.isEmpty());
        QVERIFY(test.m_generalPatterns.isEmpty());
        QVERIFY(test.m_startPatterns.isEmpty());
    }

    void addEntriesFromString()
    {
        CvsIgnoreList test;

        QString testString = ". .. core RCSLOG tags TAGS RCS SCCS .make.state";
        test.addEntriesFromString(testString);
        QVERIFY(!test.m_exactPatterns.isEmpty());
        QVERIFY(test.m_exactPatterns == testString.split(' '));
    }

    void matches()
    {
        CvsIgnoreList test;

        QString testString = ". .. core RCSLOG tags TAGS RCS SCCS .make.state *.so _$*";
        test.addEntriesFromString(testString);

        QVERIFY(test.matches(".", false));
        QVERIFY(!test.matches("cores core", true));
        QVERIFY(test.matches("core", true));
        QVERIFY(!test.matches("Core", true));
        QVERIFY(test.matches("Core", false));
        QVERIFY(!test.matches("a", false));
        QVERIFY(test.matches("core", false));
        //ends with .so
        QVERIFY(test.matches("sdf3.so", true));
        QVERIFY(!test.matches("sdf3.to", true));
        QVERIFY(test.matches("*.so", true));
        QVERIFY(test.matches("sdf4.So", false));
        QVERIFY(!test.matches("sdf4.So", true));
        //starts with _$ddsf
        QVERIFY(test.matches("_$ddsf", true));
        //Should only match exact strings not partial ones
        QVERIFY(!test.matches("sdf4.so ", true));
        QVERIFY(!test.matches(" _$ddsf", true));

        testString = "*.*";
        test = CvsIgnoreList();
        test.addEntriesFromString("*.*");

        QVERIFY(test.matches("k.K", false));
        QVERIFY(test.matches("*.K", false));
        QVERIFY(test.matches("*.*", false));
        QVERIFY(!test.matches("*+*", false));
        QVERIFY(!test.matches("asd", false));
        //The fallowing are matched by the above
        QVERIFY(test.matches("a k.k", false));
        QVERIFY(test.matches("k.k v", false));
        QVERIFY(test.matches(" k.k", false));
        QVERIFY(test.matches("k.k ", false));
    }

    void testDefaults()
    {
        CvsIgnoreList test;
        CvsIgnoreList expected;
        MocIgnoreFile file;
        t_DirectoryList dirList;

        /*
            Verify default init. For this to work we must:
                1. Unset CVSIGNORE
                2. Insure no patterns are read from a .cvsignore file.
            MocCvsIgnore emulates a blank cvs file by default insuring the second condition.
        */
        test = CvsIgnoreList();
        //
        qunsetenv("CVSIGNORE");

        expected.addEntriesFromString(defaultPatterns);

        test.init(file, &dirList);
        QVERIFY(test.m_endPatterns == expected.m_endPatterns);
        QVERIFY(test.m_exactPatterns == expected.m_exactPatterns);
        QVERIFY(test.m_startPatterns == expected.m_startPatterns);
        QVERIFY(test.m_generalPatterns == expected.m_generalPatterns);
    }
};

QTEST_MAIN(CvsIgnoreListTest);

#include "CvsIgnorelist.moc"
