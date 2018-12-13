#ifndef _TEXT2D
#define _TEXT2D

#include "mat.h"

/** @brief Helper namepsace for 2-D text display.
*/
namespace Text2D {
		/** Draws relatively unchanging (static) text on the screen.
		 * 
		 * @note This function will initialize SDL_TTF if it has not been already.
		 * 
		 * @attention This function should be used to draw text that doesn't change often. For every different
		 * input string @c s, a texture is generated and cached. Texture generation is somewhat
		 * expensive, and cached textures are never cleaned up. Consequently, using this function
		 * for many different strings could easily lead to poor performance or resource exhaustion.
		 * 
		 * @param s The string to draw.
		 * @param color The color with which to render the text.
		 * @param location_x The X coordinate of the upper-left corner of the drawn text, normalized between 0 and 1.
		 * @param location_x The Y coordinate of the upper-left corner of the drawn text, normalized between 0 and 1.
		 */
		void drawStaticText(const char* s, vec4 color, GLfloat location_x, GLfloat location_y);
}

#endif
