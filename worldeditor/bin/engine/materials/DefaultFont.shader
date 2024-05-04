<shader version="11">
    <properties>
        <property type="vec4" name="mainColor"/>
        <property type="vec4" name="clipRect"/>
        <property type="float" name="weight"/>
        <property binding="0" type="texture2d" name="mainTexture"/>
    </properties>
    <vertex><![CDATA[
#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec4 color;

layout(location = 0) out vec4 _uvMask;
layout(location = 1) out vec4 _color;
layout(location = 2) out int _instanceOffset;

void main(void) {
#pragma offset

#pragma instance

    _uvMask = vec4(uv0, vertex.xy * 2.0 - clipRect.xy - clipRect.zw);
    _color = color * mainColor;
    gl_Position = g.projection * ((g.view * modelMatrix) * vec4(vertex, 1.0));
}
]]></vertex>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D mainTexture;

layout(location = 0) in vec4 _uvMask;
layout(location = 1) in vec4 _color;
layout(location = 2) flat in int _instanceOffset;

layout(location = 0) out vec4 color;

void main() {
#pragma instance

    float softness = 0.02f;

    float sdf = texture(mainTexture, _uvMask.xy).x;
    float mask = smoothstep(1.0f - weight - softness, 1.0f - weight + softness, sdf);

    if(g.clip >= mask) {
        discard;
    }

#ifdef VISIBILITY_BUFFER
    color = vec4(objectId);
    return;
#endif

    color = vec4(_color.xyz, mask);
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="Surface" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
        <depth comp="Less" write="false" test="true"/>
    </pass>
</shader>
