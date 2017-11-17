#if 0
global char *VertexShaderSource = R"STR(
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

global char *GeometryShaderSource = R"STR(
#version 330 core
layout(lines) in;
layout(triangle_strip, max_vertices = 12) out;

struct rect
{
    vec2 BottomLeft;
    vec2 BottomRight;
    vec2 TopLeft;
    vec2 TopRight;
};

uniform vec2 WindowDimensions;
uniform float LineWidth;

out vec2 fsDistToLine;

void main()
{
    vec2 P1 = gl_in[0].gl_Position.xy;
    vec2 P2 = gl_in[1].gl_Position.xy;

    // get the original line positions
    vec2 OriginalP1 = (P1 + 1.0f) * (WindowDimensions / 2.0f);
    vec2 OriginalP2 = (P2 + 1.0f) * (WindowDimensions / 2.0f);

    vec2 OriginalLineDirection = normalize(vec2(OriginalP2 - OriginalP1));
    vec2 OriginalLineNormal = vec2(-OriginalLineDirection.y, OriginalLineDirection.x);

    // we need a vector such that after the viewport transform, it is normal to the line on the screen;
    // this vector is used as the normal in clip space
    vec2 ClipNormal = normalize(OriginalLineNormal / (WindowDimensions / 2.0f));

    float ClipLineWidth = 2.0f * sqrt(LineWidth*LineWidth / (ClipNormal.x*ClipNormal.x * WindowDimensions.x*WindowDimensions.x + 
                                                             ClipNormal.y*ClipNormal.y * WindowDimensions.y*WindowDimensions.y));
    float HalfLineWidth = ClipLineWidth / 2.0f;

    // draw main rect

    rect MainRect;
    MainRect.BottomLeft = P1 - HalfLineWidth*ClipNormal;
    MainRect.BottomRight = P2 - HalfLineWidth*ClipNormal;
    MainRect.TopLeft = P1 + HalfLineWidth*ClipNormal;
    MainRect.TopRight = P2 + HalfLineWidth*ClipNormal;

    fsDistToLine = vec2(0.0f, -1.0f);
    gl_Position = vec4(MainRect.BottomLeft, 0.0f, 1.0f);
    EmitVertex();

    fsDistToLine = vec2(0.0f, -1.0f);
    gl_Position = vec4(MainRect.BottomRight, 0.0f, 1.0f);
    EmitVertex();

    fsDistToLine = vec2(0.0f, 1.0f);
    gl_Position = vec4(MainRect.TopLeft, 0.0f, 1.0f);
    EmitVertex();

    fsDistToLine = vec2(0.0f, 1.0f);
    gl_Position = vec4(MainRect.TopRight, 0.0f, 1.0f);
    EmitVertex();

    EndPrimitive();

#if 1
    // draw left edge

    vec2 LineDirection = normalize(P2 - P1);
    vec2 EdgeExtender = HalfLineWidth * LineDirection;

    rect LeftEdgeRect;
    LeftEdgeRect.BottomLeft = MainRect.BottomLeft - EdgeExtender;
    LeftEdgeRect.BottomRight = MainRect.BottomLeft;
    LeftEdgeRect.TopLeft = MainRect.TopLeft - EdgeExtender;
    LeftEdgeRect.TopRight = MainRect.TopLeft;

    fsDistToLine = vec2(-1.0f, -1.0f);
    gl_Position = vec4(LeftEdgeRect.BottomLeft, 0.0f, 1.0f);
    EmitVertex();

    fsDistToLine = vec2(0.0f, -1.0f);
    gl_Position = vec4(LeftEdgeRect.BottomRight, 0.0f, 1.0f);
    EmitVertex();

    fsDistToLine = vec2(-1.0f, 1.0f);
    gl_Position = vec4(LeftEdgeRect.TopLeft, 0.0f, 1.0f);
    EmitVertex();

    fsDistToLine = vec2(0.0f, 1.0f);
    gl_Position = vec4(LeftEdgeRect.TopRight, 0.0f, 1.0f);
    EmitVertex();

    EndPrimitive();

    // draw right edge

    rect RightEdgeRect;
    RightEdgeRect.BottomLeft = MainRect.BottomRight;
    RightEdgeRect.BottomRight = MainRect.BottomRight + EdgeExtender;
    RightEdgeRect.TopLeft = MainRect.TopRight;
    RightEdgeRect.TopRight = MainRect.TopRight + EdgeExtender;

    fsDistToLine = vec2(0.0f, -1.0f);
    gl_Position = vec4(RightEdgeRect.BottomLeft, 0.0f, 1.0f);
    EmitVertex();

    fsDistToLine = vec2(1.0f, -1.0f);
    gl_Position = vec4(RightEdgeRect.BottomRight, 0.0f, 1.0f);
    EmitVertex();

    fsDistToLine = vec2(0.0f, 1.0f);
    gl_Position = vec4(RightEdgeRect.TopLeft, 0.0f, 1.0f);
    EmitVertex();

    fsDistToLine = vec2(1.0f, 1.0f);
    gl_Position = vec4(RightEdgeRect.TopRight, 0.0f, 1.0f);
    EmitVertex();

    EndPrimitive();
#endif
}
)STR";

