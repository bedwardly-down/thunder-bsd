#include "sprite.h"

#include "engine.h"
#include "texture.h"

#include "atlas.h"

#include <cstring>

#define HEADER  "Header"
#define DATA    "Data"
#define MESHES  "Meshes"

typedef deque<Texture *> Textures;
typedef unordered_map<int, Mesh *> Meshes;

static Vector3Vector vertEmpty;
static IndexVector trisEmpty;

class SpritePrivate {
public:
    SpritePrivate() :
        m_pTexture(nullptr),
        m_pRoot(new AtlasNode) {

    }
    Meshes m_Meshes;

    Texture *m_pTexture;

    Textures m_Sources;

    AtlasNode *m_pRoot;
};

/*!
    \class Sprite
    \brief Represents 2D sprite.
    \inmodule Resource

    Sprites usually used in games to display environment and characters in 2D games.
    This class also supports sprite sheets to contain several images in one container to simplify animation or handle tile maps.
*/

Sprite::Sprite() :
        p_ptr(new SpritePrivate) {

    p_ptr->m_pTexture = Engine::objectCreate<Texture>("", this);
    p_ptr->m_pTexture->setFiltering(Texture::Bilinear);
    resize(1024, 1024);
}

Sprite::~Sprite() {
    PROFILE_FUNCTION();

    clearAtlas();

    delete p_ptr;
    p_ptr = nullptr;
}
/*!
    \internal
*/
void Sprite::clearAtlas() {
    PROFILE_FUNCTION();

    for(auto it : p_ptr->m_Meshes) {
        delete it.second;
    }
    p_ptr->m_Meshes.clear();

    for(auto it : p_ptr->m_Sources) {
        delete it;
    }
    p_ptr->m_Sources.clear();
}
/*!
    Adds new sub \a texture as element to current sprite sheet.
    All elements will be packed to a single sprite sheet texture using Sprite::pack() method.
    Returns the id of the new element.

    \sa pack()
*/
int Sprite::addElement(Texture *texture) {
    PROFILE_FUNCTION();

    p_ptr->m_Sources.push_back(texture);

    Lod lod;
    lod.setVertices({Vector3(0.0f, 0.0f, 0.0f),
                     Vector3(0.0f, 1.0f, 0.0f),
                     Vector3(1.0f, 1.0f, 0.0f),
                     Vector3(1.0f, 0.0f, 0.0f) });

    lod.setIndices({0, 1, 2, 0, 2, 3});

    Mesh *mesh = Engine::objectCreate<Mesh>("Mesh");
    mesh->addLod(&lod);

    int index = (p_ptr->m_Sources.size() - 1);
    p_ptr->m_Meshes[index] = mesh;

    return index;
}
/*!
    Packs all added elements int to a single sprite sheet.
    Parameter \a padding can be used to delimit elements.

    \sa addElement()
*/
void Sprite::pack(int padding) {
    PROFILE_FUNCTION();

    for(size_t i = 0; i < p_ptr->m_Sources.size(); i++) {
        Texture *it = p_ptr->m_Sources[i];
        Mesh *m = mesh(i);
        Lod *lod = m->lod(0);

        int32_t width  = (it->width() + padding * 2);
        int32_t height = (it->height() + padding * 2);

        AtlasNode *n = p_ptr->m_pRoot->insert(width, height);
        if(n && lod) {
            n->fill = true;
            int32_t w = n->w - padding * 2;
            int32_t h = n->h - padding * 2;

            Vector4 uv;
            uv.x = n->x / static_cast<float>(p_ptr->m_pRoot->w);
            uv.y = n->y / static_cast<float>(p_ptr->m_pRoot->h);
            uv.z = uv.x + w / static_cast<float>(p_ptr->m_pRoot->w);
            uv.w = uv.y + h / static_cast<float>(p_ptr->m_pRoot->h);

            int8_t *src = &(it->surface(0)[0])[0];
            int8_t *dst = &(p_ptr->m_pTexture->surface(0)[0])[0];
            for(int32_t y = padding; y < h; y++) {
                memcpy(&dst[(y + n->y) * p_ptr->m_pRoot->w + n->x], &src[y * w], w);
            }

            lod->setUv0({Vector2(uv.x, uv.y),
                         Vector2(uv.z, uv.y),
                         Vector2(uv.z, uv.w),
                         Vector2(uv.x, uv.w)});
        } else {
            resize(p_ptr->m_pRoot->w * 2, p_ptr->m_pRoot->h * 2);
            pack(padding);
        }
    }

    p_ptr->m_pTexture->setDirty();
}

