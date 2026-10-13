#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "./header/Shader.h"
#include "./header/Object.h"

// ============================================================================
// ICG 2026 HW1 - STUDENT STARTER CODE
//
// This file intentionally keeps the inherited main_ori.cpp skeleton style.
// Most graded parts are left as TODOs, similar to the previous HW starter.
//
// Provided infrastructure:
//   - GLFW / GLAD initialization
//   - render loop structure
//   - object loading
//   - drawModel(..., alpha)
//   - basic state / constants needed by the assignment
//
// Students should complete the TODO sections according to the slides.
// ============================================================================

// Settings
const int INITIAL_SCR_WIDTH = 800;
const int INITIAL_SCR_HEIGHT = 600;

int SCR_WIDTH = INITIAL_SCR_WIDTH;
int SCR_HEIGHT = INITIAL_SCR_HEIGHT;

// Global objects
Shader *shader = nullptr;
Object *cube = nullptr;
Object *bird = nullptr;
Object *flower = nullptr;
Object *magicBall = nullptr;

// ============================================================================
// Scene settings
// ============================================================================

const glm::vec3 GROUND_POSITION(0.0f, -0.5f, 0.0f);
const glm::vec3 GROUND_SCALE(70.0f, 1.0f, 40.0f);
const glm::vec3 GROUND_COLOR(0.25f, 0.65f, 0.20f);

const glm::vec3 BIRD_BASE_POSITION(0.0f, 10.0f, -5.0f);

// Keep this consistent with the slides.
const glm::vec3 BIRD_SCALE(0.20f);
// Reference values only.
// You may change the ellipse size and flying speed.
const float BIRD_RADIUS_X = 10.0f;
const float BIRD_RADIUS_Z = 4.5f;
const float BIRD_SPEED = 0.45f;

const glm::vec3 FLOWER_POSITIONS[3] = {
    glm::vec3(-7.0f, 0.0f, 4.5f),
    glm::vec3(7.5f, 0.0f, 4.0f),
    glm::vec3(5.0f, 0.0f, -6.0f)};

const glm::vec3 FLOWER_SCALE(1.35f);

// ============================================================================
// Character state
// ============================================================================

struct Slime
{
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    float angle = 0.0f;
    float speed = 5.0f;

    bool movingHorizontally = false;
    float hopTime = 0.0f;

    bool movingDown = false;
    float scale_limit = 1.0f;
    float current_scale = 1.0f;
} playerSlime;

// ============================================================================
// Spell state
// ============================================================================

struct Spell
{
    bool active = false;
    float elapsed = 0.0f;
    float duration = 8.0f;

    float mainSelfAngle = 0.0f;
    float smallSelfAngle = 0.0f;
    float orbitAngle = 0.0f;
} spell;

const glm::vec3 MAGIC_BALL_ABOVE_POS(0.0f, 5.6f, 0.0f);
const glm::vec3 MAGIC_BALL_FRONT_POS(0.0f, 2.4f, 4.5f);

const float MAIN_BALL_SCALE = 0.40f;
const float SMALL_BALL_SCALE = 0.14f;
const float ORBIT_RADIUS = 2.0f;

// ============================================================================
// Function declarations
// ============================================================================

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window, float deltaTime);

void drawModel(
    std::string type,
    const glm::mat4 &model,
    const glm::mat4 &view,
    const glm::mat4 &projection,
    const glm::vec3 &color,
    float alpha = 1.0f);

void drawSlime(
    const glm::mat4 &view,
    const glm::mat4 &projection);

void drawBird(
    float currentTime,
    const glm::mat4 &view,
    const glm::mat4 &projection);

void drawFlowers(
    const glm::mat4 &view,
    const glm::mat4 &projection);

void drawMagic(
    const glm::mat4 &view,
    const glm::mat4 &projection);

void updateSpell(float deltaTime);

void cleanup();
void init();

// ============================================================================
// main()
// Keep the inherited main_ori.cpp flow.
// ============================================================================

