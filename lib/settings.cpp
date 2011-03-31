/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "memoryleak.h"
#include "settings.h"
#include "memoryleak-defines.h"
//
// Overview of McaSettings
//    - a collection of strings constants to use in QSettings calls
//    - "application" strings define filenames that are used
//    - all groups are currently free strings generated elsewhere
//    - "key" strings define keynames used in the settings key/value pairs

const char McaSettings::Organization[] = "MeeGo";

const char McaSettings::ApplicationCore[]  = "meego-ux-content";
const char McaSettings::KeyPluginPath[]    = "pluginPath";
const char McaSettings::KeyServiceName[]   = "serviceName";
const char McaSettings::KeyEnabledPrefix[] = "enabled";

const char McaSettings::ApplicationHide[] = "meego-ux-content-hide";
const char McaSettings::KeyLastDate[]     = "lastDate";
const char McaSettings::KeyHiddenPrefix[] = "hidden";
const char McaSettings::KeyEarliestTime[] = "earliestTime";
