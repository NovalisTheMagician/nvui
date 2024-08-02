#include "nvui/window.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tgmath.h>

#include "nvui/element.h"
#include "nvui/private/window.h"
#include "nvui/private/painter.h"
#include "nvui/resources.h"
#include "nvui/glutils.h"
#include "nvui/keys.h"

GlobalState global = {};

bool InitGLData(Window *window)
{
    char logbuffer[512];

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    if(!CompileShader(gShaderVertData, gShaderVertSize, &vertShader, logbuffer, sizeof logbuffer))
        return false;

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(!CompileShader(gShaderFragData, gShaderFragSize, &fragShader, logbuffer, sizeof logbuffer))
        return false;

    GLuint fontShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(!CompileShader(gShaderFontData, gShaderFontSize, &fontShader, logbuffer, sizeof logbuffer))
        return false;

    GLuint circleShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(!CompileShader(gShaderCircleData, gShaderCircleSize, &circleShader, logbuffer, sizeof logbuffer))
        return false;

    GLuint program = glCreateProgram();
    if(!LinkProgram(vertShader, fragShader, &program, logbuffer, sizeof logbuffer))
        return false;

    GLuint fontProgram = glCreateProgram();
    if(!LinkProgram(vertShader, fontShader, &fontProgram, logbuffer, sizeof logbuffer))
        return false;

    GLuint circleProgram = glCreateProgram();
    if(!LinkProgram(vertShader, circleShader, &circleProgram, logbuffer, sizeof logbuffer))
        return false;

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    glDeleteShader(fontShader);
    glDeleteShader(circleShader);

    window->glData.shaderProgram = program;
    window->glData.fontProgram = fontProgram;
    window->glData.circleProgram = circleProgram;
    window->glData.textureLoc = 0; // tex

    window->glData.whiteTexture = CreateSimpleTexture(1, 1, (uint8_t[]){ 255, 255, 255, 255 });

    const GLbitfield 
	mapping_flags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT,
	storage_flags = GL_DYNAMIC_STORAGE_BIT | mapping_flags;
    const size_t bufferSize = BUFFER_SIZE * sizeof(struct Vertex);

    glCreateBuffers(1, &window->glData.vertexBuffer);
    glNamedBufferStorage(window->glData.vertexBuffer, bufferSize, NULL, storage_flags);
    window->glData.mappedVertexBuffer = glMapNamedBufferRange(window->glData.vertexBuffer, 0, bufferSize, mapping_flags);

    VertexFormat fmt[] =
    {
        (VertexFormat){ .size = 2, .type = GL_FLOAT, .offset = offsetof(Vertex, position) },
        (VertexFormat){ .size = 2, .type = GL_FLOAT, .offset = offsetof(Vertex, texcoord) },
        (VertexFormat){ .size = 4, .type = GL_FLOAT, .offset = offsetof(Vertex, color) },
        VERTEX_FORMAT_END
    };
    window->glData.vertexFormat = CreateVertexArray(fmt);
    glVertexArrayVertexBuffer(window->glData.vertexFormat, 0, window->glData.vertexBuffer, 0, sizeof(Vertex));

    Font *font = &window->fonts[Serif];
    FontInit(font, 16);
    FontLoadMem(gFontSerifRegularData, gFontSerifRegularSize, font, Regular);
    FontLoadMem(gFontSerifItalicData, gFontSerifItalicSize, font, Italic);
    FontLoadMem(gFontSerifBoldData, gFontSerifBoldSize, font, Bold);
    FontLoadMem(gFontSerifBoldItalicData, gFontSerifBoldItalicSize, font, BoldItalic);

    font = &window->fonts[Sans];
    FontInit(font, 16);
    FontLoadMem(gFontSansRegularData, gFontSansRegularSize, font, Regular);
    FontLoadMem(gFontSansItalicData, gFontSansItalicSize, font, Italic);
    FontLoadMem(gFontSansBoldData, gFontSansBoldSize, font, Bold);
    FontLoadMem(gFontSansBoldItalicData, gFontSansBoldItalicSize, font, BoldItalic);

    font = &window->fonts[Mono];
    FontInit(font, 16);
    FontLoadMem(gFontMonoRegularData, gFontMonoRegularSize, font, Regular);
    FontLoadMem(gFontMonoItalicData, gFontMonoItalicSize, font, Italic);
    FontLoadMem(gFontMonoBoldData, gFontMonoBoldSize, font, Bold);
    FontLoadMem(gFontMonoBoldItalicData, gFontMonoBoldItalicSize, font, BoldItalic);

    glCreateBuffers(1, &window->glData.matrixBuffer);
    glNamedBufferStorage(window->glData.matrixBuffer, sizeof(MatrixData), NULL, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, window->glData.matrixBuffer);

    glCreateBuffers(1, &window->glData.paintPropBuffer);
    glNamedBufferStorage(window->glData.paintPropBuffer, sizeof(PaintPropsData), NULL, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, window->glData.paintPropBuffer);

    glCreateBuffers(1, &window->glData.circleBuffer);
    glNamedBufferStorage(window->glData.circleBuffer, sizeof(CirclePropsData), NULL, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, window->glData.circleBuffer);

    return true;
}

