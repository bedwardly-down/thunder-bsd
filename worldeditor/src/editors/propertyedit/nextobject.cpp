#include "nextobject.h"

#include <QVariant>
#include <QColor>
#include <QEvent>

#include <object.h>

#include "custom/Vector2DProperty.h"
#include "custom/Vector3DProperty.h"
#include "custom/ColorProperty.h"
#include "custom/FilePathProperty.h"
#include "custom/AssetProperty.h"

#include "managers/undomanager/undomanager.h"

#include <engine.h>
#include <components/component.h>
#include <components/actor.h>
#include <resources/material.h>
#include <resources/mesh.h>

#include "assetmanager.h"

#include <QDebug>
#include <QMap>
#include <QMenu>

#define COLOR       "Color"
#define COMPONENT   "Component"

Q_DECLARE_METATYPE(Vector4)

void fixEuler(Vector3 &euler) {
    const double negative = -0.0001;
    const double positive = 360 - 0.0001;

    if((euler.x == 180 && euler.z == 180) || (euler.x ==-180 && euler.z ==-180)) {
        euler.x = 0;
        euler.z = 0;
        euler.y = 180 - euler.y;
    }

    if(euler.x < negative) {
        euler.x += 360;
    } else if(euler.x > positive) {
        euler.x -= 360;
    }
    if(euler.y < negative) {
        euler.y += 360;
    } else if(euler.y > positive) {
        euler.y -= 360;
    }
    if(euler.z < negative) {
        euler.z += 360;
    } else if(euler.z > positive) {
        euler.z -= 360;
    }
}

QVariant qVariant(Variant &v, const string &type) {
    switch(v.userType()) {
        case MetaType::BOOLEAN:
            return QVariant(v.toBool());
        case MetaType::INTEGER:
            return QVariant(v.toInt());
        case MetaType::FLOAT:
            return QVariant(v.toFloat());
        case MetaType::STRING:
            return QVariant(v.toString().c_str());
        case MetaType::VECTOR2:
            return QVariant::fromValue(v.toVector2());
        case MetaType::VECTOR3:
            return QVariant::fromValue(v.toVector3());
        case MetaType::VECTOR4: {
            Vector4 value = v.toVector4();
            if(type == COLOR) {
                QColor r;
                r.setRgbF(value.x, value.y, value.z, value.w);
                return QVariant(r);
            }
            return QVariant::fromValue(value);
        }
        case MetaType::QUATERNION: {
            Vector3 rot   = v.toQuaternion().euler();
            fixEuler(rot);
            return QVariant::fromValue(rot);
        }
        case IConverter::ContentTexture: {
            Texture *t  = v.value<Texture *>();
            return QVariant::fromValue(Template(Engine::reference(t).c_str(), (IConverter::ContentTypes)v.userType()));
        }
        case IConverter::ContentMaterial: {
            Material *t = v.value<Material *>();
            return QVariant::fromValue(Template(Engine::reference(t).c_str(), (IConverter::ContentTypes)v.userType()));
        }
        case IConverter::ContentMesh: {
            Mesh *t     = v.value<Mesh *>();
            return QVariant::fromValue(Template(Engine::reference(t).c_str(), (IConverter::ContentTypes)v.userType()));
        }
        default: break;
    }
    return QVariant();
}

Variant aVariant(QVariant &v, int type) {
    switch(type) {
        case MetaType::BOOLEAN: {
            return Variant(v.toBool());
        }
        case MetaType::INTEGER: {
            return Variant(v.toInt());
        }
        case MetaType::FLOAT: {
            return Variant(v.toFloat());
        }
        case MetaType::STRING: {
            if(v.canConvert<FilePath>()) {
                FilePath p  = v.value<FilePath>();
                return Variant(qPrintable(p.path));
            }
            if(v.canConvert<Template>()) {
                Template p  = v.value<Template>();
                return Variant(qPrintable(p.path));
            }
            return Variant(qPrintable(v.toString()));
        }
        case MetaType::VECTOR2: {
            return Variant(v.value<Vector2>());
        }
        case MetaType::VECTOR3: {
            return Variant(v.value<Vector3>());
        }
        case MetaType::VECTOR4: {
            if(v.canConvert<QColor>()) {
                QColor c    = v.value<QColor>();
                Vector4 v(c.redF(), c.greenF(), c.blueF(), c.alphaF());
                return Variant(v);
            }
            return Variant(v.value<Vector4>());
        }
        case MetaType::QUATERNION: {
            return Variant(Quaternion (v.value<Vector3>()));
        }
        case IConverter::ContentTexture: {
            Template p  = v.value<Template>();
            if(!p.path.isEmpty()) {
                Texture *t = Engine::loadResource<Texture>(qPrintable(p.path));
                return Variant::fromValue(t);
            }
        }
        case IConverter::ContentMaterial: {
            Template p  = v.value<Template>();
            if(!p.path.isEmpty()) {
                Material *m = Engine::loadResource<Material>(qPrintable(p.path));
                return Variant::fromValue(m);
            }
        }
        case IConverter::ContentMesh: {
            Template p  = v.value<Template>();
            if(!p.path.isEmpty()) {
                Mesh *m = Engine::loadResource<Mesh>(qPrintable(p.path));
                return Variant::fromValue(m);
            }
        }
        default: break;
    }
    return Variant();
}

