#include "QVTKFrameBufferObjectItem.h"
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLState.h>
#include <vtkCommand.h>
#include <vtkInteractorStyleTrackballCamera.h>


QVTKFrameBufferObjectItem::QVTKFrameBufferObjectItem() {
    qDebug("FBOItem constructor");
    this->setMirrorVertically(true);
    this->setAcceptedMouseButtons(Qt::RightButton | Qt::LeftButton | Qt::MidButton);
}

QVTKFrameBufferObjectItem::~QVTKFrameBufferObjectItem() {
    qDebug("FBOItem delete");
}

QQuickFramebufferObject::Renderer* QVTKFrameBufferObjectItem::createRenderer() const {
    qDebug("create FBO renderer");
    return new QVTKFrameBufferObjectRenderer();
}

bool QVTKFrameBufferObjectItem::isInitialized() const {
    return (m_fbo_renderer != nullptr);
}

void QVTKFrameBufferObjectItem::setFboRenderer(QVTKFrameBufferObjectRenderer *renderer) {
    m_fbo_renderer = renderer;
}

void QVTKFrameBufferObjectItem::wheelEvent(QWheelEvent *e) {
    m_fbo_renderer->setWheelEvent(e);
    e->accept();
    this->update();
}

void QVTKFrameBufferObjectItem::mouseMoveEvent(QMouseEvent *e) {
    m_fbo_renderer->setMouseMoveEvent(e);
    e->accept();
    this->update();
}

void QVTKFrameBufferObjectItem::mousePressEvent(QMouseEvent *e) {
    if(e->button() == Qt::LeftButton)
        m_fbo_renderer->setMousePressEventL(e);

    else if(e->button() == Qt::RightButton)
        m_fbo_renderer->setMousePressEventR(e);

    else if(e->button() == Qt::MidButton)
        m_fbo_renderer->setMousePressEventW(e);

    e->accept();
    this->update();
}

void QVTKFrameBufferObjectItem::mouseReleaseEvent(QMouseEvent *e) {
    if(e->button() == Qt::LeftButton)
        m_fbo_renderer->setMouseReleaseEventL(e);

    else if (e->button() == Qt::RightButton)
        m_fbo_renderer->setMouseReleaseEventR(e);

    else if(e->button() == Qt::MidButton)
        m_fbo_renderer->setMouseReleaseEventW(e);

    e->accept();
    this->update();
}




QVTKFrameBufferObjectRenderer::QVTKFrameBufferObjectRenderer() {
    qDebug("FBO Renderer constructor");
    // Инициализация функций OpenGL
    QOpenGLFunctions::initializeOpenGLFunctions();
    QOpenGLFunctions::glUseProgram(0);

    // Renderer
    m_vtkRenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_vtkRenderWindow->AddRenderer(m_renderer);

    // Interactor
    m_interactor = vtkSmartPointer<vtkGenericRenderWindowInteractor>::New();
    vtkNew<vtkInteractorStyleTrackballCamera> styl;
    m_interactor->SetInteractorStyle(styl);
    m_interactor->EnableRenderOff();
    m_vtkRenderWindow->SetInteractor(m_interactor);

    // Initialize the OpenGL context for the renderer
    m_vtkRenderWindow->OpenGLInitContext();

    this->update();
}

QVTKFrameBufferObjectRenderer::~QVTKFrameBufferObjectRenderer() {
    m_fbo_item = nullptr;
}

void QVTKFrameBufferObjectRenderer::synchronize(QQuickFramebufferObject * item) {
    qDebug("sync");
    // При первом вызове инициализирует указаель на
    // qml объект и дает ему ссылку на себя
    if(!m_fbo_item) {
        m_fbo_item = static_cast<QVTKFrameBufferObjectItem*>(item);
        m_fbo_item->setFboRenderer(this);
    }
}

// Called from the render thread when the GUI thread is NOT blocked
void QVTKFrameBufferObjectRenderer::render() {
    qDebug("render");
    // Подотовка к публикации
    this->openGLInitState();
    //m_vtkRenderWindow->Start();

    if(m_first_render) {
        this->initScene();
        m_first_render = false;
    }

    // Обработка событий
    this->handleEvents();

    // Публикация
    m_vtkRenderWindow->Render();
    // Возвращаются исходные параметры
    m_fbo_item->window()->resetOpenGLState();

    //this->update();
}

void QVTKFrameBufferObjectRenderer::openGLInitState() {
    qDebug("OpenGLInitState");
    // Иницализация параметров для публикации
    m_vtkRenderWindow->OpenGLInitState();
    // Делаем поток текщим
    m_vtkRenderWindow->MakeCurrent();
}

QOpenGLFramebufferObject* QVTKFrameBufferObjectRenderer::createFramebufferObject(const QSize &size) {
    qDebug("createFramebufferObject");

    // Создаем OpenGLFrameBufferObject
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::Depth);
    std::unique_ptr<QOpenGLFramebufferObject> framebufferObject(new QOpenGLFramebufferObject(size, format));

    //// По идее тут должна быть инициализация параметров vtkRenderWindow под буффер,
    //// Но они они инициализируются автоматически (по крайней мере GL_COLOR_ATTACHMENT0)
    //m_vtkRenderWindow->SetBackLeftBuffer(GL_COLOR_ATTACHMENT0);
    //m_vtkRenderWindow->SetFrontLeftBuffer(GL_COLOR_ATTACHMENT0);
    //m_vtkRenderWindow->SetBackBuffer(GL_COLOR_ATTACHMENT0);
    //m_vtkRenderWindow->SetFrontBuffer(GL_COLOR_ATTACHMENT0);

    // Задаем размер, равный буфферу
    m_vtkRenderWindow->SetSize(framebufferObject->size().width(), framebufferObject->size().height());
    // Разрешаем рендеринг вне окна
    m_vtkRenderWindow->SetOffScreenRendering(true);
    m_vtkRenderWindow->Modified();

    return framebufferObject.release();
}

