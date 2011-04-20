import Qt 4.7
import MeeGo.Labs.Components 0.1
import MeeGo.Content 1.0

Window {
    id: scene
    title: qsTr("Feed Sample")
    applicationPage: feedPageComponent

    resources: [
        McaPanelManager {
            id: panelManager
            categories: ["social", "im", "email"]
            servicesEnabledByDefault: true
        }
    ]

    Component.onCompleted: {
        panelManager.initialize("feed_sample")

        // For some rason this doesn't get properly set
        console.log("Bug on McaPanelManager frozen property")
        panelManager.frozen = true
        panelManager.frozen = false
    }

    Component {
        id: feedPageComponent
        ApplicationPage {
            id: feedPage
            
            title: qsTr("Content Aggregator Sample")
            Item {
                id: buttons
                anchors.top: parent.content.top
                anchors.left: parent.content.left
                anchors.right: parent.content.right
                anchors.margins: 10

                height: childrenRect.height + 10

                Row {
                    width: parent.width
                    height: childrenRect.height

                    spacing: 5
                    
                    Button {
                        id: freezeButton
                        title: panelManager.frozen?"Freeze":"Thaw"
                        height: parent.height

                        onClicked: {
                            panelManager.frozen = !panelManager.frozen
                        }
                    }

                    Button {
                        id: modeButton
                        title: modeDefault?"Action: default":"Action: hide"
                        height: parent.height
                        property bool modeDefault: true
                        
                        onClicked: {
                            modeDefault = !modeDefault
                        }
                    }

                    Button {
                        id: settingsButton
                        title: "Settings"
                        height: parent.height
                        onClicked: {
                            scene.addApplicationPage(settingsPageComponent)
                        }
                    }

                    Button {
                        id: searchButton
                        title: "Search"
                        height: parent.height
                        onClicked: {
                            scene.addApplicationPage(searchPageComponent)
                        }
                    }
                }
            }
            ListView {
                anchors.top: buttons.bottom
                anchors.left: parent.content.left
                anchors.right: parent.content.right
                anchors.bottom: parent.content.bottom

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
        ApplicationPage {
            id: searchPage
            title: qsTr("Content Aggregator Sample Search")


            Item {
                id: searchEntry

                anchors.top: parent.content.top
                anchors.left: parent.content.left
                anchors.right: parent.content.right

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
                    title: "Search"
                    onClicked: {
                        searchManager.setSearchText(searchEntryText.text)
                    }
                }
            }

            ListView {
                anchors.top: searchEntry.bottom
                anchors.left: parent.content.left
                anchors.right: parent.content.right
                anchors.bottom: parent.content.bottom

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
        ApplicationPage {
            id: settingsPage
            title: qsTr("Content Aggregator Sample Settings")

            Item {
                id: col

                anchors.top: parent.content.top
                anchors.left: parent.content.left
                anchors.right: parent.content.right
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
                anchors.bottom: parent.content.bottom
                anchors.left: parent.content.left
                anchors.right: parent.content.right

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

