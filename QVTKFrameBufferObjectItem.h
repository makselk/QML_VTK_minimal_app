#ifndef QVTKFrameBufferObjectItem_h_
#define QVTKFrameBufferObjectItem_h_

#include <QtQuick/QQuickFramebufferObject>
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
    Q_INVOKABLE
    void changeTrackball(const bool &);

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

public:
    void wheelEvent(QWheelEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;

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

public:
    // Методы для сохранения событий
    void setWheelEvent(QWheelEvent* e);
    void setMouseMoveEvent(QMouseEvent* e);
    void setMousePressEventW(QMouseEvent* e);
    void setMousePressEventL(QMouseEvent* e);
    void setMousePressEventR(QMouseEvent* e);
    void setMouseReleaseEventW(QMouseEvent* e);
    void setMouseReleaseEventL(QMouseEvent* e);
    void setMouseReleaseEventR(QMouseEvent* e);

    void setTrackball(bool state);

private:
    // Здесь задаются параметры, необходимые vtk
    // для публикации в OpenGlFrameBufferObject
    virtual void openGLInitState();

    // Первоначальная инициализация сцены
    // (Пример чисто для проверки работоспобности)
    void initScene();

    void handleEvents();

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

    // events
    std::shared_ptr<QWheelEvent> m_mouse_wheel;
    std::shared_ptr<QMouseEvent> m_mouse_move;
    std::shared_ptr<QMouseEvent> m_mouse_press_w;
    std::shared_ptr<QMouseEvent> m_mouse_press_l;
    std::shared_ptr<QMouseEvent> m_mouse_press_r;
    std::shared_ptr<QMouseEvent> m_mouse_release_w;
    std::shared_ptr<QMouseEvent> m_mouse_release_l;
    std::shared_ptr<QMouseEvent> m_mouse_release_r;
};

#endif
