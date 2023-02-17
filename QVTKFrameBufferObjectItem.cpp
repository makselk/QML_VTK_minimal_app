#include "QVTKFrameBufferObjectItem.h"
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLState.h>


QVTKFrameBufferObjectItem::QVTKFrameBufferObjectItem() {
    qDebug("FBOItem constructor");
    this->setMirrorVertically(true);
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

    // Просто движения сферы (пример)
    static int r_color = 0;
    ++r_color;
    static double r = 0.5;
    r += (r_color % 200) > 100 ? -0.005 : 0.005;
    actor->GetProperty()->SetColor(r,0,0);
    actor->SetPosition(r * 10 - 7.5, 0, 0);

    // Публикация
    m_vtkRenderWindow->Render();
    // Возвращаются исходные параметры
    m_fbo_item->window()->resetOpenGLState();

    this->update();
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
