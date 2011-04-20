import Qt 4.7
import MeeGo.Labs.Components 0.1

Rectangle {
    id: base

    width: parent.width
    height: childrenRect.height

    property alias serviceName: serviceName.text
    property alias serviceEnabled: serviceToggle.on

    signal toggled(bool isOn)

    Column {
        width: parent.width
        height: childrenRect.height
        spacing: 10

        Rectangle {
            color: "lightgray"
            width: parent.width
            height: 1
        }

        Item {
            width: parent.width
            height: childrenRect.height
            Text {
                id: serviceName
                font.pointSize: 18
                anchors.left: parent.left
                anchors.margins: 10
            }

            ToggleButton {
                id: serviceToggle
                anchors.right: parent.right
                anchors.margins: 10
                onToggled: {
                    base.toggled(isOn)
                }
            }
        }

        Rectangle {
            color: "black"
            width: parent.width
            height: 1
        }
    }
}