void DestroyGLData(Window *window)
{
    glDeleteFramebuffers(1, &window->glData.framebuffer);
    GLuint renderbuffers[] = { window->glData.colorRb, window->glData.depthRb };
    glDeleteRenderbuffers(2, renderbuffers);
    glDeleteTextures(1, &window->glData.whiteTexture);
    glDeleteVertexArrays(1, &window->glData.vertexFormat);
    glDeleteProgram(window->glData.shaderProgram);
    glDeleteProgram(window->glData.fontProgram);
    glDeleteProgram(window->glData.circleProgram);
    glDeleteBuffers(1, &window->glData.vertexBuffer);
    glDeleteBuffers(3, (GLuint[]){ window->glData.matrixBuffer, window->glData.paintPropBuffer, window->glData.circleBuffer });
    for(size_t i = 0; i < NumVariants; ++i)
        FontFree(&window->fonts[i]);
}

void ResizeTextures(Window *window)
{
    if(window->glData.colorRb)
    {
        GLuint renderbuffers[] = { window->glData.colorRb, window->glData.depthRb };
        glDeleteRenderbuffers(2, renderbuffers);
    }

    glCreateRenderbuffers(1, &window->glData.colorRb);
    glCreateRenderbuffers(1, &window->glData.depthRb);

    glNamedRenderbufferStorage(window->glData.colorRb, GL_RGBA8, window->width, window->height);
    glNamedRenderbufferStorage(window->glData.depthRb, GL_DEPTH_COMPONENT24, window->width, window->height);

    if(!window->glData.framebuffer)
        glCreateFramebuffers(1, &window->glData.framebuffer);
    glNamedFramebufferRenderbuffer(window->glData.framebuffer, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, window->glData.colorRb);
    glNamedFramebufferRenderbuffer(window->glData.framebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, window->glData.depthRb);

    glBindFramebuffer(GL_FRAMEBUFFER, window->glData.framebuffer);
    glDisable(GL_SCISSOR_TEST);
    glClearNamedFramebufferfv(window->glData.framebuffer, GL_COLOR, 0, (float[]){ 0, 0, 0, 1 });
    glEnable(GL_SCISSOR_TEST);
    glViewport(0, 0, window->width, window->height);
}