NextObject::NextObject(Object *data, ObjectCtrl *ctrl, QObject *parent) :
        QObject(parent),
        m_pController(ctrl) {

    m_Objects.push_back(data);

    onUpdated();
}

QString NextObject::name() {
    for(auto it : m_Objects) {
        return it->name().c_str();
    }
    return QString();
}

void NextObject::setName(const QString &name) {
    for(auto it : m_Objects) {
        it->setName(qPrintable(name));
        emit updated();
    }
}

QMenu *NextObject::menu(const QString &name) {
    QMenu *result   = nullptr;
    Actor *actor    = dynamic_cast<Actor *>(m_Objects.front());
    if(actor) {
        Component *component    = actor->component(qPrintable(name));
        if(component) {
            result  = new QMenu();

            QAction *del    = new QAction(tr("Remove Component"), this);
            del->setProperty(COMPONENT, name);
            result->addAction(del);

            connect(del, SIGNAL(triggered(bool)), this, SLOT(onDeleteComponent()));
        }
    }

    return result;
}

void NextObject::onUpdated() {
    foreach(QByteArray it, dynamicPropertyNames()) {
        setProperty(it, QVariant());
    }

    for(auto object : m_Objects) {
        buildObject(object);

        setObjectName(object->typeName().c_str());
        emit updated();
    }
}

void NextObject::onDeleteComponent() {
    QObject *snd    = sender();
    QString name    = snd->property(COMPONENT).toString();
    if(!name.isEmpty()) {
        for(auto object : m_Objects) {
            Actor *actor    = dynamic_cast<Actor *>(object);
            if(actor) {
                Component *component    = actor->component(qPrintable(name));
                if(component) {
                    if(m_pController) {
                        Object::ObjectList list;
                        list.push_back(component);
                        UndoManager::instance()->push(new UndoManager::DestroyObjects(list, m_pController, tr("Remove Component ") + name));
                    }
                    onUpdated();
                }
            }
        }
    }
}

void NextObject::buildObject(Object *object, const QString &path) {
    const MetaObject *meta = object->metaObject();

    for(int i = 0; i < meta->propertyCount(); i++) {
        MetaProperty property = meta->property(i);
        QString name    = (path.isEmpty() ? "" : path + "/") + property.name();
        Variant data    = property.read(object);

        if(data.userType() == MetaType::type<MaterialArray>()) {
            MaterialArray array = data.value<MaterialArray>();
            for(uint32_t i = 0; i < array.size(); i++) {
                Variant v   = Variant::fromValue(array[i]->material());
                setProperty( qPrintable(name + "/Item" + QString::number(i)), qVariant(v, "") );
            }
        } else {
            blockSignals(true);
            setProperty( qPrintable(name), qVariant(data, property.type().name()) );
            blockSignals(false);
        }
    }
    for(Object *it : object->getChildren()) {
        if(dynamic_cast<Component *>(it) != nullptr) {
            buildObject(it, (path.isEmpty() ? "" : path + "/") + QString::fromStdString(it->typeName()));
        }
    }
}

bool NextObject::event(QEvent *e) {
    if(e->type() == QEvent::DynamicPropertyChange && !signalsBlocked()) {
        QDynamicPropertyChangeEvent *ev = static_cast<QDynamicPropertyChangeEvent *>(e);
        QString name    = ev->propertyName();
        QVariant value  = property(qPrintable(name));
        if(value.isValid()) {
            QStringList list = name.split('/');
            for(Object *it : m_Objects) {
                Object *o   = findChild(it, list);
                Variant current = o->property(qPrintable(list.front()));
                Variant target;
                if(current.userType() == MetaType::type<MaterialArray>()) {
                    MaterialArray array = current.value<MaterialArray>();
                    uint32_t id = name.mid(name.indexOf(QRegExp("[0-9]"))).toInt();
                    if(id < array.size()) {
                        Material *m     = aVariant(value, MetaType::type<Material *>()).value<Material *>();
                        if(m) {
                            MaterialInstance *inst  = array[id];
                            delete inst;
                            array[id]   = m->createInstance();
                        }
                    }
                    target  = Variant::fromValue(array);
                } else {
                    target  = aVariant(value, current.userType());
                }
                if(target.isValid() && current != target) {
                    if(m_pController) {
                        UndoManager::instance()->push(new UndoManager::PropertyObjects(m_Objects, m_pController));
                    }
                    o->setProperty(qPrintable(list.front()), target);
                    onUpdated();
                }
            }
        }
    }
    return false;
}

Object *NextObject::findChild(Object *parent, QStringList &path) {
    foreach(QString str, path) {
        for(Object *it : parent->getChildren()) {
            if(it->typeName() == str.toStdString()) {
                parent  = it;
                path.pop_front();
                break;
            }
        }
    }
    return parent;
}