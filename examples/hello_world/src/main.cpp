#include <enigine.h>

int main()
{
    Enigine enigine;
    // initialize resources
    enigine.init();

    GLFWwindow *window = enigine.window;
    RenderManager *renderManager = enigine.renderManager;
    ResourceManager *resourceManager = enigine.resourceManager;
    ShaderManager *shaderManager = enigine.shaderManager;
    PhysicsWorld *physicsWorld = enigine.physicsWorld;
    DebugDrawer *debugDrawer = enigine.debugDrawer;
    UpdateManager *updateManager = enigine.updateManager;
    TaskManager *taskManager = enigine.taskManager;
    InputManager *inputManager = enigine.inputManager;
    SoundEngine *soundEngine = enigine.soundEngine;
    Camera *mainCamera = enigine.mainCamera;

    // characters
    //NPCharacter npc1(renderManager, taskManager, resourceManager, physicsWorld, mainCamera);
    //PCharacter character(shaderManager, renderManager, taskManager, soundEngine, window, resourceManager, physicsWorld, mainCamera);
    //mainCamera->position = npc1.m_position + glm::vec3(0.f, -1.f, 10.f);

    std::vector<Character *> characters;
    //characters.push_back(&character);
    //characters.push_back(&npc1);

    //character.m_npcList.push_back(&npc1);

    //updateManager->add(&npc1);
    //updateManager->add(&character);

    mainCamera->position = glm::vec3(0.0f, 0.0f, -10.0f);
    mainCamera->front = glm::vec3(0.0f, 0.0f, 1.0f);
    mainCamera->up = glm::vec3(0.0f, 1.0f, 0.0f);
    mainCamera->mouseSensitivity = 0.000001f;
    debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);

    // create the scene
    eTransform transform;
    transform.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    transform.setScale(glm::vec3(2.0f));
    Model &shelter = *resourceManager->getModel("assets/models/rim.obj");
    renderManager->addSource(RenderSourceBuilder()
                                 .setTransform(transform)
                                 .setModel(&shelter)
                                 .build());

    // vehicle
    Models models;
    models.carBody = resourceManager->getModel("assets/models/cylinder.obj");
    models.wheelBase = resourceManager->getModel("assets/models/eim.obj");
    models.doorFront = resourceManager->getModel("assets/models/cylinder.obj");
    models.smokeParticleModel = resourceManager->getModel("assets/models/cylinder.obj");

    Model *collider = resourceManager->getModel("assets/models/cylinder.obj");
    
    eTransform bodyOffset;
    
    Vehicle vehicle(physicsWorld, VehicleType::coupe, collider, bodyOffset, glm::vec3(205.f, 2.f, 255.f));
    
    //CarController car(window, shaderManager, renderManager, resourceManager, &vehicle, mainCamera, models, 2);
    
    float wheelSize[4] = {0.36f, 0.36f, 0.38f, 0.38f};
    glm::vec3 wheelPos[4] = {
        glm::vec3(0.8, 0.81f, 1.27),
        glm::vec3(-0.8, 0.81f, 1.27),
        glm::vec3(0.82, 0.85f, -1.43),
        glm::vec3(-0.82, 0.85f, -1.43)};
    
    for (int i = 0; i < 4; i++)
    {
        vehicle.setWheelPosition(i, wheelPos[i]);
        vehicle.setWheelRadius(i, wheelSize[i]);
        //car.m_linkTireSmokes[i]->m_offset.setPosition(wheelPos[i] - glm::vec3(0.f, 0.7f, 0.f));
    }
    
    // door offset - when opened
    vehicle.setActiveDoorHingeOffsetFront(glm::vec3(0.8f, 1.17f, 0.8f), glm::vec3(0.6f, 0.0f, 0.5f));

    // set environment map
    std::string path = resourceManager->m_executablePath + "/assets/hdris/qwantani_puresky.hdr";
    TextureParams textureParams;
    textureParams.dataType = TextureDataType::Float32;
    Texture envTexture = resourceManager->textureFromFile(textureParams, path, path);
    renderManager->updateEnvironmentTexture(envTexture);
    renderManager->m_shadowManager->m_lightPos = glm::normalize(glm::vec3(0.693f, 0.51f, 0.51f));

    // add render source
    std::string modelPath = resourceManager->m_executablePath + "/assets/models/rim.glb";
    Model *rimModel = resourceManager->getModelFullPath(modelPath);
    //eTransform transform;
    transform.setScale(glm::vec3(2.f));
    RenderSource *rimSource = RenderSourceBuilder()
                                  .setTransform(transform)
                                  .setModel(rimModel)
                                  .build();
    renderManager->addSource(rimSource);

    // update camera
    mainCamera->position = glm::vec3(-2.f, 0.f, 2.f);
    mainCamera->front = glm::vec3(0.707f, 0.f, -0.707f);

    // start main loop
    enigine.start();
}