int WindowMessage(Element *element, Message message, int di, void *dp)
{
    if(message == MSG_LAYOUT)
    {
        element->window->projection = glms_ortho(0, element->window->width, 0, element->window->height, -1, 1);
        element->window->buffersNeedResize = true;
        if(element->childCount)
        {
            Rectangle bounds = element->bounds;
            if(element->window->menu)
            {
                bounds.t += MENUBAR_HEIGHT+1;
            }
            ElementMove(element->children[0], bounds, false);
            ElementRepaint(element, NULL);
        }
    }
    else if(message == MSG_PAINT && element->window->menu)
    {
        Window *window = element->window;
        Menu *menu = window->menu;
        Painter *painter = dp;

        Rectangle bounds = element->bounds;
        bounds.b = MENUBAR_HEIGHT;

        PainterSetColor(painter, COLOR_CONTROL);
        PainterFillRect(painter, bounds);

        PainterSetColor(painter, ColorFromGrayscale(0.45f));
        PainterDrawLine(painter, bounds.l, bounds.b+1, bounds.r, bounds.b+1);

        Font *menuFont = WindowGetFontVariant(window, DefaultVariant);

        int prevLeft = 0;
        for(size_t i = 0; i < menu->numItems; ++i)
        {
            MenuItem *item = menu->items[i];
            Rectangle menuBounds = bounds;
            menuBounds.l = prevLeft;
            menuBounds.r = prevLeft + floor(FontMeasureString(menuFont, DefaultStyle, item->name, item->nameBytes, 0)) + MENUITEM_MARGIN*2;
            prevLeft = menuBounds.r;

            Color backColor = item == window->itemHovered ? COLOR_BLUE : COLOR_CONTROL;

            PainterSetColor(painter, backColor);
            PainterFillRect(painter, menuBounds);

            Color textColor = item == window->itemHovered ? COLOR_WHITE : COLOR_BLACK;

            PainterSetColor(painter, textColor);
            Rectangle menuTextBounds = menuBounds;
            menuTextBounds.l += MENUITEM_MARGIN;
            menuTextBounds.r -= MENUITEM_MARGIN;
            menuTextBounds.t += 2;
            PainterDrawString(painter, menuTextBounds, item->name, item->nameBytes, false);
        }
    }
    else if(message == MSG_MOUSE_MOVE)
    {
        
    }
    else if(message == MSG_LEFT_DOWN)
    {
        
    }
    return 0;
}

void InvalidateWindow(Window *window);

static void WindowSetFocus(Window *window, Element *element)
{
    Element *previous = window->focused;
    window->focused = element;

    if(previous && previous != element)
    {
        ElementMessage(previous, MSG_UPDATE, UPDATE_FOCUS_LOSE, NULL);
    }

    if(element && previous != element)
    {
        ElementMessage(element, MSG_UPDATE, UPDATE_FOCUS_GAIN, NULL);
    }
}

static void WindowSetPressed(Window *window, Element *element, int button)
{
    Element *previous = window->pressed;

    window->pressed = element;
    window->pressedButton = button;

    if(element) WindowSetFocus(window, element);

    if(previous) ElementMessage(previous, MSG_UPDATE, UPDATE_CLICK_RELEASE, NULL);
    if(element) ElementMessage(element, MSG_UPDATE, UPDATE_CLICK_PRESS, NULL);
}

void WindowInputEvent(Window *window, Message message, int di, void *dp)
{
    if(window->pressed)
    {
        if(message == MSG_MOUSE_MOVE)
        {
            ElementMessage(window->pressed, MSG_MOUSE_DRAG, di, dp);
        }
        else if(message == MSG_LEFT_UP && window->pressedButton == 1)
        {
            if(window->hovered == window->pressed)
            {
                ElementMessage(window->pressed, MSG_CLICKED, di, dp);
            }

            ElementMessage(window->pressed, MSG_LEFT_UP, di, dp);
            WindowSetPressed(window, NULL, 1);
        }
        else if(message == MSG_MIDDLE_UP && window->pressedButton == 2)
        {
            ElementMessage(window->pressed, MSG_MIDDLE_UP, di, dp);
            WindowSetPressed(window, NULL, 2);
        }
        else if(message == MSG_RIGHT_UP && window->pressedButton == 3)
        {
            ElementMessage(window->pressed, MSG_RIGHT_UP, di, dp);
            WindowSetPressed(window, NULL, 3);
        }
    }

    if(window->pressed)
    {
        bool inside = RectangleContains(window->pressed->clip, window->cursorX, window->cursorY);

        if(inside && window->hovered == &window->e)
        {
            window->hovered = window->pressed;
            ElementMessage(window->pressed, MSG_UPDATE, UPDATE_HOVER_ENTER, NULL);
        }
        else if(!inside && window->hovered == window->pressed)
        {
            window->hovered = &window->e;
            ElementMessage(window->pressed, MSG_UPDATE, UPDATE_HOVER_LEAVE, NULL);
        }
    }
    else
    {
        Element *hovered = ElementFindByPoint(&window->e, window->cursorX, window->cursorY);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        switch(message)
        {
        case MSG_MOUSE_MOVE:
            ElementMessage(hovered, MSG_MOUSE_MOVE, di, dp);
            break;
        case MSG_LEFT_DOWN:
            WindowSetPressed(window, hovered, 1);
            ElementMessage(hovered, message, di, dp);
            break;
        case MSG_MIDDLE_DOWN:
            WindowSetPressed(window, hovered, 2);
            ElementMessage(hovered, message, di, dp);
            break;
        case MSG_RIGHT_DOWN:
            WindowSetPressed(window, hovered, 3);
            ElementMessage(hovered, message, di, dp);
            break;
        }
#pragma GCC diagnostic pop

        if(hovered != window->hovered)
        {
            Element *previous = window->hovered;
            window->hovered = hovered;
            ElementMessage(previous, MSG_UPDATE, UPDATE_HOVER_LEAVE, NULL);
            ElementMessage(window->hovered, MSG_UPDATE, UPDATE_HOVER_ENTER, NULL);
        }
    }

    InvalidateWindow(window);
}

