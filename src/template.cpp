//#define OGL_BASIC
#define OGL_TEXTURE

/* Basic shaders. */
#ifdef OGL_BASIC
global char *VertexShaderSource = R"STR(
#version 330 core
layout (location = 0) in vec2 Pos;

uniform ivec2 WindowDimensions;

void
main()
{
    vec2 ClipPos = ((2.0f * Pos) / WindowDimensions) - 1.0f;
    gl_Position = vec4(ClipPos, 0.0, 1.0);
}
)STR";

global char *FragmentShaderSource = R"STR(
#version 330 core
out vec4 OutputColor;

void
main()
{
    OutputColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
)STR";
#endif

/* Texture shaders. */
#ifdef OGL_TEXTURE
global char *VertexShaderSource = R"STR(
#version 330 core
layout (location = 0) in vec2 Pos;
layout (location = 1) in vec2 vsTexCoords;

uniform ivec2 WindowDimensions;

out vec2 fsTexCoords;

void
main()
{
    vec2 ClipPos = ((2.0f * Pos) / WindowDimensions) - 1.0f;
    gl_Position = vec4(ClipPos, 0.0, 1.0);

    fsTexCoords = vsTexCoords;
}
)STR";

global char *FragmentShaderSource = R"STR(
#version 330 core
in vec2 fsTexCoords;

uniform sampler2D Texture;

out vec4 OutputColor;

void
main()
{
    OutputColor = texture(Texture, fsTexCoords);
}
)STR";
#endif

bool32 IsInitialized;
uint32_t ShaderProgram, VertexShader, FragmentShader, Vbo, Vao;

internal void
UpdateAndRender(uint32_t WindowWidth, uint32_t WindowHeight)
{
    if(!IsInitialized)
    {
        glViewport(0, 0, WindowWidth, WindowHeight);

        /* Basic objects. */
#ifdef OGL_BASIC
        VertexShader = CreateShader(GL_VERTEX_SHADER, VertexShaderSource);
        FragmentShader = CreateShader(GL_FRAGMENT_SHADER, FragmentShaderSource);

        ShaderProgram = glCreateProgram();
        glAttachShader(ShaderProgram, VertexShader);
        glAttachShader(ShaderProgram, FragmentShader);
        glLinkProgram(ShaderProgram);

        bool32 DidProgramLinkSuccessfully;
        glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &DidProgramLinkSuccessfully);
        if(!DidProgramLinkSuccessfully)
        {
            PrintOpenGLError(ShaderProgram, glGetProgramInfoLog, "Error linking shader program: \n");
            Assert(0);
        }

        glDeleteShader(VertexShader);
        glDeleteShader(FragmentShader);

        float Vertices[] = {
            100.0f, 100.0f,
            200.0f, 100.0f,
            200.0f, 200.0f,

            100.0f, 100.0f,
            200.0f, 200.0f,
            100.0f, 200.0f,
        };

        glGenBuffers(1, &Vbo);
        glBindBuffer(GL_ARRAY_BUFFER, Vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

        glGenVertexArrays(1, &Vao);
        glBindVertexArray(Vao);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
#endif

        /* Texture objects. */
#ifdef OGL_TEXTURE
        VertexShader = CreateShader(GL_VERTEX_SHADER, VertexShaderSource);
        FragmentShader = CreateShader(GL_FRAGMENT_SHADER, FragmentShaderSource);

        ShaderProgram = glCreateProgram();
        glAttachShader(ShaderProgram, VertexShader);
        glAttachShader(ShaderProgram, FragmentShader);
        glLinkProgram(ShaderProgram);

        bool32 DidProgramLinkSuccessfully;
        glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &DidProgramLinkSuccessfully);
        if(!DidProgramLinkSuccessfully)
        {
            PrintOpenGLError(ShaderProgram, glGetProgramInfoLog, "Error linking shader program: \n");
            Assert(0);
        }

        glDeleteShader(VertexShader);
        glDeleteShader(FragmentShader);

        float Vertices[] = {
            300.0f, 100.0f, 0.0f, 0.0f, 
            400.0f, 100.0f, 1.0f, 0.0f, 
            400.0f, 200.0f, 1.0f, 1.0f,

            300.0f, 100.0f, 0.0f, 0.0f, 
            400.0f, 200.0f, 1.0f, 1.0f,
            300.0f, 200.0f, 0.0f, 1.0f,
        };

        glGenBuffers(1, &Vbo);
        glBindBuffer(GL_ARRAY_BUFFER, Vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

        glGenVertexArrays(1, &Vao);
        glBindVertexArray(Vao);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        uint32_t Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int Width, Height, ChannelCount;
        uint8_t *ImageData = stbi_load("container.jpg", &Width, &Height, &ChannelCount, 0);
        if(ImageData)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, ImageData);
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        stbi_image_free(ImageData);
#endif

        IsInitialized = true;
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(ShaderProgram);
    glUniform2i(GetUniformLocation(ShaderProgram, "WindowDimensions"), WindowWidth, WindowHeight);
    glBindVertexArray(Vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
