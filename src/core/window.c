#include "nvui/window.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nvui/element.h"
#include "nvui/private/window.h"
#include "nvui/private/painter.h"
#include "nvui/resources.h"
#include "nvui/glutils.h"
#include "nvui/keys.h"

#define WINDOW_MIN_WIDTH 200
#define WINDOW_MIN_HEIGHT 200
#define BUFFER_SIZE 1024 * 64 //64k vertices as a draw buffer. that should support complex drawing operations

typedef struct GlobalState
{
    Window **windows;
    size_t windowCount;
    Window *mainWindow;

#ifdef __linux__
    Display *display;
    Atom windowClosedID;
#endif
} GlobalState;

GlobalState global = {};
static bool glFuncsLoaded = false;

static bool InitGLData(Window *window)
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

static void DestroyGLData(Window *window)
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

static void ResizeTextures(Window *window)
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

static int WindowMessage(Element *element, Message message, int di, void *dp)
{
    if(message == MSG_LAYOUT)
    {
        element->window->projection = glms_ortho(0, element->window->width, 0, element->window->height, -1, 1);
        element->window->buffersNeedResize = true;
        if(element->childCount)
        {
            ElementMove(element->children[0], element->bounds, false);
            ElementRepaint(element, NULL);
        }
    }
    return 0;
}

void InvalidateWindow(Window *window);

static void WindowSetPressed(Window *window, Element *element, int button)
{
    Element *previous = window->pressed;

    window->pressed = element;
    window->pressedButton = button;
    if(element)
        window->focused = element;

    if(previous) ElementMessage(previous, MSG_UPDATE, UPDATE_PRESSED, NULL);
    if(element) ElementMessage(element, MSG_UPDATE, UPDATE_PRESSED, NULL);
}

static void WindowInputEvent(Window *window, Message message, int di, void *dp)
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
            ElementMessage(window->pressed, MSG_UPDATE, UPDATE_HOVERED, NULL);
        }
        else if(!inside && window->hovered == window->pressed)
        {
            window->hovered = &window->e;
            ElementMessage(window->pressed, MSG_UPDATE, UPDATE_HOVERED, NULL);
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
            ElementMessage(previous, MSG_UPDATE, UPDATE_HOVERED, NULL);
            ElementMessage(window->hovered, MSG_UPDATE, UPDATE_HOVERED, NULL);
        }
    }

    InvalidateWindow(window);
}

static void WindowKeyEvent(Window *window, Message message, int di, void *dp)
{
    if(window->focused)
    {
        ElementMessage(window->focused, message, di, dp);
    }
}

static void WindowCharEvent(Window *window, Message message, int di, void *dp)
{
    if(window->focused)
    {
        di = di == '\r' ? '\n' : di;
        ElementMessage(window->focused, message, di, dp);
    }
}

static void WindowEndPaint(Window *window, Painter *painter);

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

static void RemoveWindow(Window *window)
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

