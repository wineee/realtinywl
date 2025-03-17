import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: root

    property color color1: "#0b135b"
    property color color2: "#74478a"

    Item {
        anchors.fill: parent

        Image {
            anchors.fill: parent
            source: "file:///home/rewine/pic/desktop.png" // TODO: use config
        }

        Pane {
            id: pane
            anchors.centerIn: parent
            width: 400
            height: 360

            ColumnLayout {
                anchors {
                    centerIn: parent
                    verticalCenterOffset: 30
                }

                Image {
                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: 128
                    Layout.preferredHeight: 128
                    source: "file:///home/rewine/pic/test.png" //Backend.iconsSrc + "/" + Backend.user
                }

                Label {
                    Layout.alignment: Qt.AlignCenter
                    font.bold: true
                    font.pointSize: 13
                    font.capitalization: Font.AllUppercase
                    text: "testuser"//Backend.user
                    opacity: 0.85
                    color: root.color1
                }

                TextField {
                    id: passwordField
                    Layout.alignment: Qt.AlignCenter
                    Layout.topMargin: 20
                    Layout.preferredWidth: 180
                    Layout.maximumHeight: 40
                    echoMode: TextInput.Password
                    horizontalAlignment: TextInput.AlignHCenter
                    color: root.color2
                    Component.onCompleted: forceActiveFocus()
                    onAccepted: {
                        if (text) {
                            enabled = false;
                            Greetd.authenticate("testuser", passwordField.text);
                        }
                    }

                    function wrongPassword() {
                        selectAll();
                        wrongPasswordAnimation.start();
                    }

                    transform: Translate {
                        id: passwordFieldTranslate
                        x: 0
                    }

                    SequentialAnimation {
                        id: wrongPasswordAnimation
                        loops: 3
                        onFinished: passwordField.enabled = true

                        PropertyAnimation {
                            target: passwordFieldTranslate
                            property: "x"
                            from: 0
                            to: 10
                            duration: 25
                            easing.type: Easing.InQuad
                        }

                        PropertyAnimation {
                            target: passwordFieldTranslate
                            property: "x"
                            from: 10
                            to: -10
                            duration: 50
                            easing.type: Easing.OutInQuad
                        }

                        PropertyAnimation {
                            target: passwordFieldTranslate
                            property: "x"
                            from: -10
                            to: 0
                            duration: 25
                            easing.type: Easing.OutQuad
                        }
                    }
                }
            }

            Label {
                id: timeLabel
                anchors {
                    top: parent.top
                    right: parent.right
                    topMargin: -5
                }
                font.bold: true
                font.pointSize: 11
                opacity: 0.8
                color: root.color1
                Component.onCompleted: update()

                function update() {
                    text = Qt.formatTime(new Date(), "hh:mm");
                }

                Timer {
                    interval: 1000
                    repeat: true
                    running: true
                    onTriggered: timeLabel.update()
                }
            }

            Label {
                id: dateLabel
                anchors {
                    top: parent.top
                    left: parent.left
                    topMargin: -5
                }
                font.bold: true
                font.pointSize: 11
                opacity: 0.8
                color: root.color1
                text: Qt.formatDate(new Date(), "ddd, MMM dd");
            }

            ParallelAnimation {
                id: hideAnimation
                onFinished: Qt.quit()

                PropertyAnimation {
                    target: pane
                    property: "scale"
                    from: 1.0
                    to: 0.7
                    duration: 150
                    easing.type: Easing.InOutQuad
                }

                PropertyAnimation {
                    target: pane
                    property: "opacity"
                    from: 1.0
                    to: 0.0
                    duration: 150
                    easing.type: Easing.InOutQuad
                }

                PauseAnimation {
                    duration: 300
                }
            }
        }

        // Connections {
        //     target: Backend

        //     function onSessionSuccess() {
        //         hideAnimation.start();
        //     }

        //     function onSessionError(type, description) {
        //         if (type == "auth_error") {
        //             passwordField.wrongPassword();
        //         } else {
        //             console.log(type, description);
        //         }
        //     }

        //     function onInfoMessage(message) {
        //         console.log(message);
        //     }

        //     function onErrorMessage(message) {
        //         console.log(message);
        //     }
        // }
    }
}