bool Sprite::composeMesh(int key, Mesh *spriteMesh, Vector2 &size, bool tiled, bool resetSize) {
    Mesh *m = mesh(key);
    if(m) {
        Lod *lod = m->lod(0);
        spriteMesh->setLod(0, lod);
        lod = spriteMesh->lod(0);
        Vector3Vector &vetrs = lod->vertices();

        Vector3 delta(vetrs[15] - vetrs[0]);
        if(resetSize) {
            size = Vector2(delta.x, delta.y);
        }

        if(tiled) {
            Vector2Vector &uvs = lod->uv0();
            IndexVector &indices = lod->indices();

            Vector2 ubl(uvs[0]);
            Vector2 utr(uvs[15]);

            int width = ceilf(size.x / delta.x);
            int height = ceilf(size.y / delta.y);

            vetrs.resize(width * height * 4);
            uvs.resize(width * height * 4);
            indices.resize(width * height * 6);

            Vector3 bl(Vector3(size, 0.0f) * -0.5f);

            int i = 0;
            for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++) {
                    int index = (y * width + x) * 4;

                    Vector2 f(1.0f);
                    if(x == width - 1) {
                        f.x = MIN((size.x * 0.5f - bl.x) / delta.x, 1.0f);
                    }
                    if(y == height - 1) {
                        f.y = MIN((size.y * 0.5f - bl.y) / delta.y, 1.0f);
                    }

                    vetrs[index] = bl;
                    vetrs[index + 1] = bl + Vector3(delta.x * f.x, 0.0f, 0.0f);
                    vetrs[index + 2] = bl + Vector3(delta.x * f.x, delta.y * f.y, 0.0f);
                    vetrs[index + 3] = bl + Vector3(0.0f, delta.y * f.y, 0.0f);

                    uvs[index] = ubl;
                    uvs[index + 1] = ubl + Vector2((utr.x - ubl.x) * f.x, 0.0f);
                    uvs[index + 2] = ubl + Vector2((utr.x - ubl.x) * f.x, (utr.y - ubl.y) * f.y);
                    uvs[index + 3] = ubl + Vector2(0.0f, (utr.y - ubl.y) * f.y);

                    indices[i]     = index;
                    indices[i + 1] = index + 1;
                    indices[i + 2] = index + 2;
                    indices[i + 3] = index;
                    indices[i + 4] = index + 2;
                    indices[i + 5] = index + 3;

                    bl.x += delta.x;

                    i += 6;
                }
                bl.y += delta.y;
            }
        } else {
            Vector2 scale(size.x / delta.x, size.y / delta.y);
            {
                float bl = vetrs[0].x;
                float br = vetrs[3].x;
                vetrs[ 0].x *= scale.x; vetrs[ 3].x *= scale.x;
                vetrs[ 4].x *= scale.x; vetrs[ 7].x *= scale.x;
                vetrs[ 8].x *= scale.x; vetrs[11].x *= scale.x;
                vetrs[12].x *= scale.x; vetrs[15].x *= scale.x;
                float dl = vetrs[0].x - bl;
                float dr = vetrs[3].x - br;
                vetrs[ 1].x += dl; vetrs[ 2].x += dr;
                vetrs[ 5].x += dl; vetrs[ 6].x += dr;
                vetrs[ 9].x += dl; vetrs[10].x += dr;
                vetrs[13].x += dl; vetrs[14].x += dr;
            }
            {
                float bl = vetrs[ 0].y;
                float br = vetrs[12].y;
                vetrs[ 0].y *= scale.y; vetrs[12].y *= scale.y;
                vetrs[ 1].y *= scale.y; vetrs[13].y *= scale.y;
                vetrs[ 2].y *= scale.y; vetrs[14].y *= scale.y;
                vetrs[ 3].y *= scale.y; vetrs[15].y *= scale.y;
                float dl = vetrs[ 0].y - bl;
                float dr = vetrs[12].y - br;
                vetrs[ 4].y += dl; vetrs[ 8].y += dr;
                vetrs[ 5].y += dl; vetrs[ 9].y += dr;
                vetrs[ 6].y += dl; vetrs[10].y += dr;
                vetrs[ 7].y += dl; vetrs[11].y += dr;
            }
        }

        spriteMesh->setFlags(m->flags());
        spriteMesh->setMode(m->mode());
        spriteMesh->recalcBounds();
        return true;
    }
    return false;
}