int main()
{
    // GLFW: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW window creation
    GLFWwindow *window =
        glfwCreateWindow(
            SCR_WIDTH,
            SCR_HEIGHT,
            "ICG 2026 HW1",
            nullptr,
            nullptr);

    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);

    // GLAD: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // TODO: Enable depth test and face culling.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    // Alpha support is provided by the starter code.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize Object and Shader
    init();

    float lastFrame =
        static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window))
    {
        // Calculate delta time for animation
        float currentFrame =
            static_cast<float>(glfwGetTime());

        float deltaTime =
            currentFrame - lastFrame;

        lastFrame =
            currentFrame;

        // TODO: Update spell / animation state.
        updateSpell(deltaTime);

        // Render background
        glClearColor(
            0.65f,
            0.85f,
            1.0f,
            1.0f);

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

        shader->use();

        /*=================== Example of creating model matrix =======================
        1. translate
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 1.0f, 0.0f));
        drawModel("cube", model, view, projection, glm::vec3(0.9f, 0.8f, 0.6f));

        2. scale
        glm::mat4 model(1.0f);
        model = glm::scale(model, glm::vec3(0.5f, 1.0f, 2.0f));
        drawModel("cube", model, view, projection, glm::vec3(0.9f, 0.8f, 0.6f));

        3. rotate
        glm::mat4 model(1.0f);
        model = glm::rotate(
            model,
            glm::radians(45.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));
        drawModel("cube", model, view, projection, glm::vec3(0.9f, 0.8f, 0.6f));
        ==========================================================================*/

        // TODO: Create view matrix and perspective projection matrix.
        //
        // Camera:
        // Position = (0, 14, 23)
        // Target   = (0, 5, 0)
        // Up       = (0, 1, 0)
        // FOV      = 45
        // Near     = 0.1
        // Far      = 1000
        //
        // The identity matrices below are placeholders so the starter compiles.
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 14.0f, 23.0f),
            glm::vec3(0.0f, 5.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f,
            1000.0f);

        // TODO: Scene Setup
        // - Draw the ground cube using the fixed position / scale / color.
        glm::mat4 ground_model(1.0f);
        ground_model = glm::translate(ground_model, GROUND_POSITION);
        ground_model = glm::scale(ground_model, GROUND_SCALE);
        drawModel("cube", ground_model, view, projection, glm::vec3(0.25f, 0.65f, 0.20f));

        // - Draw the bird.
        // - Draw exactly three flowers at the fixed positions / scale.
        //
        // Suggested calls after implementing each function:
        drawBird(currentFrame, view, projection);
        drawFlowers(view, projection);

        // TODO: Draw the main slime character.
        // - At least 3 cubes.
        // - Hierarchical transformation required.
        // - Shared root transformation.
        // - Facing direction should be visually distinguishable.
        //
        drawSlime(view, projection);

        // TODO: Draw / animate the magic spell.
        drawMagic(view, projection);

        // TODO: Implement input processing.
        processInput(window, deltaTime);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup();
    glfwTerminate();
    return 0;
}

// ============================================================================
// Window callback
// ============================================================================

