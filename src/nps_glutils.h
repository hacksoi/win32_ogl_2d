struct render_objects
{
    uint32_t ShaderProgram;
    uint32_t Vbo, Ebo, Vao;
};

PFNGLATTACHSHADERPROC glAttachShader;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLCLEARCOLORPROC glClearColor;
PFNGLCLEARPROC glClear;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLDRAWELEMENTSPROC glDrawElements;
PFNGLDRAWARRAYSPROC glDrawArrays;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLUNIFORM1UIPROC glUniform1ui;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM2IPROC glUniform2i;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLVIEWPORTPROC glViewport;
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
PFNGLGENTEXTURESPROC glGenTextures;
PFNGLBINDTEXTUREPROC glBindTexture;
PFNGLTEXIMAGE2DPROC glTexImage2D;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
PFNGLTEXPARAMETERIPROC glTexParameteri;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLENABLEPROC glEnable;
PFNGLDISABLEPROC glDisable;
PFNGLFRONTFACEPROC glFrontFace;
PFNGLBLENDFUNCPROC glBlendFunc;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLLINEWIDTHPROC glLineWidth;
PFNGLBLENDEQUATIONPROC glBlendEquation;
PFNGLPOINTSIZEPROC glPointSize;
PFNGLPOLYGONMODEPROC glPolygonMode;

global char *BasicVertexShaderSource = R"STR(
#version 330 core
layout(location = 0) in vec2 Pos;

uniform vec2 WindowDimensions;

void
main()
{
    vec2 ClipPos = ((2.0f * Pos) / WindowDimensions) - 1.0f;
    gl_Position = vec4(ClipPos, 0.0, 1.0f);
}
)STR";

global char *BasicFragmentShaderSource = R"STR(
#version 330 core
out vec4 OutputColor;

void
main()
{
    OutputColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
)STR";

global char *BasicColorVertexShaderSource = R"STR(
#version 330 core
layout(location = 0) in vec2 Pos;
layout(location = 1) in vec4 vsColor;

uniform vec2 WindowDimensions;

out vec4 fsColor;

void
main()
{
    vec2 ClipPos = ((2.0f * Pos) / WindowDimensions) - 1.0f;
    gl_Position = vec4(ClipPos, 0.0, 1.0f);

    fsColor = vsColor;
}
)STR";

global char *BasicColorFragmentShaderSource = R"STR(
#version 330 core
in vec4 fsColor;

out vec4 OutputColor;

void
main()
{
    OutputColor = fsColor;
}
)STR";

/* Window Dimensions */
uint32_t npsWindowWidth, npsWindowHeight;

/* Render Objects */
render_objects BasicRenderObjects;
render_objects BasicColorRenderObjects;

/* Colors */
v4 NPS_RED = {1.0f, 0.0f, 0.0f, 1.0f};
v4 NPS_GREEN = {0.0f, 1.0f, 0.0f, 1.0f};
v4 NPS_BLUE = {0.0f, 0.0f, 1.0f, 1.0f};
v4 NPS_YELLOW = {1.0f, 1.0f, 0.0f, 1.0f};
v4 NPS_ORANGE = {1.0f, 0.63f, 0.48f, 1.0f};

#define LoadGLFunction(FunctionLowerCase, FunctionUpperCase) \
    FunctionLowerCase = (PFN ##FunctionUpperCase ##PROC)GetGLFunctionAddress(#FunctionLowerCase)
internal void *
GetGLFunctionAddress(const char *name)
{
    void *p = (void *)wglGetProcAddress(name);
    if(p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1))
    {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void *)GetProcAddress(module, name);

        if(p == 0)
        {
            Assert(0);
        }
    }

    return p;
}

