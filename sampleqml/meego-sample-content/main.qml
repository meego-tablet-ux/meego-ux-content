import Qt 4.7
import MeeGo.Components 0.1
import MeeGo.Content 1.0

Window {
    id: window
    toolBarTitle: "Feed Sample"

    resources: [
        McaPanelManager {
            id: panelManager
            categories: ["social", "im", "email"]
            servicesEnabledByDefault: true
        }
    ]

    Component.onCompleted: {
        switchBook(feedPageComponent)

        panelManager.initialize("feed_sample")

        // For some rason this doesn't get properly set
        console.log("Bug on McaPanelManager frozen property")
        panelManager.frozen = true
        panelManager.frozen = false
    }

    Component {
        id: feedPageComponent
        AppPage {
            id: feedPage            
            pageTitle: "Content Aggregator Sample"

            Item {
                id: buttons

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 10

                height: childrenRect.height + 10

                Row {
                    width: parent.width
                    height: childrenRect.height

                    spacing: 5
                    
                    Button {
                        id: freezeButton
                        text: panelManager.frozen?"Freeze":"Thaw"

                        onClicked: {
                            panelManager.frozen = !panelManager.frozen
                        }
                    }

                    Button {
                        id: modeButton
                        text: modeDefault?"Action: default":"Action: hide"
                        property bool modeDefault: true
                        
                        onClicked: {
                            modeDefault = !modeDefault
                        }
                    }

                    Button {
                        id: settingsButton
                        text: "Settings"
                        onClicked: {
                            window.addPage(settingsPageComponent)
                        }
                    }

                    Button {
                        id: searchButton
                        text: "Search"
                        onClicked: {
                            window.addPage(searchPageComponent)
                        }
                    }
                }
            }
            ListView {
                anchors.top: buttons.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                model: panelManager.feedModel
                delegate: panelDelegate
                clip: true
            }

            resources: [
                Component {
                    id: panelDelegate

                    SampleItem {
                        id: sampleitem
                        titleText: title
                        contentText: content
                        serviceName: servicename
                        itemID: id

                        onClicked: {
                            if (modeButton.modeDefault) 
                                actions.performStandardAction("default", itemID)
                            else
                                actions.performStandardAction("hide", itemID)
                        }
                    }
                }
            ]
        }
    }

    Component {
        id: searchPageComponent
        AppPage {
            id: searchPage
            pageTitle: "Content Aggregator Sample Search"

            Item {
                id: searchEntry
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                // There's something strange about TextEntry
                // the + 20 is to work around the problem
                height: childrenRect.height + 20

                TextEntry {
                    id: searchEntryText
                    anchors.left: parent.left
                    anchors.right: sbutton1.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.margins: 10
                }

                Button {
                    id: sbutton1
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: 10
                    text: "Search"
                    onClicked: {
                        searchManager.setSearchText(searchEntryText.text)
                    }
                }
            }

            ListView {
                anchors.top: searchEntry.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                model: searchManager.feedModel
                delegate: searchDelegate
                clip: true
            }

            resources: [
                Component {
                    id: searchDelegate

                    SampleItem {
                        id: sampleitem
                        titleText: title
                        contentText: content
                        serviceName: servicename
                        itemID: id

                        onClicked: {
                            actions.performStandardAction("default", itemID)
                        }
                    }
                },
                McaSearchManager {
                        id: searchManager
                }
            ]

            Component.onCompleted: {
                searchManager.initialize("feed_sample")
            }
        }
    }

    Component {
        id: settingsPageComponent
        AppPage {
            id: settingsPage
            pageTitle: "Content Aggregator Sample Settings"

            Item {
                id: col

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 10
                height: childrenRect.height + 10

                Text {
                    id: stitle
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Feeds Settings"
                    font.pointSize: 20
                }
            }

            ListView {
                anchors.top: col.bottom
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right

                model: panelManager.serviceModel
                delegate: serviceDelegate
                clip: true
            }

            resources: [
                Component {
                    id: serviceDelegate
                    ServiceItem {
                        serviceName: displayname
                        serviceEnabled: panelManager.isServiceEnabled(upid)

                        onToggled: {
                            panelManager.setServiceEnabled(upid, isOn)
                        }
                    }
                }
            ]
        }
    }
}