void framebuffer_size_callback(
    GLFWwindow *window,
    int width,
    int height)
{

    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

// ============================================================================
// Input
// ============================================================================

void processInput(
    GLFWwindow *window,
    float deltaTime)
{

    // We use processInput() in the display loop instead of relying only on
    // keyCallback() because continuous movement requires checking key state
    // every frame.

    // TODO:
    // Controls:
    // - W / S           : Move forward / backward in world space.
    // - A / D           : Move left / right in world space.
    // - SPACE / LSHIFT  : Move up / down.
    //
    // Behavior:
    // - Use deltaTime for frame-rate-independent movement.
    // - Slime facing direction follows horizontal movement direction.
    // - Horizontal movement on the ground should produce hopping.
    // - Hopping is not required while flying.
    // - The character must not move below the ground.
    /* horizantal */
    playerSlime.movingHorizontally = false;
    playerSlime.movingDown = false;
    float distance = playerSlime.speed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        // 前進
        playerSlime.angle = 0.0f;
        playerSlime.position.z += distance;
        playerSlime.movingHorizontally = true;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        // 後退
        playerSlime.angle = 180.0f;
        playerSlime.position.z -= distance;
        playerSlime.movingHorizontally = true;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        // 向左
        playerSlime.angle = -90.0f;
        playerSlime.position.x -= distance;
        playerSlime.movingHorizontally = true;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        // 向右
        playerSlime.angle = 90.0f;
        playerSlime.position.x += distance;
        playerSlime.movingHorizontally = true;
    }

    if (playerSlime.position.y <= 0.01f && playerSlime.movingHorizontally)
    {
        playerSlime.hopTime += deltaTime;
    }
    else
    {
        playerSlime.hopTime = 0.0f;
    }

    /* vertical */
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        // 向上
        playerSlime.position.y += distance;
        playerSlime.scale_limit = 0.7f;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        // 向下
        // playerSlime.position.y -= distance;
        // playerSlime.scale_limit = 1.3f;
        // playerSlime.movingDown = true;
        playerSlime.movingDown = true;
        if (playerSlime.position.y > 0.0f)
        {
            playerSlime.position.y -= distance;
            if (playerSlime.position.y <= 0.0f)
            {
                // 向下時碰到地板
                playerSlime.position.y = 0.0f;
                playerSlime.scale_limit = 0.7f;
            }
            else
            {
                playerSlime.scale_limit = 1.3f;
            }
        }
        else
        {
            // 原本就在地上
            playerSlime.position.y = 0.0f;
            playerSlime.scale_limit = 0.7f;
        }
    }
    else if (playerSlime.position.y <= 0.01f && playerSlime.movingHorizontally)
    {
        // squeeze during hop
        float w = 8.0f;
        playerSlime.scale_limit = 0.75f + 0.3f * abs(sin(w * playerSlime.hopTime));
    }
    else
    {
        playerSlime.scale_limit = 1.0f;
    }

    // squeeze
    float squeeze_speed = 12.0f * deltaTime;
    playerSlime.current_scale = glm::mix(playerSlime.current_scale, playerSlime.scale_limit, glm::clamp(squeeze_speed, 0.0f, 1.0f));

    // TODO: Ground boundary collision.
    if (playerSlime.position.y < 0.0f)
    {
        playerSlime.position.y = 0.0f;
    }
}

void keyCallback(
    GLFWwindow *window,
    int key,
    int scancode,
    int action,
    int mods)
{

    // The action is one of GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE.
    // https://www.glfw.org/docs/3.3/input_guide.html

    if (key == GLFW_KEY_ESCAPE &&
        action == GLFW_PRESS)
    {

        glfwSetWindowShouldClose(window, true);
    }

    // TODO: Press R to cast the spell.
    //
    // Pressing R while a spell is active should not restart the animation.
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        if (!spell.active && spell.elapsed <= 0.01f)
            spell.active = true;
    }
}

// ============================================================================
// drawModel()
// alpha support is provided.
// ============================================================================

void drawModel(
    std::string type,
    const glm::mat4 &model,
    const glm::mat4 &view,
    const glm::mat4 &projection,
    const glm::vec3 &color,
    float alpha)
{

    shader->set_uniform("projection", projection);
    shader->set_uniform("view", view);
    shader->set_uniform("model", model);
    shader->set_uniform("objectColor", color);
    shader->set_uniform("objectAlpha", alpha);

    if (type == "cube")
    {
        cube->draw();
    }
    else if (type == "bird")
    {
        bird->draw();
    }
    else if (type == "flower")
    {
        flower->draw();
    }
    else if (type == "magic_ball")
    {
        magicBall->draw();
    }
}

// ============================================================================
// init()
// ============================================================================

void init()
{
#if defined(__linux__) || defined(__APPLE__)
    std::string dirShader = "shaders/";
    std::string dirAsset = "asset/";
#else
    std::string dirShader = "shaders\\";
    std::string dirAsset = "asset\\";
#endif

    shader =
        new Shader(
            (dirShader + "easy.vert").c_str(),
            (dirShader + "easy.frag").c_str());

    cube =
        new Object(dirAsset + "cube.obj");

    bird =
        new Object(dirAsset + "bird.obj");

    flower =
        new Object(dirAsset + "flower.obj");

    magicBall =
        new Object(dirAsset + "magic_ball.obj");
}