global char *FragmentShaderSource = R"STR(
#version 330 core
in vec2 fsDistToLine;

out vec4 OutputColor;

void
main()
{
#if 0
    float SquaredDistToLine = fsDistToLine.x*fsDistToLine.x + fsDistToLine.y*fsDistToLine.y;
    float Alpha = clamp(1.0f - SquaredDistToLine, 0.0f, 1.0f);
    vec4 LineColor = vec4(0.0f, 0.0f, 0.0f, Alpha);
    OutputColor = LineColor;
#endif

#if 1
    float DistToLine = length(fsDistToLine);
    float Alpha;
    if(DistToLine < 0.9f)
    {
        Alpha = 1.0f;
    }
    else
    {
        Alpha = clamp(1.0f - ((DistToLine - 0.9f) * 10.0f), 0.0f, 1.0f);
    }
    vec4 LineColor = vec4(0.0f, 0.0f, 0.0f, Alpha);
    OutputColor = LineColor;
#endif

#if 0
    vec4 LineColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    OutputColor = LineColor;
#endif
}
)STR";
#endif

global char *VertexShaderSource = R"STR(
#version 330 core
layout(location = 0) in vec2 vsPos;
layout(location = 1) in vec2 vsLineP1;
layout(location = 2) in vec2 vsLineP2;

uniform vec2 WindowDimensions;

out vec2 fsPos;
out vec2 fsLineP1;
out vec2 fsLineP2;

void
main()
{
    vec2 ClipPos = ((2.0f * vsPos) / WindowDimensions) - 1.0f;
    gl_Position = vec4(ClipPos, 0.0, 1.0f);

    fsPos = vsPos;
    fsLineP1 = vsLineP1;
    fsLineP2 = vsLineP2;
}
)STR";

global char *FragmentShaderSource = R"STR(
#version 330 core
in vec2 fsPos;
in vec2 fsLineP1;
in vec2 fsLineP2;

uniform float LineWidth;

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
    float Alpha;// = clamp(1.0f - pow(, 2), 0.0f, 1.0f);
    float Distance = DistToLine() / (LineWidth / 2.0f);
    if(Distance <= 0.9f)
    {
        Alpha = 1.0f;
    }
    else
    {
        Alpha = 1.0f - (10.0f * (Distance - 0.9f));
    }
    vec4 LineColor = vec4(0.0f, 0.0f, 0.0f, Alpha);
    OutputColor = LineColor;
}
)STR";

bool32 IsInitialized;
render_objects AARenderObjects;

v2 P1, P2, P3;
bool DrawDebug = true;

//float Rotation;