static void Update(Window *window)
{
    if(ElementDestroyNow(window->e.children[0]))
    {
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

static void GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user)
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

#ifdef _WIN32
#define WINDOW_CLASS "NVWINDOW"

static bool LoadPreGLFunctions(void)
{
    HWND dummy = CreateWindowExW(
        0, L"STATIC", L"DummyWindow", WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, NULL, NULL);
    if(!dummy) return false;

    HDC dc = GetDC(dummy);
    if(!dc) return false;

    PIXELFORMATDESCRIPTOR pfd = 
    {
        .nSize = sizeof pfd,
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 24
    };

    int format = ChoosePixelFormat(dc, &pfd);
    if(!format) return false;

    int ok = DescribePixelFormat(dc, format, sizeof pfd, &pfd);
    if(!ok) return false;

    if(!SetPixelFormat(dc, format, &pfd)) return false;

    HGLRC rc = wglCreateContext(dc);
    if(!rc) return false;

    ok = wglMakeCurrent(dc, rc);
    if(!ok) return false;

    // I assume that every computer in todays age should support the required wgl extension to create a modern opengl context
    gladLoaderLoadWGL(dc);

    if(!GLAD_WGL_ARB_create_context || !GLAD_WGL_ARB_create_context_profile || !GLAD_WGL_ARB_pixel_format || !GLAD_WGL_EXT_swap_control)
    {
        fprintf(stderr, "This system does not support the required wgl extensions\n");
        return false;
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(rc);
    ReleaseDC(dummy, dc);
    DestroyWindow(dummy);

    return true;
}

static bool LoadGLFunctions(void)
{
    if(!glFuncsLoaded)
    {
        int ret = gladLoadGL((GLADloadfunc)wglGetProcAddress);
        if(ret) glFuncsLoaded = true;
        else return ret;
    }

    glEnable(GL_SCISSOR_TEST);

#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLDebugCallback, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif

    return true;
}

static void WindowEndPaint(Window *window, Painter *painter)
{
}

static Keycode TranslateKey(WPARAM wParam)
{
    if(wParam == VK_LEFT) return KEY_LEFT;
    if(wParam == VK_RIGHT) return KEY_RIGHT;
    return KEY_NONE;
}

void InvalidateWindow(Window *window)
{
    InvalidateRect(window->hwnd, NULL, false);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Window *window = (Window*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    if(!window) return DefWindowProcA(hwnd, message, wParam, lParam);

    switch(message)
    {
    case WM_CLOSE:
        {
            DestroyWindow(hwnd);
        }
        break;
    case WM_DESTROY:
        {
            bool exit = global.windowCount == 1;
            if(global.mainWindow == window)
            {
                exit = true;
                for(size_t i = 0; i < global.windowCount; ++i)
                {
                    ShowWindow(global.windows[i]->hwnd, SW_HIDE);
                }

                for(size_t i = 0; i < global.windowCount; ++i)
                {
                    if(global.windows[i] == window) continue;
                    DestroyWindow(global.windows[i]->hwnd);
                    i--;
                }
            }

            ElementDestroy(&window->e);
            
            wglMakeCurrent(window->hdc, window->hglrc);
            Update(window);

            DestroyGLData(window);
            wglDeleteContext(window->hglrc);

            if(exit)
                PostQuitMessage(0);
        }
        break;
    case WM_SIZE:
        {
            RECT client;
            GetClientRect(hwnd, &client);
            if(client.right > 0 && client.bottom > 0)
            {
                window->width = client.right;
                window->height = client.bottom;
                window->e.bounds = (Rectangle){ .r = window->width, .b = window->height };
                window->e.clip = (Rectangle){ .r = window->width, .b = window->height };
                ElementMessage(&window->e, MSG_LAYOUT, 0, NULL);
                //InvalidateRect(element->window->hwnd, NULL, false);
            }
        }
        break;
    case WM_MOUSEMOVE:
        {
            if(!window->trackingLeave)
            {
                window->trackingLeave = true;
                TRACKMOUSEEVENT leave = 
                {
                    .cbSize = sizeof leave,
                    .dwFlags = TME_LEAVE,
                    .hwndTrack = window->hwnd
                };
                TrackMouseEvent(&leave);
            }

            POINT cursor = {};
            GetCursorPos(&cursor);
            ScreenToClient(window->hwnd, &cursor);
            if(!(window->prevCursorX == cursor.x && window->prevCursorY == cursor.y))
            {
                window->prevCursorX = cursor.x;
                window->prevCursorY = cursor.y;
                window->cursorX = cursor.x;
                window->cursorY = cursor.y;
                WindowInputEvent(window, MSG_MOUSE_MOVE, 0, NULL);
            }
        }
        break;
    case WM_MOUSELEAVE:
        {
            window->trackingLeave = false;

            if(!window->pressed)
            {
                window->cursorX = -1;
                window->cursorY = -1;
            }

            WindowInputEvent(window, MSG_MOUSE_MOVE, 0, NULL);
        }
        break;
    case WM_LBUTTONDOWN:
        {
            SetCapture(window->hwnd);
            WindowInputEvent(window, MSG_LEFT_DOWN, 0, NULL);
        }
        break;
    case WM_LBUTTONUP:
        {
            if(window->pressedButton == 1) ReleaseCapture();
            WindowInputEvent(window, MSG_LEFT_UP, 0, NULL);
        }
        break;
    case WM_MBUTTONDOWN:
        {
            SetCapture(window->hwnd);
            WindowInputEvent(window, MSG_MIDDLE_DOWN, 0, NULL);
        }
        break;
    case WM_MBUTTONUP:
        {
            if(window->pressedButton == 2) ReleaseCapture();
            WindowInputEvent(window, MSG_MIDDLE_UP, 0, NULL);
        }
        break;
    case WM_RBUTTONDOWN:
        {
            SetCapture(window->hwnd);
            WindowInputEvent(window, MSG_RIGHT_DOWN, 0, NULL);
        }
        break;
    case WM_RBUTTONUP:
        {
            if(window->pressedButton == 3) ReleaseCapture();
            WindowInputEvent(window, MSG_RIGHT_UP, 0, NULL);
        }
        break;
    case WM_KEYDOWN:
        {
            int key = TranslateKey(wParam);
            WindowKeyEvent(window, MSG_KEY_DOWN, key, NULL);
        }
        break;
    case WM_KEYUP:
        {
            int key = TranslateKey(wParam);
            WindowKeyEvent(window, MSG_KEY_UP, key, NULL);
        }
        break;
    case WM_CHAR:
        {
            WindowCharEvent(window, MSG_CHAR, wParam, NULL);
        }
        break;
    case WM_UNICHAR:
        {
            
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT paint;
            BeginPaint(hwnd, &paint);
            wglMakeCurrent(window->hdc, window->hglrc);
            if(window->buffersNeedResize)
            {
                ResizeTextures(window);
                window->buffersNeedResize = false;
            }
            Update(window);
            
            glDisable(GL_SCISSOR_TEST);
            glClearNamedFramebufferfv(0, GL_COLOR, 0, (float[]){ 0, 1, 1, 1 });
            glBlitNamedFramebuffer(window->glData.framebuffer, 0, 0, 0, window->width, window->height, 0, window->height, window->width, 0, GL_COLOR_BUFFER_BIT, GL_LINEAR);
            SwapBuffers(window->hdc);
            glEnable(GL_SCISSOR_TEST);
            EndPaint(hwnd, &paint);
        }
        break;
    case WM_GETMINMAXINFO:
        {
            RECT windowRect = { .right = WINDOW_MIN_WIDTH, .bottom = WINDOW_MIN_HEIGHT };
            AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);
            MINMAXINFO *mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = windowRect.right - windowRect.left;
            mmi->ptMinTrackSize.y = windowRect.bottom - windowRect.top;
        }
    default:
        return DefWindowProcA(hwnd, message, wParam, lParam);
    }

    return 0;
}

NVAPI void Initialize(void)
{
    LoadPreGLFunctions();

    WNDCLASSEXA windowClass = 
    {
        .cbSize = sizeof windowClass,
        .lpfnWndProc = WndProc,
        .hCursor = LoadCursorA(NULL, IDC_ARROW),
        .hIcon = LoadIconA(NULL, IDI_APPLICATION),
        .hInstance = GetModuleHandleA(NULL),
        .style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
        .lpszClassName = WINDOW_CLASS
    };
    RegisterClassExA(&windowClass);
}

NVAPI Window* WindowCreate(const char *title, int width, int height)
{
    Window *window = (Window*)ElementCreate(sizeof *window, NULL, 0, WindowMessage);
    if(!window) return NULL;

    window->e.window = window;
    window->hovered = &window->e;

    global.windowCount++;
    global.windows = realloc(global.windows, sizeof(Window*) * global.windowCount);
    global.windows[global.windowCount - 1] = window;
    if(!global.mainWindow)
        global.mainWindow = window;

    RECT windowRect = { .right = width, .bottom = height };
    AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);

    window->hwnd = CreateWindowExA(WS_EX_OVERLAPPEDWINDOW, WINDOW_CLASS, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, GetModuleHandleA(NULL), NULL);
    if(!window->hwnd) return NULL;
    SetWindowLongPtrA(window->hwnd, GWLP_USERDATA, (LONG_PTR)window);

    window->hdc = GetDC(window->hwnd);
    if(!window->hdc) return NULL;

    int pixelattrib[] = 
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 24,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,

        //WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,

        //WGL_SAMPLE_BUFFERS_ARB, 1,
        //WGL_SAMPLES_ARB, 4, // 4x MSAA

        0
    };

    int format;
    UINT formats;
    if(!wglChoosePixelFormatARB(window->hdc, pixelattrib, NULL, 1, &format, &formats)) return NULL;

    PIXELFORMATDESCRIPTOR pfd = { .nSize = sizeof pfd };
    int ok = DescribePixelFormat(window->hdc, format, sizeof pfd, &pfd);
    if(!ok) return NULL;

    if(!SetPixelFormat(window->hdc, format, &pfd)) return NULL;

    int glattrib[] = 
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#if _DEBUG
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
        0
    };

    window->hglrc = wglCreateContextAttribsARB(window->hdc, NULL, glattrib);
    if(!window->hglrc) return NULL;

    ok = wglMakeCurrent(window->hdc, window->hglrc);
    if(!ok) return NULL;

    wglSwapIntervalEXT(1);

    if(!LoadGLFunctions())
        return NULL;

    if(!InitGLData(window))
        return NULL;

    ShowWindow(window->hwnd, SW_SHOW);
    PostMessageA(window->hwnd, WM_SIZE, 0, 0);

    return window;
}