void WindowKeyEvent(Window *window, Message message, int di, void *dp)
{
    if(di == KEY_LCTRL || di == KEY_RCTRL) window->ctrlDown = message == MSG_KEY_DOWN;
    if(di == KEY_LSHIFT || di == KEY_RSHIFT) window->shiftDown = message == MSG_KEY_DOWN;
    if(di == KEY_LALT || di == KEY_RALT) window->altDown = message == MSG_KEY_DOWN;

    if(window->focused)
    {
        uint16_t metaState = (window->ctrlDown << 2) | (window->shiftDown << 1) | window->altDown;
        ElementMessage(window->focused, message, di, (void*)(uintptr_t)metaState);
    }
}

void WindowCharEvent(Window *window, Message message, int di, void *dp)
{
    if(window->focused)
    {
        di = di == '\r' ? '\n' : di;
        ElementMessage(window->focused, message, di, dp);
    }
}

void WindowEndPaint(Window *window, Painter *painter);

static void ElementPaint(Element *element, Painter *painter)
{
    Rectangle clip = RectangleIntersection(element->clip, painter->clip);
    if(!RectangleValid(clip)) return;

    const GLData gldata = element->window->glData;

    painter->clip = clip;
    glScissor(clip.l - 1, clip.t - 1, clip.r - clip.l + 1, clip.b - clip.t + 1);
    
    glUseProgram(gldata.shaderProgram);
    glBindTextureUnit(0, gldata.whiteTexture);
    glUniform1i(gldata.textureLoc, 0);

    ElementMessage(element, MSG_PAINT, 0, painter);

    for(size_t i = 0; i < element->childCount; ++i)
    {
        painter->clip = clip;
        ElementPaint(element->children[i], painter);
    }
}

static bool ElementDestroyNow(Element *element)
{
    if(element->flags & ELEMENT_DESTROY_DESCENDENT)
    {
        element->flags &= ~ELEMENT_DESTROY_DESCENDENT;

        for(size_t i = 0; i < element->childCount; ++i)
        {
            if(ElementDestroyNow(element->children[i]))
            {
                memmove(&element->children[i], &element->children[i + 1], sizeof(Element*) * (element->childCount - i - 1));
                element->childCount--;
                i--;
            }
        }
    }

    if(element->flags & ELEMENT_DESTROY)
    {
        ElementMessage(element, MSG_DESTROY, 0, NULL);

        if(element->window->pressed == element)
        {
            WindowSetPressed(element->window, NULL, 0);
        }

        if(element->window->hovered == element)
        {
            element->window->hovered = &element->window->e;
        }

        free(element->children);
        free(element);
        return true;
    }
    else
    {
        return false;
    }
}

