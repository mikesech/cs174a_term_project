#include "Framebuffer.h"

Framebuffer::Framebuffer(int width, int height) : _width(width), _height(height) {
	//Initialize objects
	glGenFramebuffers(1, &_framebuffer);
	glGenFramebuffers(1, &_blitFramebuffer);
	glGenTextures(1, &_texture);
	glGenTextures(1, &_backTexture);
	glGenRenderbuffers(1, &_zBuffer);
}

Framebuffer::~Framebuffer() {
	glDeleteRenderbuffers(1, &_zBuffer);
	glDeleteTextures(1, &_backTexture);
	glDeleteTextures(1, &_texture);
	glDeleteFramebuffers(1, &_blitFramebuffer);
	glDeleteFramebuffers(1, &_framebuffer);
}

void Framebuffer::render(RenderFunc renderFunc, int width, int height) {
	//Save the original viewport so we can restore it in the end
	GLint origViewport[4];
	glGetIntegerv(GL_VIEWPORT, origViewport);

	glViewport(0,0,width,height);

	//Bind framebuffer and texture
	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
	glBindTexture(GL_TEXTURE_2D, _backTexture);

	//Define and reset texture
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

	//Create new Z buffer
	glBindRenderbuffer(GL_RENDERBUFFER, _zBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _zBuffer);

	//Attach texture to frame buffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _backTexture, 0);

	//Render stuff as normal
	renderFunc();

	// Copy from _backTexture to _texture
	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _blitFramebuffer);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0); // Level 0
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST); // GL_NEAREST for no scaling

	//Configure texture and generate mipmap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	//Unbind framebuffer and render buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//Reset viewport
	glViewport(origViewport[0], origViewport[1], origViewport[2], origViewport[3]);
}
