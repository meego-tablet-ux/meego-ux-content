/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7

Rectangle {
    id: searchItem

    color: "white"

    width: parent.width
    height: outer.height

    property alias itemTitle: titleText.text
    property alias itemContent: contentText.text
    property alias itemImage: thumbnail.source
    property alias itemTimestamp: timeText.text
    property string itemUniqueID: ""

    signal clicked(string uniqueid)

    MouseArea {
        anchors.fill: parent
        onClicked: searchItem.clicked(itemUniqueID)
    }

    Column {
        id: outer

        width: parent.width

        Rectangle {
            width: parent.width
            height: 2
            color: "black"
        }

        Row {
            id: row

            spacing: 5

            Image {
                id: thumbnail

                height: 120
                width: 120
            }

            Column {
                spacing: 10

                Item {  // spacer
                    width: 1
                    height: 1
                }

                Text {
                    id: titleText

                    anchors.topMargin: 15

                    font.pointSize: 18
                    color: "#0000c0"
                }

                Text {
                    id: contentText

                    anchors.margins: 5

                    font.pointSize: 14
                    color: "black"
                }
            }
        }

        Text {
            id: timeText

            font.pointSize: 14
            color: "black"
        }
    }
}