void RemoveWindow(Window *window)
{
    for(size_t i = 0; i < global.windowCount; ++i)
    {
        if(global.windows[i] == window)
        {
            global.windows[i] = global.windows[global.windowCount - 1];
            global.windowCount--;
            return;
        }
    }
}

void Update(Window *window)
{
    if(ElementDestroyNow(window->e.children[0]))
    {
        if(window->menu)
            MenuDestroy(window->menu);
        RemoveWindow(window);
    }
    else if(RectangleValid(window->updateRegion))
    {
        const GLData gldata = window->glData;

        Painter painter = {};
        painter.width = window->width;
        painter.height = window->height;
        painter.clip = RectangleIntersection((Rectangle){ .r = window->width, .b = window->height }, window->updateRegion);
        painter.lineWidth = 1;
        painter.pixelSizeW = 1.0f / window->width;
        painter.pixelSizeH = 1.0f / window->height;

        painter.defaultFont = &window->fonts[DefaultVariant];
        painter.fontStyle = Regular;
        painter.gldata = gldata;
        painter.vertexMap = gldata.mappedVertexBuffer;

        glNamedBufferSubData(gldata.matrixBuffer, 0, sizeof(MatrixData), &(MatrixData){ .projection = window->projection });

        glBindFramebuffer(GL_FRAMEBUFFER, gldata.framebuffer);
        glBindVertexArray(gldata.vertexFormat);
        glUseProgram(gldata.shaderProgram);

        ElementPaint(&window->e, &painter);
        WindowEndPaint(window, &painter);
        window->updateRegion = (Rectangle){ 0 };
    }
}

void GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user)
{
    const char *src_str = ({
        char *v = "";
		switch (source) {
		case GL_DEBUG_SOURCE_API: v = "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: v = "WINDOW SYSTEM"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: v = "SHADER COMPILER"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: v = "THIRD PARTY"; break;
		case GL_DEBUG_SOURCE_APPLICATION: v = "APPLICATION"; break;
		case GL_DEBUG_SOURCE_OTHER: v = "OTHER"; break;
		}
        v;
	});

	const char *type_str = ({
        char *v = "";
		switch (type) {
		case GL_DEBUG_TYPE_ERROR: v = "ERROR"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: v = "DEPRECATED_BEHAVIOR"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: v = "UNDEFINED_BEHAVIOR"; break;
		case GL_DEBUG_TYPE_PORTABILITY: v = "PORTABILITY"; break;
		case GL_DEBUG_TYPE_PERFORMANCE: v = "PERFORMANCE"; break;
		case GL_DEBUG_TYPE_MARKER: v = "MARKER"; break;
		case GL_DEBUG_TYPE_OTHER: v = "OTHER"; break;
		}
        v;
	});

	const char *severity_str = ({
        char *v = "";
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: v = "NOTIFICATION"; break;
		case GL_DEBUG_SEVERITY_LOW: v = "LOW"; break;
		case GL_DEBUG_SEVERITY_MEDIUM: v = "MEDIUM"; break;
		case GL_DEBUG_SEVERITY_HIGH: v = "HIGH"; break;
		}
        v;
	});
    printf("%s, %s, %s, %u: %s\n", src_str, type_str, severity_str, id, message);
}

NVAPI Element* WindowGetRootElement(Window *window)
{
    return &window->e;
}

NVAPI Font* WindowGetFontVariant(Window *window, FontVariant variant)
{
    return &window->fonts[variant];
}

NVAPI void SetMainWindow(Window *window)
{
    if(window)
        global.mainWindow = window;
}

NVAPI void WindowSetMenu(Window *window, Menu *menu)
{
    window->menu = menu;
}

NVAPI Element* WindowGetPressed(Window *window)
{
    return window->pressed;
}

NVAPI Element* WindowGetHovered(Window *window)
{
    return window->hovered;
}

NVAPI Element* WindowGetFocused(Window *window)
{
    return window->focused;
}

NVAPI void WindowSetFocused(Window *window, Element *element)
{
    WindowSetFocus(window, element);
}

NVAPI void WindowGetCursorPos(Window *window, int *x, int *y)
{
    *x = window->cursorX;
    *y = window->cursorY;
}
