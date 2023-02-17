#ifndef QVTKFrameBufferObjectItem_h_
#define QVTKFrameBufferObjectItem_h_

#include <QtQuick/QQuickFramebufferObject>

// Use the OpenGL API abstraction from Qt instead of from VTK because vtkgl.h
// and other Qt OpenGL-related headers do not play nice when included in the
// same compilation unit
#include <QOpenGLFunctions>
#include <QQuickWindow>
#include <QQuickFramebufferObject>
#include <QOpenGLFramebufferObject>

#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>

#include <vtkGenericRenderWindowInteractor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkObjectFactory.h>
#include <QVTKOpenGLNativeWidget.h>

#include <vtkRendererCollection.h>
#include <vtkCamera.h>


class QVTKFrameBufferObjectRenderer;


// Непосредственно сам объект, который видит QML
class QVTKFrameBufferObjectItem : public QQuickFramebufferObject {
    Q_OBJECT
public:
    QVTKFrameBufferObjectItem();
    ~QVTKFrameBufferObjectItem();

    // После создания объекта, qml просит создать рендерер,
    // который будет предоставлять OpenGL инофрмацию
    Renderer *createRenderer() const override;

    // Проверка наличия инициализированного поля рендерера
    bool isInitialized() const;
    // Инициализирует поле рендерера
    void setFboRenderer(QVTKFrameBufferObjectRenderer* renderer);

protected:
    QVTKFrameBufferObjectRenderer* m_fbo_renderer = nullptr;
};



// Объект, в котором происходит вся магия передачи изображения
// от VTK к QML через OpenGLFrameBufferObject
class QVTKFrameBufferObjectRenderer :
        public QObject,
        public QQuickFramebufferObject::Renderer,
        protected QOpenGLFunctions {

public:
    QVTKFrameBufferObjectRenderer();
    ~QVTKFrameBufferObjectRenderer();

    // После инициализации рендерера вызывается функция синхронизации
    // для инициализации оставшихся данных (из-за того, что
    // QVTKFrameBufferObjectItem::createRenderer - const, внутри него
    // нельзя определить нестатичные поля)
    // Также вызывается для синхронизации данных рендерера и item'a
    // в случае изменения параметров последнего (например, размера)
    virtual void synchronize(QQuickFramebufferObject * item) override;

    // Создание OpenGlFrameBufferObject'a, через который происходит передача данных
    // Также вызывается при изменении item'a
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

    // Публикация данных (в данном случае vtk) в OpenGlFrameBufferObject
    virtual void render() override;

    // Здесь задаются параметры, необходимые vtk
    // для публикации в OpenGlFrameBufferObject
    virtual void openGLInitState();

    // Первоначальная инициализация сцены
    // (Пример чисто для проверки работоспобности)
    void initScene();

private:
    // Ссылка на связанный qml объект
    QVTKFrameBufferObjectItem* m_fbo_item = nullptr;

    // vtk stuff
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_vtkRenderWindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkGenericRenderWindowInteractor> m_interactor;
    vtkSmartPointer<vtkActor> actor;
    double angle = 0.0;
    bool m_first_render = true;
};

#endif
