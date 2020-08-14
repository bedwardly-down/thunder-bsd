#include "meshedit.h"
#include "ui_meshedit.h"

#include <QSettings>
#include <QMessageBox>

#include <engine.h>

#include <components/actor.h>
#include <components/transform.h>
#include <components/meshrender.h>
#include <components/directlight.h>
#include <components/camera.h>

#include <resources/prefab.h>

#include <global.h>

#include "editors/propertyedit/nextobject.h"
#include "controllers/objectctrl.h"
#include "graph/sceneview.h"

#include "assimpconverter.h"

MeshEdit::MeshEdit(Engine *engine) :
        QMainWindow(nullptr),
        IAssetEditor(engine),
        ui(new Ui::MeshEdit),
        m_pMesh(nullptr),
        m_pGround(nullptr),
        m_pDome(nullptr),
        m_pLight(nullptr),
        m_pSettings(nullptr),
        m_pConverter(nullptr) {

    ui->setupUi(this);

    glWidget    = new Viewport(this);
    CameraCtrl *ctrl    = new CameraCtrl(glWidget);
    ctrl->blockMovement(true);
    ctrl->setFree(false);
    ctrl->init(nullptr);
    glWidget->setController(ctrl);
    glWidget->setScene(Engine::objectCreate<Scene>("Scene"));
    glWidget->setObjectName("Preview");
    glWidget->setWindowTitle("Preview");

    ui->treeView->setWindowTitle("Properties");

    connect(glWidget, SIGNAL(inited()), this, SLOT(onGLInit()), Qt::DirectConnection);
    startTimer(16);

    ui->centralwidget->addToolWindow(glWidget, QToolWindowManager::EmptySpaceArea);
    ui->centralwidget->addToolWindow(ui->treeView, QToolWindowManager::ReferenceLeftOf, ui->centralwidget->areaFor(glWidget));

    foreach(QWidget *it, ui->centralwidget->toolWindows()) {
        QAction *action = ui->menuWindow->addAction(it->windowTitle());
        action->setObjectName(it->windowTitle());
        action->setData(QVariant::fromValue(it));
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onToolWindowActionToggled(bool)));
    }
    connect(ui->centralwidget, SIGNAL(toolWindowVisibilityChanged(QWidget *, bool)), this, SLOT(onToolWindowVisibilityChanged(QWidget *, bool)));

    readSettings();
}

MeshEdit::~MeshEdit() {
    writeSettings();

    delete ui;

    delete glWidget;
    delete m_pMesh;
    delete m_pGround;
    delete m_pLight;
}

void MeshEdit::timerEvent(QTimerEvent *) {
    glWidget->repaint();
}

void MeshEdit::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    restoreGeometry(settings.value("mesh.mesh.geometry").toByteArray());
    ui->centralwidget->restoreState(settings.value("mesh.mesh.windows"));

}

void MeshEdit::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("mesh.mesh.geometry", saveGeometry());
    settings.setValue("mesh.mesh.windows", ui->centralwidget->saveState());
}

void MeshEdit::closeEvent(QCloseEvent *event) {
    QMainWindow::closeEvent(event);

    writeSettings();

    if(isModified()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(tr("The %1 import settings has been modified.").arg(tr("mesh")));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        int result  = msgBox.exec();
        if(result == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
        if(result == QMessageBox::Yes) {
            on_actionSave_triggered();
        }
    }
}

void MeshEdit::loadAsset(IConverterSettings *settings) {
    show();

    if(m_pMesh) {
        delete m_pMesh;
    }

    Prefab *prefab = Engine::loadResource<Prefab>( settings->destination() );
    if(prefab) {
        m_pMesh = static_cast<Actor *>(prefab->actor()->clone(glWidget->scene()));
    }

    float bottom;
    glWidget->controller()->setFocusOn(m_pMesh, bottom);
    if(m_pGround) {
        Transform *t = m_pGround->transform();
        t->setPosition(Vector3(0.0f, bottom - (t->scale().y * 0.5f), 0.0f));
    }

    m_pSettings = settings;
    connect(m_pSettings, SIGNAL(updated()), this, SLOT(onUpdateTemplate()));
    ui->treeView->setObject(m_pSettings);

    m_pConverter = AssetManager::instance()->getConverter(m_pSettings);
}

void MeshEdit::onGLInit() {
    Scene *scene = glWidget->scene();

    m_pLight = Engine::objectCreate<Actor>("LightSource", scene);
    m_pLight->transform()->setRotation(Quaternion(Vector3(-30.0f, 45.0f, 0.0f)));
    DirectLight *light  = static_cast<DirectLight *>(m_pLight->addComponent("DirectLight"));
    light->setCastShadows(true);
    //light->setColor(Vector4(0.99f, 0.83985f, 0.7326f, 1.0f));

    Prefab *prefab = Engine::loadResource<Prefab>(".embedded/cube.fbx");
    if(prefab) {
        m_pGround = static_cast<Actor *>(prefab->actor()->clone(scene));
        m_pGround->transform()->setScale(Vector3(100.0f, 1.0f, 100.0f));
    }
}

void MeshEdit::onUpdateTemplate() {
    setModified(true);
    //m_pMesh->loadUserData(m_pConverter->convertResource(m_pSettings));
}

void MeshEdit::onToolWindowActionToggled(bool state) {
    QWidget *toolWindow = static_cast<QAction*>(sender())->data().value<QWidget *>();
    ui->centralwidget->moveToolWindow(toolWindow, state ?
                                              QToolWindowManager::NewFloatingArea :
                                              QToolWindowManager::NoArea);
}

void MeshEdit::onToolWindowVisibilityChanged(QWidget *toolWindow, bool visible) {
    QAction *action = ui->menuWindow->findChild<QAction *>(toolWindow->windowTitle());
    if(action) {
        action->blockSignals(true);
        action->setChecked(visible);
        action->blockSignals(false);
    }
}

void MeshEdit::on_actionSave_triggered() {
    m_pSettings->saveSettings();
    setModified(false);
}