internal void
LoadGLFunctions()
{
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)GetGLFunctionAddress("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)GetGLFunctionAddress("wglCreateContextAttribsARB");

    LoadGLFunction(glAttachShader, GLATTACHSHADER);
    LoadGLFunction(glBindBuffer, GLBINDBUFFER);
    LoadGLFunction(glBindVertexArray, GLBINDVERTEXARRAY);
    LoadGLFunction(glBufferData, GLBUFFERDATA);
    LoadGLFunction(glClearColor, GLCLEARCOLOR);
    LoadGLFunction(glClear, GLCLEAR);
    LoadGLFunction(glCompileShader, GLCOMPILESHADER);
    LoadGLFunction(glCreateProgram, GLCREATEPROGRAM);
    LoadGLFunction(glCreateShader, GLCREATESHADER);
    LoadGLFunction(glDeleteShader, GLDELETESHADER);
    LoadGLFunction(glDrawElements, GLDRAWELEMENTS);
    LoadGLFunction(glEnableVertexAttribArray, GLENABLEVERTEXATTRIBARRAY);
    LoadGLFunction(glGenBuffers, GLGENBUFFERS);
    LoadGLFunction(glGenVertexArrays, GLGENVERTEXARRAYS);
    LoadGLFunction(glGetProgramInfoLog, GLGETPROGRAMINFOLOG);
    LoadGLFunction(glGetProgramiv, GLGETPROGRAMIV);
    LoadGLFunction(glGetShaderInfoLog, GLGETSHADERINFOLOG);
    LoadGLFunction(glGetShaderiv, GLGETSHADERIV);
    LoadGLFunction(glGetUniformLocation, GLGETUNIFORMLOCATION);
    LoadGLFunction(glUniform1ui, GLUNIFORM1UI);
    LoadGLFunction(glUniform2f, GLUNIFORM2F);
    LoadGLFunction(glUniform2i, GLUNIFORM2I);
    LoadGLFunction(glLinkProgram, GLLINKPROGRAM);
    LoadGLFunction(glShaderSource, GLSHADERSOURCE);
    LoadGLFunction(glUseProgram, GLUSEPROGRAM);
    LoadGLFunction(glVertexAttribPointer, GLVERTEXATTRIBPOINTER);
    LoadGLFunction(glViewport, GLVIEWPORT);
    LoadGLFunction(glDrawArrays, GLDRAWARRAYS);
    LoadGLFunction(glGenTextures, GLGENTEXTURES);
    LoadGLFunction(glBindTexture, GLBINDTEXTURE);
    LoadGLFunction(glTexImage2D, GLTEXIMAGE2D);
    LoadGLFunction(glGenerateMipmap, GLGENERATEMIPMAP);
    LoadGLFunction(glTexParameteri, GLTEXPARAMETERI);
    LoadGLFunction(glUniformMatrix4fv, GLUNIFORMMATRIX4FV);
    LoadGLFunction(glFrontFace, GLFRONTFACE);
    LoadGLFunction(glEnable, GLENABLE);
    LoadGLFunction(glDisable, GLDISABLE);
    LoadGLFunction(glBlendFunc, GLBLENDFUNC);
    LoadGLFunction(glUniform1f, GLUNIFORM1F);
    LoadGLFunction(glLineWidth, GLLINEWIDTH);
    LoadGLFunction(glBlendEquation, GLBLENDEQUATION);
    LoadGLFunction(glPointSize, GLPOINTSIZE);
    LoadGLFunction(glPolygonMode, GLPOLYGONMODE);
}

typedef void opengl_get_error_info(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog); // glGetShaderInfoLog()
inline internal void
PrintOpenGLError(uint32_t OpenGLObject, opengl_get_error_info OpenGLGetErrorInfo, char *ExitMessage)
{
    size_t ExitMessageLength = strlen(ExitMessage);

    strcpy(GeneralBuffer, ExitMessage);
    OpenGLGetErrorInfo(OpenGLObject, (GLsizei)(sizeof(GeneralBuffer) - ExitMessageLength - 1), 
                       NULL, GeneralBuffer + ExitMessageLength);

    Log("%s\n", GeneralBuffer);
}

inline internal uint32_t
GetUniformLocation(uint32_t ShaderProgram, char *UniformName)
{
    uint32_t Result = glGetUniformLocation(ShaderProgram, UniformName);
    if(Result == -1)
    {
        Log("Could not find uniform: %s\n", UniformName);
    }

    return Result;
}

internal uint32_t
CreateShader(GLenum ShaderType, char *ShaderSource)
{
    uint32_t Shader = glCreateShader(ShaderType);
    glShaderSource(Shader, 1, &ShaderSource, NULL);
    glCompileShader(Shader);

    bool32 DidCompileSuccessfully;
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &DidCompileSuccessfully);
    if(!DidCompileSuccessfully)
    {
        char *ErrorMessage;
        switch(ShaderType)
        {
            case GL_VERTEX_SHADER:
            {
                ErrorMessage = "error compiling vertex shader: \n";
            } break;

            case GL_FRAGMENT_SHADER:
            {
                ErrorMessage = "error compiling fragment shader: \n";
            } break;

            case GL_GEOMETRY_SHADER:
            {
                ErrorMessage = "error compiling geometry shader: \n";
            } break;

            default:
            {
                ErrorMessage = "error compiling unknown shader type: \n";
            } break;
        }

        PrintOpenGLError(Shader, glGetShaderInfoLog, ErrorMessage);
        Shader = 0;

        Assert(0);
    }

    return Shader;
}