/*!
    \internal
    Changes current size of the sprite sheet and sets resorce state to ResourceState::ToBeUpdated.
*/
void Sprite::resize(int32_t width, int32_t height) {
    PROFILE_FUNCTION();

    if(p_ptr->m_pRoot) {
        delete p_ptr->m_pRoot;
        p_ptr->m_pRoot = new AtlasNode;
    }
    p_ptr->m_pRoot->w = width;
    p_ptr->m_pRoot->h = height;

    p_ptr->m_pTexture->resize(width, height);
}
/*!
    \internal
*/
void Sprite::loadUserData(const VariantMap &data) {
    clearAtlas();
    {
        auto it = data.find(DATA);
        if(it != data.end()) {
            Object *object = ObjectSystem::toObject(it->second);
            Texture *texture = dynamic_cast<Texture *>(object);
            if(texture) {
                delete p_ptr->m_pTexture; // May lead to crash in case of m_pTexture had references
                p_ptr->m_pTexture = texture;
                p_ptr->m_pTexture->setParent(this);
            }
        }
    }
    {
        auto it = data.find(MESHES);
        if(it != data.end()) {
            for(auto mesh : it->second.toList()) {
                VariantList array = mesh.toList();
                Object *object = ObjectSystem::toObject(array.back());
                Mesh *m = dynamic_cast<Mesh *>(object);
                if(m) {
                    int key = array.front().toInt();
                    setMesh(key, m);
                }
            }
        }
    }

    setState(Ready);
}
/*!
    \internal
*/
VariantMap Sprite::saveUserData() const {
    VariantMap result;

    Variant data = ObjectSystem::toVariant(p_ptr->m_pTexture);
    if(data.isValid()) {
        result[DATA] = data;
    }

    if(!p_ptr->m_Meshes.empty()) {
        VariantList meshes;

        for(auto it : p_ptr->m_Meshes) {
            VariantList mesh;
            mesh.push_back(it.first);
            mesh.push_back(ObjectSystem::toVariant(it.second));
            meshes.push_back(mesh);
        }
        result[MESHES] = meshes;
    }

    return result;
}
/*!
    Returns a mesh which represents the sprite with \a key.
*/
Mesh *Sprite::mesh(int key) const {
    PROFILE_FUNCTION();

    auto it = p_ptr->m_Meshes.find(key);
    if(it != p_ptr->m_Meshes.end()) {
        return it->second;
    }
    return nullptr;
}
/*!
    Sets a new \a mesh for the sprite with \a key.
    The old mesh will be deleted and no longer available.
*/
void Sprite::setMesh(int key, Mesh *mesh) {
    PROFILE_FUNCTION();

    if(p_ptr->m_Meshes[key]) {
        delete p_ptr->m_Meshes[key];
    }
    p_ptr->m_Meshes[key] = mesh;
}
/*!
    Returns a sprite sheet texture.
*/
Texture *Sprite::texture() const {
    PROFILE_FUNCTION();

    return p_ptr->m_pTexture;
}
/*!
    Sets a new sprite sheet \a texture.
*/
void Sprite::setTexture(Texture *texture) {
    PROFILE_FUNCTION();

    if(p_ptr->m_pTexture) {
        delete p_ptr->m_pTexture;
    }
    p_ptr->m_pTexture = texture;
    p_ptr->m_pTexture->setParent(this);
}