NVAPI int MessageLoop(void)
{
    MSG message = {};

    while(GetMessageA(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    free(global.windows);

    return message.wParam;
}

#elif defined __linux__
#include <string.h>

static Window* FindWindow(X11Window window)
{
    for(size_t i = 0; i < global.windowCount; ++i)
    {
        if(global.windows[i]->window == window)
            return global.windows[i];
    }
    return NULL;
}

static void WindowEndPaint(Window *window, Painter *painter)
{
}

void InvalidateWindow(Window *window)
{
    XClearArea(global.display, window->window, 0, 0, 1, 1, true);
    XFlush(global.display);
}

static bool LoadGLFunctions(void)
{
    if(!glFuncsLoaded)
    {
        int ret = gladLoadGL((GLADloadfunc)glXGetProcAddress);
        if(ret) glFuncsLoaded = true;
        else return ret;
    }

    glEnable(GL_SCISSOR_TEST);

#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLDebugCallback, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif

    return true;
}

NVAPI void Initialize(void)
{
    global.display = XOpenDisplay(NULL);
    global.windowClosedID = XInternAtom(global.display, "WM_DELETE_WINDOW", 0);
}

NVAPI Window* WindowCreate(const char *title, int width, int height)
{
    Window *window = (Window*)ElementCreate(sizeof *window, NULL, 0, WindowMessage);
    if(!window) return NULL;

    window->e.window = window;
    window->hovered = &window->e;

    window->width = width;
    window->height = height;

    global.windowCount++;
    global.windows = realloc(global.windows, sizeof(Window*) * global.windowCount);
    global.windows[global.windowCount - 1] = window;
    if(!global.mainWindow)
        global.mainWindow = window;

    int screenId = DefaultScreen(global.display);

    // I assume that every computer in todays age should support the required glx extension to create a modern opengl context
    gladLoaderLoadGLX(global.display, screenId);

    GLint glxAttribs[] =
    {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        
        //GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, True,

        //GLX_SAMPLE_BUFFERS, True,
        //GLX_SAMPLES, 8,
        None
    };
    int fbcount;
    GLXFBConfig *fbc = glXChooseFBConfig(global.display, screenId, glxAttribs, &fbcount);
    if(!fbc) return NULL;

    window->visual = glXGetVisualFromFBConfig(global.display, fbc[0]);
    if(!window->visual) return NULL;

    XSetWindowAttributes windowAttrib = {};
    windowAttrib.bit_gravity = StaticGravity;
    windowAttrib.border_pixel = BlackPixel(global.display, screenId);
    windowAttrib.background_pixel = WhitePixel(global.display, screenId);
    windowAttrib.override_redirect = True;
    windowAttrib.colormap = XCreateColormap(global.display, DefaultRootWindow(global.display), window->visual->visual, AllocNone);
    window->colormap = windowAttrib.colormap;

    window->window = XCreateWindow(global.display, DefaultRootWindow(global.display), 0, 0, width, height, 0, window->visual->depth, InputOutput, window->visual->visual, CWBackingPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttrib);
    XStoreName(global.display, window->window, title);
    XSelectInput(global.display, window->window, SubstructureNotifyMask | ExposureMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask | ButtonMotionMask | KeymapStateMask | FocusChangeMask | PropertyChangeMask);

    XSetWMProtocols(global.display, window->window, &global.windowClosedID, 1);
    XMapRaised(global.display, window->window);

    XSizeHints* sizeHints = XAllocSizeHints();
    sizeHints->flags = PMinSize;
    sizeHints->min_width = WINDOW_MIN_WIDTH;
    sizeHints->min_height = WINDOW_MIN_HEIGHT;
    XSetWMNormalHints(global.display, window->window, sizeHints);
    XFree(sizeHints);

    int contextAttribs[] = 
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 6,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifdef _DEBUG
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
#endif

        None
    };

    window->context = glXCreateContextAttribsARB(global.display, fbc[0], NULL, true, contextAttribs);
    if(!window->context) return NULL;
    if(!glXIsDirect(global.display, window->context)) return NULL;

    glXMakeCurrent(global.display, window->window, window->context);

    glXSwapIntervalEXT(global.display, window->window, 1);

    if(!LoadGLFunctions())
        return NULL;

    if(!InitGLData(window))
        return NULL;

    window->firstTimeLayout = true;

    return window;
}

#if 0
static void CloseWindow(Window *window)
{
    long mask = SubstructureRedirectMask | SubstructureNotifyMask;

    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.message_type = global.windowClosedID;
    event.xclient.window = window->window;
    event.xclient.format = 32;
    event.xclient.data.l[0] = 0;
    event.xclient.data.l[1] = 0;
    event.xclient.data.l[2] = 0;
    event.xclient.data.l[3] = 0;
    event.xclient.data.l[4] = 0;

    XSendEvent(global.display, DefaultRootWindow(global.display), False, mask, &event);
    XSync(global.display, False);
}
#endif

static Keycode TranslateKey(unsigned int xkey)
{
    if(xkey == XKeysymToKeycode(global.display, XK_Left))
        return KEY_LEFT;
    else if(xkey == XKeysymToKeycode(global.display, XK_Right))
        return KEY_RIGHT;
    return KEY_NONE;
}

static void CloseWindow(Window *window)
{
    ElementDestroy(&window->e);
                    
    glXMakeCurrent(global.display, window->window, window->context);
    Update(window);

    DestroyGLData(window);
    glXDestroyContext(global.display, window->context);
    
    XFree(window->visual);
    XFreeColormap(global.display, window->colormap);
    XDestroyWindow(global.display, window->window);
}

NVAPI int MessageLoop(void)
{
    bool isRunning = true;
    while(isRunning)
    {
        XEvent event;
        XNextEvent(global.display, &event);

        switch(event.type)
        {
        case Expose:
            {
                Window *window = FindWindow(event.xexpose.window);
                if(!window) continue;

                glXMakeCurrent(global.display, window->window, window->context);
                if(window->firstTimeLayout)
                {
                    window->e.bounds = (Rectangle){ .r = window->width, .b = window->height };
                    window->e.clip = (Rectangle){ .r = window->width, .b = window->height };

                    ElementMessage(&window->e, MSG_LAYOUT, 0, NULL);

                    window->firstTimeLayout = false;
                }

                if(window->buffersNeedResize)
                {
                    ResizeTextures(window);
                    window->buffersNeedResize = false;
                }

                Update(window);
                glDisable(GL_SCISSOR_TEST);
                glClearNamedFramebufferfv(0, GL_COLOR, 0, (float[]){ 0, 1, 0, 1 });
                glBlitNamedFramebuffer(window->glData.framebuffer, 0, 0, 0, window->width, window->height, 0, window->height, window->width, 0, GL_COLOR_BUFFER_BIT, GL_LINEAR);
                glXSwapBuffers(global.display, window->window);
                glEnable(GL_SCISSOR_TEST);
            }
            break;
        case ConfigureNotify:
            {
                Window *window = FindWindow(event.xconfigure.window);
                if(!window) continue;

                if(window->width != event.xconfigure.width || window->height != event.xconfigure.height)
                {
                    window->width = event.xconfigure.width;
                    window->height = event.xconfigure.height;

                    window->e.bounds = (Rectangle){ .r = window->width, .b = window->height };
                    window->e.clip = (Rectangle){ .r = window->width, .b = window->height };

                    ElementMessage(&window->e, MSG_LAYOUT, 0, NULL);
                }
            }
            break;
        case MotionNotify:
            {
                Window *window = FindWindow(event.xmotion.window);
                if(!window) continue;
                window->cursorX = event.xmotion.x;
                window->cursorY = event.xmotion.y;
                WindowInputEvent(window, MSG_MOUSE_MOVE, 0, NULL);
            }
            break;
        case LeaveNotify:
            {
                Window *window = FindWindow(event.xcrossing.window);
                if(!window) continue;

                if(!window->pressed)
                {
                    window->cursorX = -1;
                    window->cursorY = -1;
                }
                
                WindowInputEvent(window, MSG_MOUSE_MOVE, 0, NULL);
            }
            break;
        case ButtonPress:
        case ButtonRelease:
            {
                Window *window = FindWindow(event.xbutton.window);
                if(!window) continue;
                window->cursorX = event.xbutton.x;
                window->cursorY = event.xbutton.y;

                if(event.xbutton.button >= 1 && event.xbutton.button <= 3)
                {
                    WindowInputEvent(window, (Message)((event.type == ButtonPress ? MSG_LEFT_DOWN : MSG_LEFT_UP) + event.xbutton.button * 2 - 2), 0, NULL);
                }
            }
            break;
        case KeyPress:
        case KeyRelease:
            {
                Window *window = FindWindow(event.xkey.window);
                if(!window) continue;

                Keycode key = TranslateKey(event.xkey.keycode);
                WindowKeyEvent(window, event.type == KeyPress ? MSG_KEY_DOWN : MSG_KEY_UP, key, NULL);

                if(event.type == KeyPress)
                {
                    char buffer[16] = {};
                    XLookupString(&event.xkey, buffer, sizeof buffer, NULL, NULL);
                    WindowCharEvent(window, MSG_CHAR, buffer[0], NULL);
                }
            }
            break;
        case ClientMessage:
            {
                Window *window = FindWindow(event.xclient.window);
                if(!window) continue;
                if(event.xclient.data.l[0] == global.windowClosedID)
                {
                    bool exit = global.windowCount == 1;
                    if(global.mainWindow == window)
                    {
                        exit = true;
                        for(size_t i = 0; i < global.windowCount; ++i)
                        {
                            if(global.windows[i] == window) continue;
                            CloseWindow(global.windows[i]);
                            i--;
                        }
                    }

                    CloseWindow(window);
                    
                    if(exit)
                        isRunning = false;
                }
            }
            break;
#if 0
        case DestroyNotify:
            {
                isRunning = false;
            }
            break;
#endif
        }
    }

    free(global.windows);

    XCloseDisplay(global.display);
    return 0;
}

#endif