internal void
UpdateAndRender(uint32_t WindowWidth, uint32_t WindowHeight, app_input *Input, float dt)
{
    if(!IsInitialized)
    {
        float LineLength = 300.0f;
        P1 = V2((float)WindowWidth/2.0f - LineLength/2.0f, (float)WindowHeight/2.0f);
        P2 = V2((float)WindowWidth/2.0f, (float)WindowHeight/2.0f/* + 300.0f*/);
        P3 = V2((float)WindowWidth/2.0f + LineLength/2.0f, (float)WindowHeight/2.0f);

        // point offsets
        {
            float BaseOffsetY = -200.0f;
            P1.Y += BaseOffsetY;
            P2.Y += BaseOffsetY;
            P3.Y += BaseOffsetY;
        }

        glViewport(0, 0, WindowWidth, WindowHeight);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        /* Anti-aliased line objects. */
        {
            uint32_t VertexShader = CreateShader(GL_VERTEX_SHADER, VertexShaderSource);
            //uint32_t GeometryShader = CreateShader(GL_GEOMETRY_SHADER, GeometryShaderSource);
            uint32_t FragmentShader = CreateShader(GL_FRAGMENT_SHADER, FragmentShaderSource);

            AARenderObjects.ShaderProgram = glCreateProgram();
            glAttachShader(AARenderObjects.ShaderProgram, VertexShader);
            //glAttachShader(AARenderObjects.ShaderProgram, GeometryShader);
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
            //glDeleteShader(GeometryShader);
            glDeleteShader(FragmentShader);

            glGenBuffers(1, &AARenderObjects.Vbo);
            glBindBuffer(GL_ARRAY_BUFFER, AARenderObjects.Vbo);

            glGenBuffers(1, &AARenderObjects.Ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AARenderObjects.Ebo);

            glGenVertexArrays(1, &AARenderObjects.Vao);
            glBindVertexArray(AARenderObjects.Vao);

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(4 * sizeof(float)));
            glEnableVertexAttribArray(2);
        }

        IsInitialized = true;
    }

    /* Process input. */

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

    /* Create lines. */

    line2 TestLine1 = {P1, P2};
    line2 TestLine2 = {P2, P3};

    /* Fill vertex buffer. */

    float LineWidth = 40.0f;

    rect2 Line1RectSimple = CreateLineRect(TestLine1, LineWidth);
    rect2 Line2RectSimple = CreateLineRect(TestLine2, LineWidth);

    v2 Line1TopDirection = Line1RectSimple.TopRight - Line1RectSimple.TopLeft;
    v2 Line2TopDirection = Line2RectSimple.TopLeft - Line2RectSimple.TopRight;
    v2 Line1BottomDirection = Line1RectSimple.BottomRight - Line1RectSimple.BottomLeft;
    v2 Line2BottomDirection = Line2RectSimple.BottomLeft - Line2RectSimple.BottomRight;

    ray2 Line1TopRay = {Line1RectSimple.TopLeft, Line1TopDirection};
    ray2 Line2TopRay = {Line2RectSimple.TopRight, Line2TopDirection};
    ray2 Line1BottomRay = {Line1RectSimple.BottomLeft, Line1BottomDirection};
    ray2 Line2BottomRay = {Line2RectSimple.BottomRight, Line2BottomDirection};

    v2 TopIntersection = FindIntersection(Line1TopRay, Line2TopRay);
    v2 BottomIntersection = FindIntersection(Line1BottomRay, Line2BottomRay);

#if 0
    rect2 Line1RectFinal = {
        Line1RectSimple.BottomLeft,
        BottomIntersection,
        TopIntersection,
        Line1Rect.TopLeft
    };

    rect2 Line2RectFinal = {
        BottomIntersection,
        Line2Rect.BottomRight,
        Line2Rect.TopRight,
        TopIntersection,
    };

    float VertexBuffer[] = {
        EXPANDV2(Line1RectFinal.BottomLeft), -1.0f, // bottom left
        EXPANDV2(Line1RectFinal.TopLeft), 1.0f, // top left
        EXPANDV2(Line1RectFinal.BottomRight), -GetDistance(Line1RectFinal.BottomRight, P2), // middle bottom
        EXPANDV2(Line1RectFinal.TopRight), GetDistance(Line1RectFinal.TopRight, P2), // middle top

        EXPANDV2(Line2RectFinal.BottomLeft), -GetDistance(Line2RectFinal.BottomLeft, P2),
        EXPANDV2(Line2RectFinal.BottomRight), -1.0f,
        EXPANDV2(Line2RectFinal.TopRight), 1.0f,
        EXPANDV2(Line2RectFinal.TopLeft), GetDistance(Line2RectFinal.TopLeft, P2),
    };
