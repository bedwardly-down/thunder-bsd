#include "pipelinetasks/reflections.h"

#include "engine.h"

#include "components/world.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

#include "filters/blur.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "amath.h"

namespace {
    const char *reflections("graphics.reflections");
};

Reflections::Reflections() :
        m_slrMaterial(nullptr),
        m_combineMaterial(nullptr),
        m_environmentTexture(nullptr),
        m_slrTarget(Engine::objectCreate<RenderTarget>()),
        m_combineTarget(Engine::objectCreate<RenderTarget>()) {

    setName("Reflections");

    m_inputs.push_back("normalsMap");
    m_inputs.push_back("paramsMap");
    m_inputs.push_back("emissiveMap");
    m_inputs.push_back("depthMap");

    Engine::setValue(reflections, true);

    m_outputs.push_back(make_pair("Result", nullptr));

    Texture *slrTexture(Engine::objectCreate<Texture>("localReflections"));
    slrTexture->setFormat(Texture::RGBA32Float);
    m_outputs.push_back(make_pair(slrTexture->name(), slrTexture));

    m_slrTarget->setColorAttachment(0, slrTexture);

    Material *material = Engine::loadResource<Material>(".embedded/IblReflections.shader");
    if(material) {
        m_combineMaterial = material->createInstance();
        m_combineMaterial->setTexture("rgbMap", slrTexture);
        m_combineMaterial->setTexture("environmentMap", m_environmentTexture);
    }

    material = Engine::loadResource<Material>(".embedded/SSLR.shader");
    if(material) {
        m_slrMaterial = material->createInstance();
    }
}

void Reflections::exec(PipelineContext *context) {
    CommandBuffer *buffer = context->buffer();

    if(m_slrMaterial) { // sslr step
        buffer->setRenderTarget(m_slrTarget);
        buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_slrMaterial);
    }

    if(m_combineMaterial) { // combine step
        buffer->setRenderTarget(m_combineTarget);
        buffer->clearRenderTarget();
        buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_combineMaterial);
    }
}

void Reflections::setInput(int index, Texture *texture) {
    m_combineTarget->setColorAttachment(0, texture);

    if(index == 2) {
        m_outputs.front().second = texture;
    }
}