// ============================================================================
// cleanup()
// ============================================================================

void cleanup()
{
    if (shader)
    {
        delete shader;
        shader = nullptr;
    }

    if (cube)
    {
        delete cube;
        cube = nullptr;
    }

    if (bird)
    {
        delete bird;
        bird = nullptr;
    }

    if (flower)
    {
        delete flower;
        flower = nullptr;
    }

    if (magicBall)
    {
        delete magicBall;
        magicBall = nullptr;
    }
}

// ============================================================================
// Main character
// ============================================================================

void drawSlime(
    const glm::mat4 &view,
    const glm::mat4 &projection)
{
    glm::vec3 cube_size[5] = {glm::vec3(3.0f, 2.7f, 3.0f),
                              glm::vec3(2.7f, 0.7f, 2.7f),
                              glm::vec3(2.1f, 0.7f, 2.1f),
                              glm::vec3(1.4f, 0.6f, 1.4f),
                              glm::vec3(0.9f, 0.5f, 0.9f)};
    glm::vec3 cube_color[5] = {glm::vec3(245.0f / 255.0f, 130.0f / 255.0f, 32.0f / 255.0f),
                               glm::vec3(45.0f / 255.0f, 90.0f / 255.0f, 39.0f / 255.0f),
                               glm::vec3(58.0f / 255.0f, 115.0f / 255.0f, 50.0f / 255.0f),
                               glm::vec3(72.0f / 255.0f, 140.0f / 255.0f, 60.0f / 255.0f),
                               glm::vec3(95.0f / 255.0f, 170.0f / 255.0f, 75.0f / 255.0f)};
    glm::vec3 cube_position[5] = {glm::vec3(0.0f, 1.35f, 0.0f),
                                  glm::vec3(0.0f, 1.70f, -0.15f),
                                  glm::vec3(0.0f, 0.70f, -0.60f),
                                  glm::vec3(0.0f, 0.65f, -0.75f),
                                  glm::vec3(0.0f, 0.55f, -0.65f)};

    float animation = 0.0f;
    if (spell.active)
    {
        animation = glm::clamp(spell.elapsed / 1.0f, 0.0f, 1.0f);
    }
    else if (spell.elapsed > 0.0f)
    {
        animation = glm::clamp(spell.elapsed / spell.duration, 0.0f, 1.0f);
    }

    glm::vec3 spell_color[5];
    spell_color[0] = cube_color[0];
    int round = 3;
    float cycle = animation * 4.0f * round;
    int color_shift = (int)cycle;
    float color_mix = cycle - color_shift;
    for (int i = 1; i < 5; i++)
    {
        int color_from = 1 + ((i - 1 + color_shift) % 4);
        int color_to = 1 + ((i - 1 + color_shift + 1) % 4);
        spell_color[i] = glm::mix(cube_color[color_from], cube_color[color_to], color_mix);
    }

    // TODO: Slime hopping.
    //
    // Suggested idea:
    // yOffset = H * abs(sin(w * t))
    //
    // Only apply hopping when the slime is moving horizontally on the ground.
    float yOffset = 0.0f;
    if (playerSlime.position.y <= 0.01f && playerSlime.movingHorizontally)
    {
        float H = 1.5f;
        float w = 8.0f;
        yOffset = H * abs(sin(w * playerSlime.hopTime));
    }

    /* squeeze */
    float area_scale = std::sqrt(1.0f / playerSlime.current_scale);
    glm::vec3 squeeze_scale(area_scale, playerSlime.current_scale, area_scale);

    // float squeeze_offset = 0.0f;
    // if (playerSlime.current_scale > 1.0f)
    // {
    //     squeeze_offset = cube_position[0].y * (playerSlime.current_scale - 1.0f);
    // }
    float squeeze_offset = -cube_position[0].y * (1.0f - playerSlime.current_scale);

    // TODO: Build the shared root transformation.
    //
    // The root should contain the character's world position and facing
    // rotation. All slime parts should be children of this root.
    /* body */
    glm::mat4 global(1.0f);
    global = glm::translate(global, playerSlime.position + glm::vec3(0.0f, yOffset + squeeze_offset, 0.0f));
    global = glm::rotate(global, glm::radians(playerSlime.angle), glm::vec3(0.0f, 1.0f, 0.0f));

    /* hair */
    static glm::vec3 hair_transformation(0.0f);
    glm::vec3 movement = playerSlime.movingHorizontally ? glm::vec3(0.0f, 0.3f, -0.5f) : glm::vec3(0.0f);
    hair_transformation = glm::mix(hair_transformation, movement, 0.1f);

    // TODO: Draw the slime using at least 3 cubes.
    //
    // Reuse the same root matrix for all children.
    // The facing direction must be visually distinguishable.
    glm::mat4 body_transformation = glm::translate(global, cube_position[0]);
    glm::mat4 body_model = glm::scale(body_transformation, cube_size[0] * squeeze_scale);
    drawModel("cube", body_model, view, projection, cube_color[0]);

    glm::mat4 hierarchy_transformation = body_transformation;
    for (int i = 1; i < 5; i++)
    {
        // squeeze
        glm::vec3 positoin = cube_position[i];
        positoin.y *= playerSlime.current_scale;

        // stratch
        glm::vec3 layer_hair_offset(hair_transformation.x, hair_transformation.y * (1.25f - 0.25f * i), hair_transformation.z * (0.26f * i));

        hierarchy_transformation = glm::translate(hierarchy_transformation, positoin + layer_hair_offset);
        glm::mat4 hair_model = glm::scale(hierarchy_transformation, cube_size[i] * squeeze_scale);
        drawModel("cube", hair_model, view, projection, spell_color[i]);
    }

    /* 表情 */
    glm::vec3 face_color(255.0f / 255.0f, 220.0f / 255.0f, 80.0f / 255.0f);
    float faceY = -0.675f * playerSlime.current_scale;
    float faceZ = 1.501f * area_scale;
    // 左眼
    glm::mat4 left_eye = glm::translate(body_transformation, glm::vec3(-1.0f * area_scale, faceY, faceZ));
    left_eye = glm::scale(left_eye, glm::vec3(0.4f * area_scale, 0.4f * playerSlime.current_scale, 0.001f));
    drawModel("cube", left_eye, view, projection, face_color);
    // 右眼
    glm::mat4 right_eye = glm::translate(body_transformation, glm::vec3(1.0f * area_scale, faceY, faceZ));
    right_eye = glm::scale(right_eye, glm::vec3(0.4f * area_scale, 0.4f * playerSlime.current_scale, 0.001f));
    drawModel("cube", right_eye, view, projection, face_color);
    // 嘴巴
    glm::mat4 mouth = glm::translate(body_transformation, glm::vec3(0.0f, faceY, faceZ));
    mouth = glm::scale(mouth, glm::vec3(0.6f * area_scale, 0.1f * playerSlime.current_scale, 0.001f));
    drawModel("cube", mouth, view, projection, face_color);
}