#endif

#if 1
#if 0
    float VertexBuffer[] = {
        EXPANDV2(Line1RectSimple.BottomLeft), -1.0f, // bottom left (0)
        EXPANDV2(Line1RectSimple.TopLeft), 1.0f, // top left (1)

#if 1
        EXPANDV2(BottomIntersection), -BottomDistance / (LineWidth / 2.0f), // bottom middle (2)
        EXPANDV2(TopIntersection), TopDistance / (LineWidth / 2.0f), // top middle (3)
#else
        EXPANDV2(BottomIntersection), -1.0f,//-GetDistance(BottomIntersection, TestLine1.P2) / LineWidth / 2.0f, // bottom middle (2)
        EXPANDV2(TopIntersection), 1.0f,//GetDistance(TopIntersection, TestLine1.P2) / LineWidth / 2.0f, // top middle (3)
#endif

        EXPANDV2(Line2RectSimple.BottomRight), -1.0f, // bottom right (4)
        EXPANDV2(Line2RectSimple.TopRight), 1.0f, // top right (5)
    };
#else
    float BottomIntersectionProjectedOntoLine1 = GetProjectedCoord(BottomIntersection, TestLine1);
    float TopIntersectionProjectedOntoLine1 = GetProjectedCoord(TopIntersection, TestLine1);

    float BottomIntersectionProjectedOntoLine2 = GetProjectedCoord(BottomIntersection, TestLine2);
    float TopIntersectionProjectedOntoLine2 = GetProjectedCoord(TopIntersection, TestLine2);

    float VertexBuffer[] = {
        /* Left line */
        EXPANDV2(Line1RectSimple.BottomLeft), EXPANDV2(TestLine1.P1), EXPANDV2(TestLine1.P2), // bottom left (0)
        EXPANDV2(BottomIntersection), EXPANDV2(TestLine1.P1), EXPANDV2(TestLine1.P2), // bottom right (1)
        EXPANDV2(TopIntersection), EXPANDV2(TestLine1.P1), EXPANDV2(TestLine1.P2), // top right (2)
        EXPANDV2(Line1RectSimple.TopLeft), EXPANDV2(TestLine1.P1), EXPANDV2(TestLine1.P2), // top left (3)

        /* Right line */
        EXPANDV2(BottomIntersection), EXPANDV2(TestLine2.P1), EXPANDV2(TestLine2.P2), // bottom left (4)
        EXPANDV2(Line2RectSimple.BottomRight), EXPANDV2(TestLine2.P1), EXPANDV2(TestLine2.P2), // bottom right (5)
        EXPANDV2(Line2RectSimple.TopRight), EXPANDV2(TestLine2.P1), EXPANDV2(TestLine2.P2), // top right (6)
        EXPANDV2(TopIntersection), EXPANDV2(TestLine2.P1), EXPANDV2(TestLine2.P2), // top left (7)
    };
#endif

    uint32_t IndexBuffer[] = {
        0, 1, 2,
        0, 2, 3,

#if 1
        4, 5, 6,
        4, 6, 7,
#endif
    };
#else 
    float VertexBuffer[] = {
        EXPANDV2(Line1RectSimple.BottomLeft), -1.0f, // bottom left (0)
        EXPANDV2(Line1RectSimple.BottomRight), 1.0f, // top left (1)
        EXPANDV2(Line1RectSimple.TopRight), 1.0f, // top left (2)
        EXPANDV2(Line1RectSimple.TopLeft), 1.0f, // top left (3)

        EXPANDV2(Line2RectSimple.BottomLeft), -1.0f, // bottom left (4)
        EXPANDV2(Line2RectSimple.BottomRight), 1.0f, // top left (5)
        EXPANDV2(Line2RectSimple.TopRight), 1.0f, // top left (6)
        EXPANDV2(Line2RectSimple.TopLeft), 1.0f, // top left (7)
    };

    uint32_t IndexBuffer[] = {
        0, 1, 2,
        0, 2, 3,

        4, 5, 6,
        4, 6, 7,
    };