void QVTKFrameBufferObjectRenderer::setWheelEvent(QWheelEvent* e) {
    m_mouse_wheel = std::make_shared<QWheelEvent>(*e);
    m_mouse_wheel->ignore();
}

void QVTKFrameBufferObjectRenderer::setMouseMoveEvent(QMouseEvent* e) {
    m_mouse_move = std::make_shared<QMouseEvent>(*e);
    m_mouse_move->ignore();
}

void QVTKFrameBufferObjectRenderer::setMousePressEventW(QMouseEvent* e) {
    m_mouse_press_w = std::make_shared<QMouseEvent>(*e);
    m_mouse_press_w->ignore();
}

void QVTKFrameBufferObjectRenderer::setMousePressEventL(QMouseEvent* e) {
    m_mouse_press_l = std::make_shared<QMouseEvent>(*e);
    m_mouse_press_l->ignore();
}

void QVTKFrameBufferObjectRenderer::setMousePressEventR(QMouseEvent* e) {
    m_mouse_press_r = std::make_shared<QMouseEvent>(*e);
    m_mouse_press_r->ignore();
}

void QVTKFrameBufferObjectRenderer::setMouseReleaseEventW(QMouseEvent* e) {
    m_mouse_release_w = std::make_shared<QMouseEvent>(*e);
    m_mouse_release_w->ignore();
}

void QVTKFrameBufferObjectRenderer::setMouseReleaseEventL(QMouseEvent* e) {
    m_mouse_release_l = std::make_shared<QMouseEvent>(*e);
    m_mouse_release_l->ignore();
}

void QVTKFrameBufferObjectRenderer::setMouseReleaseEventR(QMouseEvent* e) {
    m_mouse_release_r = std::make_shared<QMouseEvent>(*e);
    m_mouse_release_r->ignore();
}

void QVTKFrameBufferObjectRenderer::handleEvents() {
    // Колесо мыши прокурчено
    if(m_mouse_wheel && !m_mouse_wheel->isAccepted()) {
        if(m_mouse_wheel->angleDelta().y() > 0)
            m_interactor->InvokeEvent(vtkCommand::MouseWheelForwardEvent);
        else
            m_interactor->InvokeEvent(vtkCommand::MouseWheelBackwardEvent);
        m_mouse_wheel->accept();
    }

    // Колесо мыши нажато
    if(m_mouse_press_w && !m_mouse_press_w->isAccepted()) {
        m_interactor->SetEventInformationFlipY(m_mouse_press_w->x(), m_mouse_press_w->y());
        m_interactor->InvokeEvent(vtkCommand::MiddleButtonPressEvent, m_mouse_press_w.get());
        m_mouse_press_w->accept();
    }

    // ЛКМ нажата
    if(m_mouse_press_l && !m_mouse_press_l->isAccepted()) {
        m_interactor->SetEventInformationFlipY(m_mouse_press_l->x(), m_mouse_press_l->y());
        m_interactor->InvokeEvent(vtkCommand::LeftButtonPressEvent, m_mouse_press_l.get());
        m_mouse_press_l->accept();
    }

    // ПКМ нажата
    if(m_mouse_press_r && !m_mouse_press_r->isAccepted()) {
        m_interactor->SetEventInformationFlipY(m_mouse_press_r->x(), m_mouse_press_r->y());
        m_interactor->InvokeEvent(vtkCommand::RightButtonPressEvent, m_mouse_press_r.get());
        m_mouse_press_r->accept();
    }

    // Движение мышью
    if(m_mouse_move && !m_mouse_move->isAccepted()) {
        m_interactor->SetEventInformationFlipY(m_mouse_move->x(), m_mouse_move->y());
        m_interactor->InvokeEvent(vtkCommand::MouseMoveEvent, m_mouse_move.get());
        m_mouse_move->accept();
    }

    // Колесо мыши отжато
    if(m_mouse_release_w && !m_mouse_release_w->isAccepted()) {
        m_interactor->SetEventInformationFlipY(m_mouse_release_w->x(), m_mouse_release_w->y());
        m_interactor->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, m_mouse_release_w.get());
        m_mouse_release_w->accept();
    }

    // ЛКМ отжата
    if(m_mouse_release_l && !m_mouse_release_l->isAccepted()) {
        m_interactor->SetEventInformationFlipY(m_mouse_release_l->x(), m_mouse_release_l->y());
        m_interactor->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, m_mouse_release_l.get());
        m_mouse_release_l->accept();
    }

    // ПКМ отжата
    if(m_mouse_release_r && !m_mouse_release_r->isAccepted()) {
        m_interactor->SetEventInformationFlipY(m_mouse_release_r->x(), m_mouse_release_r->y());
        m_interactor->InvokeEvent(vtkCommand::RightButtonReleaseEvent, m_mouse_release_r.get());
        m_mouse_release_r->accept();
    }
}


void QVTKFrameBufferObjectRenderer::initScene() {
    qDebug("init scene");

    m_vtkRenderWindow->SetOffScreenRendering(true);

    m_renderer->SetBackground(0.5, 0.5, 0.7);
    m_renderer->SetBackground2(0.7, 0.5, 0.5);
    m_renderer->GradientBackgroundOn();

    vtkNew<vtkSphereSource> sphere_source;
    sphere_source->SetCenter(0,0,0);
    sphere_source->SetRadius(13);
    sphere_source->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(sphere_source->GetOutput());

    actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1,0,0);

    m_renderer->AddViewProp(actor);
}