// ============================================================================
// Bird
// ============================================================================

void drawBird(
    float currentTime,
    const glm::mat4 &view,
    const glm::mat4 &projection)
{

    // TODO: Bird elliptical flight.
    //
    // Base position is the ellipse center:
    //     BIRD_BASE_POSITION
    //
    // Position:
    //     x = cx + rx * cos(t)
    //     z = cz + rz * sin(t)
    //
    // Facing:
    //     vx = -rx * sin(t)
    //     vz =  rz * cos(t)
    //     angle = atan2(vx, vz)
    float cx = BIRD_BASE_POSITION.x;
    float cz = BIRD_BASE_POSITION.z;
    float rx = 7.3;
    float rz = 3.7;

    glm::mat4 bird_model(1.0f);
    bird_model = glm::translate(bird_model, glm::vec3(cx + rx * cos(currentTime), BIRD_BASE_POSITION.y, cz + rz * sin(currentTime)));
    bird_model = glm::rotate(bird_model, atan2(-rx * sin(currentTime), rz * cos(currentTime)), glm::vec3(0.0f, 1.0f, 0.0f));
    bird_model = glm::scale(bird_model, BIRD_SCALE);
    drawModel("bird", bird_model, view, projection, glm::vec3(0.82f, 0.67f, 0.67f));
}

