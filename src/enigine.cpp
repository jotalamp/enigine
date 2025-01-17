#include "enigine.h"

Enigine::Enigine()
{
}

Enigine::~Enigine()
{
    // cleanup objects
    delete physicsWorld;
    delete debugDrawer;
    delete soundEngine;
    delete shaderManager;
    delete resourceManager;
    delete renderManager;
    delete inputManager;
    delete mainCamera;

    // cleanup ui
    delete systemMonitorUI;
    delete shadowmapUI;
    delete cameraUI;
    delete resourceUI;
    delete renderUI;
    delete physicsWorldUI;
    delete rootUI;

    // cleanup imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // cleanup glfw
    glfwDestroyWindow(window);
    glfwTerminate();
}

// TODO: return init result enum
int Enigine::init()
{
    CommonUtil::printStartInfo();
    std::string executablePath = CommonUtil::getExecutablePath();

    // Setup window
    glfwSetErrorCallback(CommonUtil::glfwErrorCallback);
    if (!glfwInit())
        return 1;

// Decide GL+GLSL versions
#if __APPLE__
    // GL 4.1 + GLSL 410
    const char *glsl_version = "#version 410";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
    // GL 4.1 + GLSL 410
    const char *glsl_version = "#version 410";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    // TODO: EnigineBuilder()
    window = glfwCreateWindow(1280, 720, "enigine", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    bool err = glewInit() != GLEW_OK;

    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // TODO: move to RenderManager
    // Enable depth test, z-buffer
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    // glDepthFunc(GL_LESS);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Init OpenAL
    soundEngine = new SoundEngine();
    if (!soundEngine->init())
    {
        fprintf(stderr, "Failed to initialize OpenAL!\n");
        return 1;
    }
    soundEngine->setListenerPosition(4.0f, 4.0f, 4.0f);

    // Init Physics
    physicsWorld = new PhysicsWorld();

    debugDrawer = new DebugDrawer();
    debugDrawer->setDebugMode(btIDebugDraw::DBG_NoDebug);
    physicsWorld->m_dynamicsWorld->setDebugDrawer(debugDrawer);

    // Shaders
    shaderManager = new ShaderManager(executablePath);
    shaderManager->addShader(ShaderDynamic(&simpleShader, "assets/shaders/simple-shader.vs", "assets/shaders/simple-shader.fs"));
    shaderManager->addShader(ShaderDynamic(&simpleDeferredShader, "assets/shaders/pbr.vs", "assets/shaders/simple-deferred.fs"));
    shaderManager->addShader(ShaderDynamic(&lineShader, "assets/shaders/line-shader.vs", "assets/shaders/line-shader.fs"));
    shaderManager->addShader(ShaderDynamic(&textureArrayShader, "assets/shaders/simple-texture.vs", "assets/shaders/texture-array.fs"));

    // Create geometries
    resourceManager = new ResourceManager(executablePath);

    Model &cube = *resourceManager->getModel("assets/models/cube.obj");
    Model &sphere = *resourceManager->getModel("assets/models/sphere.obj");
    Model &icosahedron = *resourceManager->getModel("assets/models/icosahedron.obj");
    Model &quad = *resourceManager->getModel("assets/models/quad.obj");

    // Shadowmap display quad
    CommonUtil::createQuad(q_vbo, q_vao, q_ebo);

    mainCamera = new Camera(glm::vec3(10.0f, 3.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //mainCamera->setScrollCallback(window);
    renderManager = new RenderManager(shaderManager, mainCamera, &cube, &quad, &icosahedron, q_vao);
    updateManager = new UpdateManager();
    inputManager = new InputManager(window);

    // Time
    lastFrame = (float)glfwGetTime();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    ImGui::StyleColorsDark();

    // Setup Dear ImGui Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Debug draw objects
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    // UI
    rootUI = new RootUI();
    systemMonitorUI = new SystemMonitorUI();
    shadowmapUI = new ShadowmapUI(renderManager->m_shadowManager, renderManager->m_shadowmapManager);
    cameraUI = new CameraUI(mainCamera);
    resourceUI = new ResourceUI(resourceManager);
    renderUI = new RenderUI(inputManager, renderManager, resourceManager);
    physicsWorldUI = new PhysicsWorldUI(physicsWorld, debugDrawer);
    rootUI->m_uiList.push_back(systemMonitorUI);
    rootUI->m_uiList.push_back(shadowmapUI);
    rootUI->m_uiList.push_back(cameraUI);
    rootUI->m_uiList.push_back(resourceUI);
    rootUI->m_uiList.push_back(renderUI);
    rootUI->m_uiList.push_back(physicsWorldUI);

    updateManager->add(renderUI);
    updateManager->add(systemMonitorUI);

    return 0;
}

void Enigine::start()
{
    while (!glfwWindowShouldClose(window))
    {
        // Calculate deltaTime
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Poll events
        glfwPollEvents();

        // Process input
        mainCamera->processInput(window, deltaTime);

        // Update Physics
        physicsWorld->update(deltaTime);

        // Update updatables
        // TODO: optimize not visible
        updateManager->update(deltaTime);

        // Update audio listener
        soundEngine->setListenerPosition(mainCamera->position.x, mainCamera->position.y, mainCamera->position.z);
        std::vector<float> listenerOrientation;
        listenerOrientation.push_back(mainCamera->front.x);
        listenerOrientation.push_back(mainCamera->front.y);
        listenerOrientation.push_back(mainCamera->front.z);
        listenerOrientation.push_back(mainCamera->up.x);
        listenerOrientation.push_back(mainCamera->up.y);
        listenerOrientation.push_back(mainCamera->up.z);
        soundEngine->setListenerOrientation(&listenerOrientation);

        // render manager - start
        renderManager->setupFrame(window);
        // TODO: transform manager?
        renderManager->updateTransforms();
        renderManager->renderDepth();
        renderManager->renderOpaque();
        renderManager->renderSSAO();
        renderManager->renderDeferredShading();

        // Update Debug Drawer
        debugDrawer->getLines().clear();
        physicsWorld->m_dynamicsWorld->debugDrawWorld();

        // Draw physics debug lines
        glm::mat4 mvp = renderManager->m_viewProjection;
        debugDrawer->drawLines(lineShader, mvp, vbo, vao, ebo);

        // culling debug
        renderManager->m_cullingManager->m_debugDrawer->getLines().clear();
        renderManager->m_cullingManager->m_collisionWorld->debugDrawWorld();
        renderManager->m_cullingManager->m_debugDrawer->drawLines(lineShader, mvp, vbo, vao, ebo);

        // Shadowmap debug
        shadowmapUI->drawFrustum(simpleShader, mvp, vbo, vao, ebo);
        shadowmapUI->drawFrustumAABB(simpleShader, mvp, vbo, vao, ebo);
        shadowmapUI->drawLightAABB(simpleShader, mvp, renderManager->m_inverseDepthViewMatrix, vbo, vao, ebo);

        // render manager debug
        renderUI->drawSelectedSource(simpleShader, mvp);
        renderUI->drawSelectedNormals(lineShader, mvp, vbo, vao, ebo);
        renderUI->drawSelectedArmature(simpleShader);

        // render manager - end
        renderManager->renderBlend();
        renderManager->renderTransmission();
        renderManager->renderPostProcess();

        // Shadowmap debug - should be called after post process
        shadowmapUI->drawShadowmap(textureArrayShader, renderManager->m_screenW, renderManager->m_screenH, q_vao);

        // Render UI
        rootUI->render();

        // Swap buffers
        glfwSwapBuffers(window);
    }
}