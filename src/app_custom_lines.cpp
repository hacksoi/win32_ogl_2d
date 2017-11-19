global char *VertexShaderSource = R"STR(
#version 330 core

layout(location = 0) in vec2 vsBottomPos;
layout(location = 1) in vec2 vsTopPos;

uniform vec2 WindowDimensions;

out vec2 gsWindowDimensions;
out vec2 gsBottomPos;
out vec2 gsTopPos;

void
main()
{
    // dummy
    gl_Position = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    gsWindowDimensions = WindowDimensions;
    gsBottomPos = vsBottomPos;
    gsTopPos = vsTopPos;
}
)STR";

global char *GeometryShaderSource = R"STR(
#version 330 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in vec2 gsWindowDimensions[];
in vec2 gsBottomPos[];
in vec2 gsTopPos[];

out vec2 fsPos;
out vec2 fsLineP1;
out vec2 fsLineP2;

vec4
ToClip(vec2 WorldPos)
{
    return vec4(((2.0f * WorldPos) / gsWindowDimensions[0]) - 1.0f, 0.0f, 1.0f);
}

void
main()
{
    vec2 LineP1 = gsBottomPos[0] + 0.5f*(gsTopPos[0] - gsBottomPos[0]);
    vec2 LineP2 = gsBottomPos[1] + 0.5f*(gsTopPos[1] - gsBottomPos[1]);

    fsLineP1 = LineP1;
    fsLineP2 = LineP2;

    // bottom left
    fsPos = gsBottomPos[0];
    gl_Position = ToClip(gsBottomPos[0]);
    EmitVertex();

    // bottom right
    fsPos = gsBottomPos[1];
    gl_Position = ToClip(gsBottomPos[1]);
    EmitVertex();

    // top left
    fsPos = gsTopPos[0];
    gl_Position = ToClip(gsTopPos[0]);
    EmitVertex();

    // top right
    fsPos = gsTopPos[1];
    gl_Position = ToClip(gsTopPos[1]);
    EmitVertex();

    EndPrimitive();
}
)STR";

global char *FragmentShaderSource = R"STR(
#version 330 core

uniform float LineWidth;

in vec2 fsPos;
in vec2 fsLineP1;
in vec2 fsLineP2;

out vec4 OutputColor;

float
DistToLine()
{
    vec2 LineDir = fsLineP2 - fsLineP1;
    vec2 RelPos = fsPos - fsLineP1;
    float RelPosDotLineDir = dot(RelPos, LineDir);
    float LineDirLenSq = dot(LineDir, LineDir);
    float t = clamp(RelPosDotLineDir / LineDirLenSq, 0.0f, 1.0f);
    vec2 ClosestPointOnLine = fsLineP1 + t*LineDir;
    vec2 Diff = fsPos - ClosestPointOnLine;
    return length(Diff);
}

void
main()
{
    float Distance = DistToLine() / (LineWidth / 2.0f);
    float CutOff = 0.5f;
    float Alpha;
    if(Distance <= CutOff)
    {
        Alpha = 1.0f;
    }
    else
    {
        Alpha = 1.0f - ((1.0f / CutOff) * (Distance - CutOff));
    }
    vec4 LineColor = vec4(0.0f, 0.0f, 0.0f, Alpha);
    OutputColor = LineColor;
}
)STR";

bool32 IsInitialized;
render_objects AARenderObjects;

v2 P1, P2, P3;
bool DrawDebug = true;

const float LineWidth = 40.0f;

shape_renderer ShapeRenderer;