// ============================================================================
// Flowers
// ============================================================================

void drawFlowers(
    const glm::mat4 &view,
    const glm::mat4 &projection)
{

    // TODO: Render exactly three flowers.
    //
    // Requirements:
    // - Use FLOWER_POSITIONS.
    // - Use the fixed FLOWER_SCALE.
    // - Use visibly different colors.
    // - When the spell is cast, gradually tilt / fall.
    // - During recovery, gradually return upright.
    // - Rotate around the bottom / base of flower.obj.
    // - Falling direction depends on:
    //
    //       FLOWER_POSITIONS[i] - playerSlime.position
    //
    // - Ignore the vertical component when deciding falling direction.
    // - Each flower should generally fall away from the slime.
    glm::vec3 flower_color[3] = {glm::vec3(198 / 255.0, 240 / 255.0, 151 / 255.0),
                                 glm::vec3(237 / 255.0, 188 / 255.0, 199 / 255.0),
                                 glm::vec3(167 / 255.0, 187 / 255.0, 236 / 255.0)};

    float animation = 0.0f;
    if (spell.active)
    {
        animation = glm::clamp(spell.elapsed / 1.0f, 0.0f, 1.0f);
    }
    else if (spell.elapsed > 0.0f)
    {
        animation = glm::clamp(spell.elapsed / spell.duration, 0.0f, 1.0f);
    }

    float jump_height[3] = {0.0f, 0.0f, 0.0f};
    if (spell.active && animation > 0.0f && animation < 1.0f)
    {
        float max_height = 1.2f;
        float t = animation * 4.0f;

        if (t < 2.0f)
        {
            jump_height[0] = max_height * abs(sin(t * 3.1415f));
        }
        if (t >= 1.0f && t < 3.0f)
        {
            jump_height[1] = max_height * abs(sin((t - 1.0f) * 3.1415f));
        }
        if (t >= 2.0f && t <= 4.0f)
        {
            jump_height[2] = max_height * abs(sin((t - 2.0f) * 3.1415f));
        }
    }

    for (int i = 0; i < 3; i++)
    {
        glm::mat4 flower_model(1.0f);
        glm::vec3 current_position = FLOWER_POSITIONS[i] + glm::vec3(0.0f, jump_height[i], 0.0f);
        flower_model = glm::translate(flower_model, current_position);

        glm::vec3 fall_direction = FLOWER_POSITIONS[i] - playerSlime.position;
        fall_direction = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::normalize(fall_direction));
        fall_direction.y = 0.0f;
        if (animation >= 1.0f)
        {
            float fall_progress = glm::clamp((spell.elapsed - 1.0f) / (spell.duration - 1.0f), 0.0f, 1.0f);
            flower_model = glm::rotate(flower_model, glm::radians(50.0f * fall_progress), fall_direction);
        }
        else if (!spell.active && spell.elapsed > 0.0f)
        {
            float fall_progress = glm::clamp(spell.elapsed / (spell.duration - 1.0f), 0.0f, 1.0f);
            flower_model = glm::rotate(flower_model, glm::radians(50.0f * fall_progress), fall_direction);
        }

        flower_model = glm::scale(flower_model, FLOWER_SCALE);
        drawModel("flower", flower_model, view, projection, flower_color[i]);
    }
}

// ============================================================================
// Spell update
// ============================================================================