internal void
npsGluInit(uint32_t WindowWidth, uint32_t WindowHeight)
{
    npsWindowWidth = WindowWidth;
    npsWindowHeight = WindowHeight;

    /* BasicRenderObjects */
    {
        uint32_t VertexShader = CreateShader(GL_VERTEX_SHADER, BasicVertexShaderSource);
        uint32_t FragmentShader = CreateShader(GL_FRAGMENT_SHADER, BasicFragmentShaderSource);

        BasicRenderObjects.ShaderProgram = glCreateProgram();
        glAttachShader(BasicRenderObjects.ShaderProgram, VertexShader);
        glAttachShader(BasicRenderObjects.ShaderProgram, FragmentShader);
        glLinkProgram(BasicRenderObjects.ShaderProgram);

        bool32 DidProgramLinkSuccessfully;
        glGetProgramiv(BasicRenderObjects.ShaderProgram, GL_LINK_STATUS, &DidProgramLinkSuccessfully);
        if(!DidProgramLinkSuccessfully)
        {
            PrintOpenGLError(BasicRenderObjects.ShaderProgram, glGetProgramInfoLog, "Error linking shader program: \n");
            Assert(0);
        }

        glDeleteShader(VertexShader);
        glDeleteShader(FragmentShader);

        glGenBuffers(1, &BasicRenderObjects.Vbo);
        glBindBuffer(GL_ARRAY_BUFFER, BasicRenderObjects.Vbo);

        glGenVertexArrays(1, &BasicRenderObjects.Vao);
        glBindVertexArray(BasicRenderObjects.Vao);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
    }

    /* BasicColorRenderObjects */
    {
        uint32_t VertexShader = CreateShader(GL_VERTEX_SHADER, BasicColorVertexShaderSource);
        uint32_t FragmentShader = CreateShader(GL_FRAGMENT_SHADER, BasicColorFragmentShaderSource);

        BasicColorRenderObjects.ShaderProgram = glCreateProgram();
        glAttachShader(BasicColorRenderObjects.ShaderProgram, VertexShader);
        glAttachShader(BasicColorRenderObjects.ShaderProgram, FragmentShader);
        glLinkProgram(BasicColorRenderObjects.ShaderProgram);

        bool32 DidProgramLinkSuccessfully;
        glGetProgramiv(BasicColorRenderObjects.ShaderProgram, GL_LINK_STATUS, &DidProgramLinkSuccessfully);
        if(!DidProgramLinkSuccessfully)
        {
            PrintOpenGLError(BasicColorRenderObjects.ShaderProgram, glGetProgramInfoLog, "Error linking shader program: \n");
            Assert(0);
        }

        glDeleteShader(VertexShader);
        glDeleteShader(FragmentShader);

        glGenBuffers(1, &BasicColorRenderObjects.Vbo);
        glBindBuffer(GL_ARRAY_BUFFER, BasicColorRenderObjects.Vbo);

        glGenVertexArrays(1, &BasicColorRenderObjects.Vao);
        glBindVertexArray(BasicColorRenderObjects.Vao);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
}

internal void
DrawPoint(v2 P, v4 Color)
{
    float VertexData[] = {
        EXPANDV2(P), EXPANDV4(Color),
    };

    glBindBuffer(GL_ARRAY_BUFFER, BasicColorRenderObjects.Vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_DYNAMIC_DRAW);

    glPointSize(7.5f);

    glUseProgram(BasicColorRenderObjects.ShaderProgram);
    glUniform2f(GetUniformLocation(BasicColorRenderObjects.ShaderProgram, "WindowDimensions"), (float)npsWindowWidth, (float)npsWindowHeight);
    glBindVertexArray(BasicColorRenderObjects.Vao);
    glDrawArrays(GL_POINTS, 0, 1);
}

internal void
DrawRay(ray2 Ray, float Length, v4 Color)
{
    line2 Line = {Ray.Position, Ray.Position + Length*Ray.Direction};

    float VertexData[] = {
        EXPANDV2(Line.P1), EXPANDV4(Color),
        EXPANDV2(Line.P2), EXPANDV4(Color),
    };

    glBindBuffer(GL_ARRAY_BUFFER, BasicColorRenderObjects.Vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_DYNAMIC_DRAW);

    glUseProgram(BasicColorRenderObjects.ShaderProgram);
    glUniform2f(GetUniformLocation(BasicColorRenderObjects.ShaderProgram, "WindowDimensions"), (float)npsWindowWidth, (float)npsWindowHeight);
    glBindVertexArray(BasicColorRenderObjects.Vao);
    glDrawArrays(GL_LINES, 0, 2);
}
