/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Components 0.1
import MeeGo.Content 0.1

Window {
    id: window
    showToolBarSearch: false

    Component.onCompleted: {
        switchBook(searchPageComponent)
    }

    Component {
        id: searchPageComponent

        AppPage {
            id: searchPage
            pageTitle: "Search"

            resources: [
                McaSearchManager {
                    id: searchManager
                },

                Component {
                    id: resultsDelegate

                    SearchItem {
                        itemTitle: title
                        itemContent: content
                        itemImage: avatar ? avatar : ""
                        itemUniqueID: id
                        itemTimestamp: fuzzy.getFuzzy(timestamp)

                        onClicked: {
                            actions.performStandardAction("default", uniqueid)
                        }
                    }
                },

                FuzzyDateTime {
                    id: fuzzy
                }
            ]

            Component.onCompleted: {
                searchManager.initialize("");
            }

            Column {
                parent: searchPage.content
                anchors.fill: parent

                Rectangle {
                    id: editborder

                    color: "white"
                    anchors.top: parent.content.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 10
                    width: parent.width
                    height: edit.height + edit.anchors.margins * 2

                    TextEntry {
                        id: edit

                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: searchButton.left
                        anchors.margins: 5

                        focus: true

                        Keys.onReturnPressed: {
                            searchManager.searchText = edit.text
                        }
                    }

                    Button {
                        id: searchButton

                        anchors.right: parent.right

                        text: "Search"
                        property string searchText: ""

                        onClicked: {
                            searchManager.searchText = edit.text
                        }
                    }
                }

                ListView {
                    anchors.top: editborder.bottom
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 10

                    model: searchManager.feedModel
                    delegate: resultsDelegate

                    clip: true
                }
            }
        }
    }
}
