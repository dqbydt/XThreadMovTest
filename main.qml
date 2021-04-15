import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12

import "."

ApplicationWindow {
    id: appWnd
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
    color: "steelblue"


    Item {
        id: xtmui
        objectName: "xtmui"
        anchors.fill: parent

        focus: true

        signal sendIt
        signal quit

        Button {
            id: sendMov
            x: 20
            y: 20
            text: "Send Mov Object"
            font.pointSize: 15
            background: Rectangle {
                color: sendMov.pressed ? "white" : Colors.teal_gray
            }
            onClicked: xtmui.sendIt()
        }
    }

    // QQuickWindow::closing emitted when the QML window receives the close event
    // from the windowing system.
    // https://stackoverflow.com/a/33210490/3367247
    onClosing: xtmui.quit()
}