void updateSpell(
    float deltaTime)
{

    // TODO: Update the spell animation state.
    //
    // Required behavior:
    // - Main magic ball self rotation.
    // - Small magic balls self rotation.
    // - Small magic balls orbit around the main ball.
    // - Animation speed / timing are up to you.
    // - End / reset the spell so R can cast again later.

    if (spell.elapsed <= spell.duration && spell.active)
    {
        spell.elapsed += deltaTime;
    }
    else if (spell.elapsed > 0.0f)
    {
        spell.active = false;
        spell.elapsed -= deltaTime * 2;
    }
    if (spell.elapsed > 0.0f)
    {
        spell.mainSelfAngle += deltaTime;
        spell.smallSelfAngle -= deltaTime * 2;
        spell.orbitAngle += deltaTime * 1.5;
    }
}

glm::vec3 hsv_to_rgb(float h, float s, float v)
{
    float c = v * s;
    float x = c * (1.0f - abs(fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    float r = 0.0f, g = 0.0f, b = 0.0f;
    if (h >= 0 && h < 60)
    {
        r = c;
        g = x;
        b = 0;
    }
    else if (h >= 60 && h < 120)
    {
        r = x;
        g = c;
        b = 0;
    }
    else if (h >= 120 && h < 180)
    {
        r = 0;
        g = c;
        b = x;
    }
    else if (h >= 180 && h < 240)
    {
        r = 0;
        g = x;
        b = c;
    }
    else if (h >= 240 && h < 300)
    {
        r = x;
        g = 0;
        b = c;
    }
    else
    {
        r = c;
        g = 0;
        b = x;
    }

    return glm::vec3(r + m, g + m, b + m);
}

// ============================================================================
// Magic Casting
// ============================================================================

void drawMagic(
    const glm::mat4 &view,
    const glm::mat4 &projection)
{

    // TODO: Magic Ball appearance.
    //
    // - Press R to cast.
    // - Main + four small magic balls appear above the slime.
    // - alpha: 0 -> 1.
    // - Main ball self-rotates.
    // - Each small ball self-rotates.
    // - Four small balls orbit around the main ball.
    // - Four small balls should use visibly different colors.
    glm::vec3 ball_color[5] = {glm::vec3(0xE6 / 255.0f, 0x39 / 255.0f, 0x46 / 255.0f),
                               glm::vec3(0x2A / 255.0f, 0x9D / 255.0f, 0x8F / 255.0f),
                               glm::vec3(0xE9 / 255.0f, 0xC4 / 255.0f, 0x6A / 255.0f),
                               glm::vec3(0x45 / 255.0f, 0x7B / 255.0f, 0x9D / 255.0f),
                               glm::vec3(0x9B / 255.0f, 0x5D / 255.0f, 0xE5 / 255.0f)};
    glm::vec3 ball_position[5] = {MAGIC_BALL_ABOVE_POS,
                                  glm::vec3(0.0f, 0.0f, -ORBIT_RADIUS),
                                  glm::vec3(0.0f, 0.0f, ORBIT_RADIUS),
                                  glm::vec3(-ORBIT_RADIUS, 0.0f, 0.0f),
                                  glm::vec3(ORBIT_RADIUS, 0.0f, 0.0f)};

    // TODO: Hierarchical transformation.
    //
    // The four small balls must be children of the main magic hierarchy.
    //
    // Suggested structure:
    //
    // M_small =
    //     M_mainParent
    //     * R_orbit
    //     * T_offset
    //     * R_self
    //     * S
    //
    // Do NOT calculate small-ball positions independently in world space.

    // TODO: Move to Front.
    //
    // Before firing:
    // - Move the entire magic-ball hierarchy from above the slime
    //   to the front of the slime.
    // - Main + all four small balls move together.
    // - Rotate the entire hierarchy from horizontal to vertical.
    // - Therefore the small-ball orbit plane changes from
    //   horizontal to vertical.

    /* main */
    glm::mat4 transformation_w_beam(1.0f);
    transformation_w_beam = glm::translate(transformation_w_beam, playerSlime.position);
    transformation_w_beam = glm::rotate(transformation_w_beam, glm::radians(playerSlime.angle), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 big_transformation(1.0f);
    float animation = glm::clamp(spell.elapsed / 1.0f, 0.0f, 1.0f);
    if (spell.active)
    {
        glm::vec3 fall_position = glm::mix(MAGIC_BALL_ABOVE_POS, MAGIC_BALL_FRONT_POS, animation);
        transformation_w_beam = glm::translate(transformation_w_beam, fall_position);
        big_transformation = glm::rotate(transformation_w_beam, glm::radians(90.0f) * animation, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    else if (animation)
    {
        float back_progress = spell.elapsed / spell.duration;
        glm::vec3 fall_position = glm::mix(MAGIC_BALL_ABOVE_POS, MAGIC_BALL_FRONT_POS, back_progress);
        transformation_w_beam = glm::translate(transformation_w_beam, fall_position);
        big_transformation = glm::rotate(transformation_w_beam, glm::radians(90.0f) * back_progress, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    else
    {
        big_transformation = glm::translate(transformation_w_beam, ball_position[0]);
    }

    float alpha = glm::clamp(spell.elapsed / 0.1f, 0.0f, 1.0f);
    glm::mat4 big_model = glm::rotate(big_transformation, spell.mainSelfAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    big_model = glm::scale(big_model, glm::vec3(MAIN_BALL_SCALE));
    drawModel("magic_ball", big_model, view, projection, ball_color[0], alpha);

    /* small */
    big_transformation = glm::rotate(big_transformation, spell.orbitAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    for (int i = 1; i < 5; i++)
    {
        glm::mat4 small_transformation = glm::translate(big_transformation, ball_position[i]);
        small_transformation = glm::rotate(small_transformation, spell.smallSelfAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 small_model = glm::scale(small_transformation, glm::vec3(SMALL_BALL_SCALE));
        drawModel("magic_ball", small_model, view, projection, ball_color[i], alpha);
    }

    // TODO: Magic Beam.
    //
    // - Fire only after the magic hierarchy reaches the front.
    // - Use a cube.
    // - Start from the center of the main magic ball.
    // - Beam forward direction should align with slime facing direction.
    // - Beam length gradually increases.
    // - Use translation + scale.

    // glm::mat4 beam = transformation_w_beam;
    // beam = glm::translate(beam, glm::vec3(0.0f, 2.0f * (spell.elapsed - 0.9f), 0.0f));
    // beam = glm::scale(beam, glm::vec3(1.0f, 4.0f * (spell.elapsed - 0.9f), 1.0f));
    // float visible = (animation == 1.0f ? 1.0f : 0.0f);
    // if (spell.active)
    //     drawModel("cube", beam, view, projection, ball_color[0], visible);
    transformation_w_beam = glm::rotate(transformation_w_beam, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    float beam_start = 0.9f;
    float beam_duration = spell.elapsed - beam_start;

    if (beam_duration > 0.0f)
    {
        float beam_speed = 8.0f;
        float short_length = 0.3f;
        float max_duration = spell.duration - beam_start;
        float current_duration = spell.active ? beam_duration : max_duration;
        float total_length = current_duration * beam_speed;

        int short_cnt = (int)(total_length / short_length);
        float time = (float)glfwGetTime();

        int short_start = 0;
        if (!spell.active)
        {
            float end_progress = glm::clamp((max_duration - beam_duration) / max_duration, 0.0f, 1.0f);
            short_start = (int)(short_cnt * end_progress);
        }

        for (int i = short_start; i < short_cnt; i++)
        {
            float short_y = (i + 0.5f) * short_length;
            glm::mat4 short_model = transformation_w_beam;
            short_model = glm::translate(short_model, glm::vec3(0.0f, short_y, 0.0f));
            short_model = glm::scale(short_model, glm::vec3(1.0f, short_length, 1.0f));

            float h = fmod((time * 600.0f) - (i * 5.0f), 360.0f);
            if (h < 0.0f)
            {
                h += 360.0f;
            }
            float s = abs(0.7f * cos(time));
            glm::vec3 short_color = hsv_to_rgb(h, 1.0f - s, 1.0f);
            drawModel("cube", short_model, view, projection, short_color);
        }
    }
}
