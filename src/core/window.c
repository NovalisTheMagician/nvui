#include "nvui/window.h"

#include <X11/Xlib.h>
#include <stdlib.h>

#include "nvui/element.h"
#include "nvui/painter.h"

#include <stdio.h>

#define WINDOW_MIN_WIDTH 200
#define WINDOW_MIN_HEIGHT 200
#define BUFFER_SIZE 1024 * 64 //64k vertices as a draw buffer. that should support complex drawing operations

GlobalState global = {};
static bool glFuncsLoaded = false;
static bool buffersNeedResize = true;

static bool CompileShader(const char *shaderScr, GLuint *shader)
{
    int success;
    char infoLog[512];

    glShaderSource(*shader, 1, &shaderScr, NULL);
    glCompileShader(*shader);
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(*shader, sizeof infoLog, NULL, infoLog);
        printf("Failed to compile Shader: %s\n", infoLog);
        return false;
    };

    return true;
}

static bool LinkProgram(GLuint vertexShader, GLuint fragmentShader, GLuint *program)
{
    int success;
    char infoLog[512];

    glAttachShader(*program, vertexShader);
    glAttachShader(*program, fragmentShader);
    glLinkProgram(*program);
    glGetProgramiv(*program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(*program, sizeof infoLog, NULL, infoLog);
        printf("Failed to link Program: %s\n", infoLog);
        return false;
    }

    return true;
}

