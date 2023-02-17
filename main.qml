import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import VTK 8.2

Window {
    id: root
    property bool utv: false
    width: 800
    height: 600
    color: utv ? "lightblue" : "pink"

    Rectangle {
        id : vtkRenderWindowContainer
        anchors {
            left: parent.left
            right: parent.horizontalCenter
            top: parent.top
            bottom: parent.bottom
            margins: 20
        }

        VtkFboItem {
            id: vtk_fbo_item
            objectName: "vtkFboItem"
            visible: true
            anchors.fill: parent
            //property bool flipAngle: mouseArea.containsMouse ? 0 : 1
            //Behavior on flipAngle {}
            //MouseArea {
            //    id: mouseArea
            //    anchors.fill: parent
            //    hoverEnabled: true
            //}
        }
    }

    //Rectangle {
    //    id : vtkRenderWindowContainer1
    //    anchors {
    //        left: parent.horizontalCenter
    //        right: parent.right
    //        top: parent.top
    //        bottom: parent.bottom
    //        margins: 20
    //    }

    //    VtkFboItem {
    //        id: vtk_fbo_item1
    //        objectName: "vtkFboItem"
    //        visible: true
    //        anchors.fill: parent
    //    }
    //}

    Button {
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
        onClicked: {
            utv = !utv
            console.log(utv)
        }
    }


}
