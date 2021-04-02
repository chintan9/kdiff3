/*
 * KDiff3 - Text Diff And Merge Tool
 *
 * SPDX-FileCopyrightText: 2019-2020 Michael Reeves reeves.87@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Logging.h"

#ifdef NDEBUG
#define logLevel        QtWarningMsg

#else
#define logLevel         QtDebugMsg
#endif

Q_LOGGING_CATEGORY(kdiffMain, "org.kde.kdiff3", logLevel)
Q_LOGGING_CATEGORY(kdiffDiffTextWindow, "org.kde.kdiff3.kdifftextwindow", QtInfoMsg);
Q_LOGGING_CATEGORY(kdiffFileAccess, "org.kde.kdiff3.fileAccess", logLevel)
//kdiffCore is very noisey if debug is turned on and not really useful unless your making changes in the core data processing.
Q_LOGGING_CATEGORY(kdiffCore, "org.kde.kdiff3.core", QtWarningMsg)
Q_LOGGING_CATEGORY(kdiffGitIgnoreList, "org.kde.kdiff3.gitIgnoreList", logLevel)

#undef logLevel
