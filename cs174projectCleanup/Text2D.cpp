#include "Text2D.h"
#include "SDL.h"
#include <map>
#include <string>
#include <cassert>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include "SDL_ttf.h"
#endif

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

    const char FONT_RESOURCE_PATH[] = "resources/FreeMonoBold.ttf";
    const int FONT_PT = 128;
    const float FONT_SCALE = 0.025;

#ifdef __EMSCRIPTEN__
    TextTexture renderTextToTexture(const char* s) {
        static bool init = []() {
            EM_ASM({
                const fontPath = UTF8ToString($0);
                const contents = FS.readFile(fontPath, {encoding: 'binary'});
                const fontFace = new FontFace('text2d-font', contents);
                document.fonts.add(fontFace);
            }, FONT_RESOURCE_PATH);
            return true;
        }();

        TextTexture textTexture;
        int32_t width, height;

        glGenTextures(1,&textTexture.ref);
        glBindTexture(GL_TEXTURE_2D,textTexture.ref);

        EM_ASM({
            const text = UTF8ToString($0);
            const font = `${$1}px "text2d-font"`;
            const canvas = document.createElement('canvas');
            const ctx = canvas.getContext('2d');

            const setCtxProps = (ctx) => {
                ctx.font = font;
                ctx.textRendering = 'geometricPrecision';
                ctx.fontKerning = 'none';
                ctx.fillStyle = '#000000FF';
                // ctx.fillStyle = '#FF0000FF';
            };
            setCtxProps(ctx);
            const metrics = ctx.measureText(text);

            const width = Math.ceil(metrics.width);
            const height = Math.ceil(metrics.fontBoundingBoxAscent + metrics.fontBoundingBoxDescent);

            canvas.width = width;
            canvas.height = height;
            setCtxProps(ctx);

            ctx.fillText(text, 0, Math.ceil(metrics.fontBoundingBoxAscent));
            const gl = Module.canvas.getContext('webgl2');
            gl.texImage2D(
                gl.TEXTURE_2D, 0, gl.RGBA8, width, height,
                0, gl.RGBA, gl.UNSIGNED_BYTE, canvas
            );
            Module.setValue($2, width, 'i32');
            Module.setValue($3, height, 'i32');
        }, s, FONT_PT, &width, &height);
        textTexture.width = width;
        textTexture.height = height;
        glGenerateMipmap(GL_TEXTURE_2D);
        // Edge is less than ideal, but it'll do.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        return textTexture;
    }
#else
    TextTexture renderTextToTexture(const char* s) {
        if (!TTF_WasInit())
            TTF_Init();

        TextTexture textTexture;
        static TTF_Font* const font = TTF_OpenFont(FONT_RESOURCE_PATH, FONT_PT);
        if (!font) throw "no font";
        // Color is implemented by setting the ambient light accordingly when rendering.
        // The final color of a fragment is the ambient color plus the object's specular
        // and diffuse lighting, both of which are dependent on the scene's lighting.
        // We'll set the shininess to zero to remove specular lighting from the equation.
        // With a textured object, the object's diffuse color is the texture color, which
        // is then illuminated by each scene light to make the final diffuse lighting value.
        //
        // We'll play a trick here. We set the text's color to be black so it won't contribute
        // any R, G, or B to the final color. However, it will contribute its alpha value.
        // Consequently, where the text is black (i.e, present), the ambient color will be use.
        // Where the text is not present, the alpha will be zero and nothing will be rendered.
        SDL_Color c = {0, 0, 0, 255};
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
            // On some platforms (e.g., my new M2 Mac running Ventura), the length of each row
            // in bytes is not equal to the number of pixels per row times the number of bytes
            // per pixels. This might have something to do with alignment or other platform
            // optimization concerns. The surface's pitch is the number of bytes per row;
            // we can use it to calculate the size of each row in pixels and provide it to OpenGL.
            assert(surface->pitch % surface->format->BytesPerPixel == 0);
            int oldGLUnpackRowLength;
            glGetIntegerv(GL_UNPACK_ROW_LENGTH, &oldGLUnpackRowLength);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->pitch / surface->format->BytesPerPixel);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, surface->w, surface->h,
                        0,GL_RGBA,GL_UNSIGNED_BYTE,surface->pixels);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, oldGLUnpackRowLength);
            
            glGenerateMipmap(GL_TEXTURE_2D);
            // Clamping prevents an effect where one of the edges wraps to the
            // other side, probably due to min/mag sampling.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        }
        SDL_UnlockSurface(surface);
        SDL_FreeSurface(surface);

        return textTexture;
    }
#endif
}

void
Text2D::drawStaticText(const char* s, vec4 color, GLfloat location_x, GLfloat location_y )
{
    if (!(s && *s))
        return;

    TextTextureCache::iterator i = cache.find(s);
    TextTexture textTexture = i == cache.end() ? (cache[s] = renderTextToTexture(s)) : i->second;

    // The implementation of this routine borrows from the "UI Hack" in general.cpp/callbackDisplay().
    // That function uses the cube.obj object with a orthographic matrix to render objects in screen
    // space. This function uses the same object, textured with the text texture generated above,
    // to render text on the screen. Ideally, we'd use a different shader with a couple of polygons,
    // all purpose-built for this purpose. However, the core engine wasn't written to support multiple
    // shader programs, and the hack suffices for our purposes.

    glEnable(GL_BLEND);

    setAmbientLightColor(vec3(color.x, color.y, color.z));
    setCameraTransMatrix(mat4());
    setPerspectiveMatrix(currentCamera->getOrthographicMatrix());
    setCameraPosition(vec3(0,0,.5));

    DrawableEntity d(NULL,"resources/cube.obj");
    d.setAlpha(color.w);
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
    //
    // The -1 offsets are so that the side facing the camera lines up with the texture.
    // The values were determined empirically.
    d.setUVOffset(-0.5 - 1, -1);

    d.draw();

    glDisable(GL_BLEND);
}