static bool InitGLData(Window *window)
{
    const char *vertShaderSrc = 
        "#version 460 core\n"
        "layout(location=0) in vec2 inPosition;\n"
        "layout(location=1) in vec2 inTexCoords;\n"
        "layout(location=2) in vec4 inColor;\n"
        "out vec4 outColor;\n"
        "out vec2 outTexCoords;\n"
        "uniform mat4 viewProj;\n"
        "uniform vec2 coordOffset;\n"
        "void main() {\n"
        "   gl_Position = viewProj * vec4(inPosition, 0, 1);\n"
        "   outColor = inColor;\n"
        "   outTexCoords = inTexCoords;\n"
        "}\n";

    const char *fragShaderSrc = 
        "#version 460 core\n"
        "in vec4 outColor;\n"
        "in vec2 outTexCoords;\n"
        "out vec4 fragColor;\n"
        "uniform sampler2D tex;\n"
        "uniform vec4 tint;\n"
        "void main() {\n"
        "   fragColor = outColor * texture(tex, outTexCoords) * tint;\n"
        "}\n";

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    if(!CompileShader(vertShaderSrc, &vertShader))
        return false;

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(!CompileShader(fragShaderSrc, &fragShader))
        return false;

    GLuint program = glCreateProgram();
    if(!LinkProgram(vertShader, fragShader, &program))
        return false;

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    window->glData.shaderProgram = program;
    window->glData.projectionLoc = glGetUniformLocation(program, "viewProj");
    window->glData.tintLoc = glGetUniformLocation(program, "tint");
    window->glData.textureLoc = glGetUniformLocation(program, "tex");

    const uint8_t whiteBytes[] = { 255, 255, 255, 255 };
    glCreateTextures(GL_TEXTURE_2D, 1, &window->glData.whiteTexture);
    glTextureStorage2D(window->glData.whiteTexture, 1, GL_RGBA8, 1, 1);
    glTextureParameteri(window->glData.whiteTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(window->glData.whiteTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureSubImage2D(window->glData.whiteTexture, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, whiteBytes);

    const GLbitfield 
	mapping_flags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT,
	storage_flags = GL_DYNAMIC_STORAGE_BIT | mapping_flags;
    const size_t bufferSize = BUFFER_SIZE * sizeof(struct Vertex);

    glCreateBuffers(1, &window->glData.vertexBuffer);
    glNamedBufferStorage(window->glData.vertexBuffer, bufferSize, NULL, storage_flags);
    window->glData.mappedVertexBuffer = glMapNamedBufferRange(window->glData.vertexBuffer, 0, bufferSize, mapping_flags);

    glCreateVertexArrays(1, &window->glData.vertexFormat);
    glEnableVertexArrayAttrib(window->glData.vertexFormat, 0);
    glEnableVertexArrayAttrib(window->glData.vertexFormat, 1);
    glEnableVertexArrayAttrib(window->glData.vertexFormat, 2);
    glVertexArrayAttribFormat(window->glData.vertexFormat, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
    glVertexArrayAttribFormat(window->glData.vertexFormat, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texcoord));
    glVertexArrayAttribFormat(window->glData.vertexFormat, 2, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
    glVertexArrayAttribBinding(window->glData.vertexFormat, 0, 0);
    glVertexArrayAttribBinding(window->glData.vertexFormat, 1, 0);
    glVertexArrayAttribBinding(window->glData.vertexFormat, 2, 0);
    glVertexArrayVertexBuffer(window->glData.vertexFormat, 0, window->glData.vertexBuffer, 0, sizeof(Vertex));

    return true;
}

static void DestroyGLData(Window *window)
{
    glDeleteFramebuffers(1, &window->glData.framebuffer);
    GLuint renderbuffers[] = { window->glData.colorRb, window->glData.depthRb };
    glDeleteRenderbuffers(2, renderbuffers);
    glDeleteTextures(1, &window->glData.whiteTexture);
    glDeleteProgram(window->glData.shaderProgram);
    glUnmapNamedBuffer(window->glData.vertexBuffer);
    glDeleteBuffers(1, &window->glData.vertexBuffer);
    glDeleteVertexArrays(1, &window->glData.vertexFormat);
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
        buffersNeedResize = true;
        if(element->childCount)
        {
            ElementMove(element->children[0], element->bounds, false);
            ElementRepaint(element, NULL);
        }
    }
    return 0;
}

static void WindowEndPaint(Window *window, Painter *painter);

static void ElementPaint(Element *element, Painter *painter)
{
    Rectangle clip = RectangleIntersection(element->clip, painter->clip);
    if(!RectangleValid(clip)) return;

    const GLData gldata = element->window->glData;

    painter->clip = clip;
    glScissor(clip.l, clip.t, clip.r - clip.l, clip.b - clip.t);
    
    glBindTextureUnit(0, gldata.whiteTexture);
    glUniform1i(gldata.textureLoc, 0);
    glUniform4fv(gldata.tintLoc, 1, (float[]){ 1, 1, 1, 1 });

    painter->vertexMap = gldata.mappedVertexBuffer;
    painter->textureLoc = gldata.textureLoc;
    painter->tintLoc = gldata.tintLoc;
    painter->defaultTexture = gldata.whiteTexture;

    ElementMessage(element, MSG_PAINT, 0, painter);

    for(size_t i = 0; i < element->childCount; ++i)
    {
        painter->clip = clip;
        ElementPaint(element->children[i], painter);
    }
}

static void Update(Window *window)
{
    if(RectangleValid(window->updateRegion))
    {
        const GLData gldata = window->glData;

        Painter painter = {};
        painter.width = window->width;
        painter.height = window->height;
        painter.clip = RectangleIntersection((Rectangle){ .r = window->width, .b = window->height }, window->updateRegion);
        painter.framebuffer = gldata.framebuffer;

        glBindFramebuffer(GL_FRAMEBUFFER, gldata.framebuffer);
        glBindVertexArray(gldata.vertexFormat);
        glVertexArrayVertexBuffer(gldata.vertexFormat, 0, gldata.vertexBuffer, 0, sizeof(Vertex));
        glUseProgram(gldata.shaderProgram);
        glUniformMatrix4fv(gldata.projectionLoc, 1, GL_FALSE, (float*)&window->projection.raw);

        ElementPaint(&window->e, &painter);
        WindowEndPaint(window, &painter);
        window->updateRegion = (Rectangle){ 0 };
    }
}

static void RemoveAllElements(Element *element)
{
    for(size_t i = 0; i < element->childCount; ++i)
        RemoveAllElements(element->children[i]);

    if(element->childCount)
        free(element->children);
    free(element);
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
    case WM_PAINT:
        {
            PAINTSTRUCT paint;
            BeginPaint(hwnd, &paint);
            wglMakeCurrent(window->hdc, window->hglrc);
            if(buffersNeedResize)
            {
                ResizeTextures(window);
                buffersNeedResize = false;
            }
            Update(window);
            
            glDisable(GL_SCISSOR_TEST);
            glClearNamedFramebufferfv(0, GL_COLOR, 0, (float[]){ 0, 1, 1, 1 });
            glBlitNamedFramebuffer(window->glData.framebuffer, 0, 0, 0, window->width, window->height, 0, 0, window->width, window->height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
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

static void _DestroyWindow(Window *window)
{
    wglMakeCurrent(window->hdc, window->hglrc);
    if(window->e.childCount)
    {
        RemoveAllElements(window->e.children[0]);
        free(window->e.children);
    }

    DestroyGLData(window);

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(window->hglrc);
    DestroyWindow(window->hwnd);
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

    global.windowCount++;
    global.windows = realloc(global.windows, sizeof(Window*) * global.windowCount);
    global.windows[global.windowCount - 1] = window;

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

    //HGLRC shareCtx = global.windowCount > 0 ? global.windows[0]->hglrc : NULL;
    window->hglrc = wglCreateContextAttribsARB(window->hdc, NULL, glattrib);
    if(!window->hglrc) return NULL;

    ok = wglMakeCurrent(window->hdc, window->hglrc);
    if(!ok) return NULL;

    wglSwapIntervalEXT(1);

    LoadGLFunctions();

    InitGLData(window);

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

    for(size_t i = 0; i < global.windowCount; ++i)
    {
        _DestroyWindow(global.windows[i]);
        free(global.windows[i]);
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

static void DestroyWindow(Window *window)
{
    if(window->e.childCount)
    {
        RemoveAllElements(window->e.children[0]);
        free(window->e.children);
    }

    DestroyGLData(window);

    glXMakeCurrent(global.display, 0, NULL);
    glXDestroyContext(global.display, window->context);
    XFree(window->visual);
    XFreeColormap(global.display, window->colormap);
    XDestroyWindow(global.display, window->window);
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

    window->width = width;
    window->height = height;

    global.windowCount++;
    global.windows = realloc(global.windows, sizeof(Window*) * global.windowCount);
    global.windows[global.windowCount - 1] = window;

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

    LoadGLFunctions();

    InitGLData(window);

    window->firstTimeLayout = true;

    return window;
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

                if(buffersNeedResize)
                {
                    ResizeTextures(window);
                    buffersNeedResize = false;
                }

                Update(window);
                glDisable(GL_SCISSOR_TEST);
                glClearNamedFramebufferfv(0, GL_COLOR, 0, (float[]){ 0, 1, 0, 1 });
                glBlitNamedFramebuffer(window->glData.framebuffer, 0, 0, 0, window->width, window->height, 0, 0, window->width, window->height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
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
                    //XClearArea(global.display, window->window, 0, 0, 1, 1, true);
                    //XFlush(global.display);
                }
            }
            break;
        case ClientMessage:
            {
                Window *window = FindWindow(event.xclient.window);
                if(!window) continue;
                if(event.xclient.data.l[0] == global.windowClosedID)
                {
                    isRunning = false;
                }
            }
            break;
        case DestroyNotify:
            {
                isRunning = false;
            }
            break;
        }
    }

    for(size_t i = 0; i < global.windowCount; ++i)
    {
        DestroyWindow(global.windows[i]);
        free(global.windows[i]);
    }
    free(global.windows);

    XCloseDisplay(global.display);
    return 0;
}

#endif
