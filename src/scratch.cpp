bool32 IsInitialized;
render_objects BasicRenderObjects;

internal void
UpdateAndRender(uint32_t WindowWidth, uint32_t WindowHeight, float dt)
{
    if(!IsInitialized)
    {
        glViewport(0, 0, WindowWidth, WindowHeight);

        CreateBasicRenderObjects(&BasicRenderObjects);

        IsInitialized = true;
    }

    float VertexData[] = {
        300.0f, 200.0f,
        400.0f, 400.0f,
        500.0f, 200.0f,
        600.0f, 400.0f,
        700.0f, 200.0f,
    };
    glBindBuffer(GL_ARRAY_BUFFER, BasicRenderObjects.Vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPointSize(10.0f);

    glUseProgram(BasicRenderObjects.ShaderProgram);
    glBindVertexArray(BasicRenderObjects.Vao);
    glUniform2f(GetUniformLocation(BasicRenderObjects.ShaderProgram, "WindowDimensions"), (float)WindowWidth, (float)WindowHeight);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, ArrayCount(VertexData) / 2);
}
