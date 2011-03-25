/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <MApplication>
#include <MApplicationWindow>

#include "panelpage.h"

int main(int argc, char *argv[])
{
    // ensure deterministic behavior for now
    qsrand(0);

    MApplication app(argc, argv);
    PanelPage *page = new PanelPage;

    MApplicationWindow window;
    window.show();

    page->appear();
    app.exec();

    return 0;
}