internal void
UpdateAndRender(uint32_t WindowWidth, uint32_t WindowHeight, app_input *Input, float dt)
{
    if(!IsInitialized)
    {
        /* Intitialize opengl-related things. */
        {
            glViewport(0, 0, WindowWidth, WindowHeight);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            Initialize(&ShapeRenderer);

            /* Create anti-aliased line objects. */
            {
                /* Create shader program. */
                {
                    /* Create shaders. */
                    uint32_t VertexShader = CreateShader(GL_VERTEX_SHADER, VertexShaderSource);
                    uint32_t GeometryShader = CreateShader(GL_GEOMETRY_SHADER, GeometryShaderSource);
                    uint32_t FragmentShader = CreateShader(GL_FRAGMENT_SHADER, FragmentShaderSource);

                    /* Create shader program. */
                    AARenderObjects.ShaderProgram = glCreateProgram();
                    glAttachShader(AARenderObjects.ShaderProgram, VertexShader);
                    glAttachShader(AARenderObjects.ShaderProgram, GeometryShader);
                    glAttachShader(AARenderObjects.ShaderProgram, FragmentShader);
                    glLinkProgram(AARenderObjects.ShaderProgram);

                    bool32 DidProgramLinkSuccessfully;
                    glGetProgramiv(AARenderObjects.ShaderProgram, GL_LINK_STATUS, &DidProgramLinkSuccessfully);
                    if(!DidProgramLinkSuccessfully)
                    {
                        PrintOpenGLError(AARenderObjects.ShaderProgram, glGetProgramInfoLog, "Error linking shader program: \n");
                        Assert(0);
                    }

                    glDeleteShader(VertexShader);
                    glDeleteShader(GeometryShader);
                    glDeleteShader(FragmentShader);
                }

                /* Create vertex buffer object. */
                glGenBuffers(1, &AARenderObjects.Vbo);
                glBindBuffer(GL_ARRAY_BUFFER, AARenderObjects.Vbo);

                /* Create vertex array object. */
                glGenVertexArrays(1, &AARenderObjects.Vao);
                glBindVertexArray(AARenderObjects.Vao);

                /* Define first vertex attribute: top intersection point. */
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
                glEnableVertexAttribArray(0);

                /* Define second vertex attribute: bottom intersection point */
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
                glEnableVertexAttribArray(1);
            }
        }

        /* Define points of the two lines. */
        {
            float LineLength = 300.0f;
            P1 = V2((float)WindowWidth/2.0f - LineLength/2.0f, (float)WindowHeight/2.0f);
            P2 = V2((float)WindowWidth/2.0f, (float)WindowHeight/2.0f);
            P3 = V2((float)WindowWidth/2.0f + LineLength/2.0f, (float)WindowHeight/2.0f);

            /* Apply point offsets. */
            {
                float OffsetY = -200.0f;
                P1.Y += OffsetY;
                P2.Y += OffsetY;
                P3.Y += OffsetY;
            }
        }

        IsInitialized = true;
    }

    /* Process input. */
    {
        if(Input->IsCursorDown)
        {
            if(Input->Is1Down)
            {
                P1.X = Input->CursorPosX;
                P1.Y = Input->CursorPosY;
            }
            else if(Input->Is2Down)
            {
                P2.X = Input->CursorPosX;
                P2.Y = Input->CursorPosY;
            }
            else if(Input->Is3Down)
            {
                P3.X = Input->CursorPosX;
                P3.Y = Input->CursorPosY;
            }
        }

        if(Input->IsDPressed)
        {
            DrawDebug = !DrawDebug;
        }
    }

    /* Generate data used for rendering lines. */
    v2 BottomLeft, TopLeft, BottomMiddle, TopMiddle, BottomRight, TopRight;
    {
        /* Create lines. */
        line2 TestLine1 = {P1, P2};
        line2 TestLine2 = {P2, P3};

        /* Create quads out of two lines. */
        quad2 Line1RectSimple = CreateLineQuad(TestLine1, LineWidth);
        quad2 Line2RectSimple = CreateLineQuad(TestLine2, LineWidth);

        /* Get the directions of the quads. */
        v2 Line1TopDirection = Line1RectSimple.TopRight - Line1RectSimple.TopLeft;
        v2 Line2TopDirection = Line2RectSimple.TopLeft - Line2RectSimple.TopRight;
        v2 Line1BottomDirection = Line1RectSimple.BottomRight - Line1RectSimple.BottomLeft;
        v2 Line2BottomDirection = Line2RectSimple.BottomLeft - Line2RectSimple.BottomRight;

        /* Define the rays of the quads. */
        ray2 Line1TopRay = {Line1RectSimple.TopLeft, Line1TopDirection};
        ray2 Line2TopRay = {Line2RectSimple.TopRight, Line2TopDirection};
        ray2 Line1BottomRay = {Line1RectSimple.BottomLeft, Line1BottomDirection};
        ray2 Line2BottomRay = {Line2RectSimple.BottomRight, Line2BottomDirection};

        /* Find the intersection points. */
        v2 TopIntersection, BottomIntersection;
        {
            /* We assume these rays always intersect. */
            Assert(FindIntersection(&TopIntersection, Line1TopRay, Line2TopRay) == true);
            Assert(FindIntersection(&BottomIntersection, Line1BottomRay, Line2BottomRay) == true);
        }

        /* Define final line data. */
        {
            BottomLeft = Line1RectSimple.BottomLeft;
            TopLeft = Line1RectSimple.BottomTopLeft;

            BottomMiddle = BottomIntersection;
            TopMiddle = TopIntersection;

            BottomRight = Line2RectSimple.BottomRight;
            TopRight = Line2RectSimple.TopRight;
        }
    }

    /* Fill render buffers. */
    {
        /* Add primary line data. */
        {
            /* Define vertex buffer. */
            float VertexBuffer[] = {
                /* Vertex data for first point. */
                EXPANDV2(Line1RectSimple.BottomLeft),
                EXPANDV2(Line1RectSimple.TopLeft),

                /* Vertex data for second point. */
                EXPANDV2(BottomIntersection),
                EXPANDV2(TopIntersection),

                /* Vertex data for third point. */
                EXPANDV2(Line2RectSimple.BottomRight),
                EXPANDV2(Line2RectSimple.TopRight),
            };

            /* Fill vertex buffer object with our data. */
            glBindBuffer(GL_ARRAY_BUFFER, AARenderObjects.Vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(VertexBuffer), VertexBuffer, GL_STREAM_DRAW);
        }

        /* Add debug data. */
        {
            AddPoint(&ShapeRenderer, BottomIntersection, NPS_BLUE);
            AddPoint(&ShapeRenderer, TopIntersection, NPS_RED);
            AddPoint(&ShapeRenderer, TestLine1.P2, NPS_YELLOW);

            AddRay(&ShapeRenderer, Line1BottomRay, 400.0f, NPS_ORANGE);
            AddRay(&ShapeRenderer, Line1TopRay, 400.0f, NPS_ORANGE);

            AddRay(&ShapeRenderer, Line2BottomRay, 400.0f, NPS_GREEN);
            AddRay(&ShapeRenderer, Line2TopRay, 400.0f, NPS_GREEN);
        }
    }

    /* Begin rendering. */
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        /* Draw custom anti-aliased lines. */
        {
            /* Bind objects for rendering. */
            glUseProgram(AARenderObjects.ShaderProgram);
            glBindVertexArray(AARenderObjects.Vao);

            /* Assign uniforms. */
            glUniform2f(GetUniformLocation(AARenderObjects.ShaderProgram, "WindowDimensions"), (float)WindowWidth, (float)WindowHeight);
            glUniform1f(GetUniformLocation(AARenderObjects.ShaderProgram, "LineWidth"), LineWidth);

            /* Render our data. */
            glDrawArrays(GL_LINE_STRIP, 0, 3);

            if(DrawDebug)
            {
                /* Render debug data. */
                RenderData(&ShapeRenderer);
            }
        }
    }
}
