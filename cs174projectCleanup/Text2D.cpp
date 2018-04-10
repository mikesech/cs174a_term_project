#include "Text2D.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include <map>
#include <string>

#include "General.h"
using namespace Globals;

namespace {
    struct TextTexture {
        GLuint ref;
        int width;
        int height;
    };

    typedef std::map<std::string, TextTexture> TextTextureCache;
    TextTextureCache cache;

    const char FONT_RESOURCE_PATH[] = "Resources/FreeMonoBold.ttf";
    const int FONT_PT = 128;
    const float FONT_SCALE = 0.025;

    // SDL_TTF must be inited before calling this!
    TTF_Font* ttf_font() {
        static TTF_Font* f = TTF_OpenFont(FONT_RESOURCE_PATH, FONT_PT);
        return f;
    }
}

void
Text2D::drawStaticText(const char* s, vec4 color, GLfloat location_x, GLfloat location_y )
{
    if (!(s && *s))
        return;
    if (!TTF_WasInit())
        TTF_Init();

    TextTexture textTexture;
    TextTextureCache::iterator i = cache.find(s);
    if (i == cache.end()) {
        TTF_Font* const font = ttf_font();
        SDL_Color c = {static_cast<Uint8>(color.x * 255), static_cast<Uint8>(color.y * 255), static_cast<Uint8>(color.z * 255)};
        SDL_Surface* surface = TTF_RenderText_Blended(font, s, c);
        textTexture.width = surface->w;
        textTexture.height = surface->h;

        SDL_LockSurface(surface);
        {
            // Generate one new texture Id.
            glGenTextures(1,&textTexture.ref);
            // Make this texture the active one, so that each
            // subsequent glTex* calls will affect it.
            glBindTexture(GL_TEXTURE_2D,textTexture.ref);

            // Finally, generate the texture data in OpenGL.
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, surface->w, surface->h,
                        0,GL_RGBA,GL_UNSIGNED_BYTE,surface->pixels);
            
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        SDL_UnlockSurface(surface);
        SDL_FreeSurface(surface);
        
        cache[s] = textTexture; // TODO: use insert and not the subscript operator, which causes an initial default initialization
    } else {
        textTexture = i->second;
    }

    // The implementation of this routine borrows from the "UI Hack" in general.cpp/callbackDisplay().
    // That function uses the cube.obj object with a orthographic matrix to render objects in screen
    // space. This function uses the same object, textured with the text texture generated above,
    // to render text on the screen. Ideally, we'd use a different shader with a couple of polygons,
    // all purpose-built for this purpose. However, the core engine wasn't written to support multiple
    // shader programs, and the hack suffices for our purposes.
    //
    // It should be noted, however, that a consequence of this hack is the inability to reliably render
    // colors accurately. This is further explained in an XXX comment below.

    glEnable(GL_BLEND);

    // XXX: Using an ambient light at full power makes the text always white.
    //      However, using a less intense ambient light might cause the text
    //      to be less intense than the color parameter would otherwise indicate.
    setAmbientLightColor(vec3(1,1,1));
    setCameraTransMatrix(mat4());
    setPerspectiveMatrix(currentCamera->getOrthographicMatrix());
    setCameraPosition(vec3(0,0,.5));

    DrawableEntity d(NULL,"Resources/cube.obj");
    d.setTexture(textTexture.ref);
    const float aspectRatio = static_cast<float>(textTexture.width)/textTexture.height;
    const float xScale = FONT_SCALE * aspectRatio;
    const float yScale = FONT_SCALE;
    d.scale(xScale, yScale, .1);
    // location_x and location_y specify the desired upper-left corner of the text.
    // The origin of cube.obj, however, is in its center. The cube is 1x1x1, so we
    // need to move the cube 0.5 down and right; translation is done after scaling,
    // however, so we need to adjust for each axis' scale.
    d.translate(location_x + 0.5 * xScale, location_y + 0.5 * yScale, -10);

    // Cube is meant to be used with a texture that is divided into a 4x4 grid, one box for each side.
    // With a UV scale of 4, 4 (and S, T wrapping set to repeat), we'll use the entire texture for each
    // side instead.
    d.setUVScale(4,4);
    // For some reason, the texture vertices assigned in cube.obj have an X offset of 0.125 normalized.
    // Scaled by 4, to reverse that offset, we have to add an offset of -0.5.
    d.setUVOffset(-0.5, 0);

    d.draw();

    glDisable(GL_BLEND);
}