#endif

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* AA */
    {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glBindBuffer(GL_ARRAY_BUFFER, AARenderObjects.Vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(VertexBuffer), VertexBuffer, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AARenderObjects.Ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IndexBuffer), IndexBuffer, GL_STATIC_DRAW);

        glUseProgram(AARenderObjects.ShaderProgram);
        glUniform2f(GetUniformLocation(AARenderObjects.ShaderProgram, "WindowDimensions"), (float)WindowWidth, (float)WindowHeight);
        glUniform1f(GetUniformLocation(AARenderObjects.ShaderProgram, "LineWidth"), LineWidth);
        glBindVertexArray(AARenderObjects.Vao);
        //glDrawArrays(GL_LINE_STRIP, 0, ArrayCount(LineVertices) / 2);
        //glDrawArrays(GL_TRIANGLES, 0, ArrayCount(LineVertices) / 2);
        glDrawElements(GL_TRIANGLES, ArrayCount(IndexBuffer), GL_UNSIGNED_INT, 0);

#if 1
        if(DrawDebug)
        {
            DrawPoint(BottomIntersection, NPS_BLUE);
            DrawPoint(TopIntersection, NPS_RED);
            DrawPoint(TestLine1.P2, NPS_YELLOW);

            DrawRay(Line1BottomRay, 400.0f, NPS_ORANGE);
            DrawRay(Line1TopRay, 400.0f, NPS_ORANGE);

            DrawRay(Line2BottomRay, 400.0f, NPS_GREEN);
            DrawRay(Line2TopRay, 400.0f, NPS_GREEN);
        }
#endif
    }

#if 0
    /* Basic rectangle */
    {
        float Offset = -50.0f;

        LineVertices[1] += Offset;
        LineVertices[3] += Offset;

        rect2 LineRect = CreateLineRect(TestLine, LineWidth);
        float LineRectVerts[12] = {
            EXPANDV2(LineRect.BottomLeft),
            EXPANDV2(LineRect.BottomRight),
            EXPANDV2(LineRect.TopRight),

            EXPANDV2(LineRect.BottomLeft),
            EXPANDV2(LineRect.TopRight),
            EXPANDV2(LineRect.TopLeft),
        };

        glBindBuffer(GL_ARRAY_BUFFER, BasicRenderObjects.Vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(LineRectVerts), LineRectVerts, GL_STATIC_DRAW);

        glUseProgram(BasicRenderObjects.ShaderProgram);
        glUniform2f(GetUniformLocation(BasicRenderObjects.ShaderProgram, "WindowDimensions"), (float)WindowWidth, (float)WindowHeight);
        glBindVertexArray(BasicRenderObjects.Vao);
        glDrawArrays(GL_TRIANGLES, 0, ArrayCount(LineRectVerts) / 2);

        LineVertices[1] -= Offset;
        LineVertices[3] -= Offset;
    }

    /* Basic line */
    {
        float Offset = -150.0f;

        LineVertices[1] += Offset;
        LineVertices[3] += Offset;

        glLineWidth(LineWidth);

        glBindBuffer(GL_ARRAY_BUFFER, BasicRenderObjects.Vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(LineVertices), LineVertices, GL_STATIC_DRAW);

        glUseProgram(BasicRenderObjects.ShaderProgram);
        glUniform2f(GetUniformLocation(BasicRenderObjects.ShaderProgram, "WindowDimensions"), (float)WindowWidth, (float)WindowHeight);
        glBindVertexArray(BasicRenderObjects.Vao);
        glDrawArrays(GL_LINE_STRIP, 0, ArrayCount(LineVertices) / 2);

        LineVertices[1] -= Offset;
        LineVertices[3] -= Offset;
    }
#endif
}
