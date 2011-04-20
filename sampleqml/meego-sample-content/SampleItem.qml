import Qt 4.7

Rectangle {
    id: itembase
    color: "white"
    width: parent.width
    height: outer.height

    property alias titleText: title.text 
    property alias contentText: content.text
    property alias serviceName: serviceName.text
    property alias itemImage: image.source

    property string itemID: ""

    signal clicked

    MouseArea {
        anchors.fill: parent
        onClicked: itembase.clicked()
    }

    Column {
        id: outer
        anchors.left: parent.left
        anchors.right: parent.right

        height: childrenRect.height

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right

            height: serviceName.height

            color: "lightgray"
            
            Text {
                id: serviceName
                font.pointSize: 18
            }
        }

        Row {
            spacing: 5

            Image {
                id: image
                width: 120
                height: 120

                Text {
                    z: -1
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Picture"
                }
            }

            Rectangle {
                width: 1
                height: parent.height
                color: "lightgray"
            }

            Column {
                Text {
                    id: title 
                    font.pointSize: 18
                    clip: true
                }

                Text {
                    id: content
                    font.pointSize: 14
                    clip: true
                }
                clip: true
            }

        }
    }
}
