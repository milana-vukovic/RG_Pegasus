#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <valarray>
#include <learnopengl/shader.h>
#include "stb_image.h"
#include "rg/Texture2D.h"
#include "rg/Camera.h"
#include <learnopengl/model.h>
#include <learnopengl/mesh.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yofset);

unsigned int loadCubemap(vector<std::string> faces);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//Pocetna vrednost kamere


float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
bool firstMouse;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float shininess = 64.0f;

struct PointLight{
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float c;
    float l;
    float q;
};
struct DirLight{
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};


struct ProgramState{
    glm::vec3 clearColor = glm::vec3(0.1);
    bool ImGuiEnabled = false;

    Camera camera;

    void LoadFromDisk(std::string path);
    void SaveToDisk(std::string path);

};

void ProgramState::SaveToDisk(std::string path) {

    std::ofstream out(path);
    out << ImGuiEnabled << '\n'
        << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n'
        << camera.Yaw << '\n'
        << camera.Pitch << '\n';
}
void ProgramState::LoadFromDisk(std::string path) {

    std::ifstream in(path);
    if(in) {
        in >> ImGuiEnabled
            >> clearColor.r
            >> clearColor.g
            >> clearColor.b
            >> camera.Position.x
            >> camera.Position.y
            >> camera.Position.z
            >> camera.Front.x
            >> camera.Front.y
            >> camera.Front.z
            >> camera.Yaw
            >> camera.Pitch;
    }

}

ProgramState* programState;

void DrawImGui(ProgramState* programState);


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 3);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    //Sklanjamo kursor sa ekrana
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    //TASK01 buffer za dubinu

    //stbi_set_flip_vertically_on_load(false);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_MULTISAMPLE);

    programState = new ProgramState;
    programState->LoadFromDisk("resources/programState.txt");

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (programState->ImGuiEnabled){
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    //ImGui init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");


    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    Shader ourShader("resources/shaders/1.model_loading.vs", "resources/shaders/1.model_loading.fs");
    Shader floorShader("resources/shaders/floorShader.vs","resources/shaders/floorShader.fs");
    Shader cubemapShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader transparentShader("resources/shaders/blending.vs", "resources/shaders/blending.fs");

    Model ourModel("resources/objects/unicorn/flying-unicorn.obj");
    Model rockModel("resources/objects/rock/stone7_uv.obj");
    Model columnModel("resources/objects/OldColumn_OBJ/OldColumn.obj");

    //skybox

    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };


    //floor
    float floorVertices[] = {
            // positions                         //normals                           // texture coords
            1.0f,  0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    10.0f, 10.0f,
            1.0f, 0.0f, -1.0f,    0.0f, 1.0f, 0.0f,    10.0f, 0.0f,
            -1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 10.0f,
            -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,    0.0f, 0.0f
    };
    unsigned int floorIndices[] = {
            0, 1, 3, // first triangle
            0, 3, 2  // second triangle
    };


    float transparentVertices[] = {
            // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };
    // Floor
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);

    //position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //texture coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    Texture2D texture0("resources/textures/floor.jpg", GL_LINEAR, GL_REPEAT);
    Texture2D texture1("resources/textures/floor_specular2.png", GL_NEAREST, GL_CLAMP_TO_EDGE);
    floorShader.use();
    floorShader.setInt("texture_diffuse", 0);
    floorShader.setInt("texture_specular", 1);

    // transparent VAO
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    Texture2D transparentTexture("resources/textures/tree1.png", GL_LINEAR, GL_CLAMP_TO_EDGE);
    transparentShader.use();
    transparentShader.setInt("texture1", 0);
    vector<glm::vec3> transparentPosition
            {
                    glm::vec3(-13.5f, 7.2f, -20.0f),
                    //glm::vec3( -10.0f, 2.0f, 20.0f)
            };

    // Columns
    vector<glm::vec3> columnPosition
            {
                    glm::vec3(-8.0f, -2.0f, -3.0f),
                    glm::vec3( 8.0f, -2.0f, -3.0f),
                    glm::vec3( -14.2f, -2.0f, 8.0f),
                    glm::vec3(14.2f, -2.0f, 8.0f)
            };
    vector<float> columnRotation
            {
                    0.0f,
                    -180.0f,
                    -90,
                    -90
            };


    // Skybox
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof (skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    vector<std::string> faces{
            "resources/textures/sky3/px.png",
            "resources/textures/sky3/nx.png",
            "resources/textures/sky3/py.png",
            "resources/textures/sky3/ny.png",
            "resources/textures/sky3/pz.png",
            "resources/textures/sky3/nz.png"
    };
    unsigned int cubemapTexture = loadCubemap(faces);
    cubemapShader.setInt("skybox", 0);

    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindVertexArray(0);

    //Light data

    PointLight pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0f, 5.0f);
    pointLight.ambient = glm::vec3(0.6, 0.6, 0.4);
    pointLight.diffuse = glm::vec3 (0.7, 0.6, 0.7);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);
    pointLight.c = 1.0f;
    pointLight.l = 0.09f;
    pointLight.q = 0.032f;

    DirLight dirLight;
    dirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    dirLight.ambient = glm::vec3(0.2f);
    dirLight.diffuse = glm::vec3(0.1f);
    dirLight.specular = glm::vec3(0.1f);



    while (!glfwWindowShouldClose(window)) {
        glm::vec3 lightPosition(2.0f, 1.0f, 2.0f);

        //Zbog razlicitog broja fps, a da bi brzina bila ista bez obzira na broj prolazaka kroz petlju
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        // input
        // -----
        processInput(window);


        // render
        // ------
        //glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(programState->camera.Zoom), (float) SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();

        // Pegasus
        ourShader.use();
        // Point light
        //pointLight.position = glm::vec3(4.0f * cos(currentFrame), 4.0f, 4.0f * sin(currentFrame));
        pointLight.position = glm::vec3(0.0f, 3.0f, 3.0f);
        ourShader.setVec3("pointLight.position", pointLight.position);
        ourShader.setVec3("pointLight.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight.specular", pointLight.specular);
        ourShader.setFloat("pointLight.c", pointLight.c);
        ourShader.setFloat("pointLight.l", pointLight.l);
        ourShader.setFloat("pointLight.q", pointLight.q);
        ourShader.setVec3("viewPosition", programState->camera.Position);

        //Dirlight
        ourShader.setVec3("dirLight.direction", dirLight.direction);
        ourShader.setVec3("dirLight.ambient", dirLight.ambient);
        ourShader.setVec3("dirLight.diffuse", dirLight.diffuse);
        ourShader.setVec3("dirLight.specular", dirLight.specular);


        ourShader.setMat4("view",view);
        ourShader.setMat4("projection", projection);

        float time = glfwGetTime();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -3.5f, 5.0f));
        model = glm::rotate(model, glm::radians(280.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model,glm::vec3(0.5f));
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);


        // rock
        time = glfwGetTime();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(10.0f, -1.7f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model,glm::vec3(0.5f, 2.0f, 2.0f));
        ourShader.setMat4("model", model);
        rockModel.Draw(ourShader);

        // column
        glDisable(GL_CULL_FACE);
        ourShader.use();

        for (unsigned int i = 0; i < columnPosition.size(); i++){
            model = glm::mat4(1.0f);
            model = glm::translate(model, columnPosition[i]);
            model = glm::rotate(model, glm::radians(columnRotation[i]), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model,glm::vec3(0.1f ));
            ourShader.setMat4("model", model);
            columnModel.Draw(ourShader);
        }

        //floor
        floorShader.use();
        glBindVertexArray(VAO);

        texture0.activeTexture(GL_TEXTURE0);
        texture0.bindTexture();
        texture1.activeTexture(GL_TEXTURE1);
        texture1.bindTexture();

        floorShader.setVec3("pointLight.position", pointLight.position);
        floorShader.setVec3("pointLight.ambient", pointLight.ambient);
        floorShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        floorShader.setVec3("pointLight.specular", pointLight.specular);
        floorShader.setFloat("pointLight.c", pointLight.c);
        floorShader.setFloat("pointLight.l", pointLight.l);
        floorShader.setFloat("pointLight.q", pointLight.q);
        floorShader.setVec3("viewPosition", programState->camera.Position);

        floorShader.setVec3("dirLight.direction", dirLight.direction);
        floorShader.setVec3("dirLight.ambient", dirLight.ambient);
        floorShader.setVec3("dirLight.diffuse", dirLight.diffuse);
        floorShader.setVec3("dirLight.specular", dirLight.specular);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 2.0f));
        model = glm::scale(model,glm::vec3(20.0f));
        floorShader.setMat4("model", model);
        floorShader.setMat4("view",view);
        floorShader.setMat4("projection", projection);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


        // Transparent objects
        glDisable(GL_CULL_FACE);
        glBindVertexArray(transparentVAO);
        transparentShader.use();
        transparentShader.setMat4("view",view);
        transparentShader.setMat4("projection", projection);
        transparentTexture.activeTexture(GL_TEXTURE0);
        transparentTexture.bindTexture();

        for (unsigned int i = 0; i < transparentPosition.size(); i++){
            model = glm::mat4(1.0f);
            model = glm::translate(model, transparentPosition[i]);
            model = glm::scale(model,glm::vec3(30.0f,22.0f, 0.0f ));
            transparentShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glEnable(GL_CULL_FACE);


        //TODO: Draw Skybox last
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        cubemapShader.use();
        cubemapShader.setMat4("view", glm::mat4(glm::mat3(view)));
        cubemapShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        if(programState->ImGuiEnabled){
            DrawImGui(programState);
        }


       glfwSwapBuffers(window);
       glfwPollEvents();
    }

    programState->SaveToDisk("resources/programState.txt");

    // ImGui Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    //ourShader.deleteProgram();
    delete programState;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    float cameraSpeed = 2.5 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    // Pomeranje
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){

        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){

        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){

        programState->camera.ProcessKeyboard(RIGHT, deltaTime);

    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){

    //Kada hocemo da samo jednom registruje dodir

    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
    // Promena boje
    if(key == GLFW_KEY_C && action == GLFW_PRESS){
        glClearColor(0.1, 0.3, 0.4, 1.0);
    }
    if(key == GLFW_KEY_UP && action == GLFW_PRESS){
        //p = std::min(p+0.1, 1.0);
        shininess += 2.0;
    }
    if(key == GLFW_KEY_DOWN && action == GLFW_PRESS){
        shininess -= 2.0;
    }
    if(key == GLFW_KEY_F1 && action == GLFW_PRESS){
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        //Da bi nam se pojavio kursor
        if(programState->ImGuiEnabled){
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else{
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    if(key == GLFW_KEY_I && action == GLFW_PRESS){
        glEnable(GL_MULTISAMPLE);
    }
    if(key == GLFW_KEY_O && action == GLFW_PRESS){
        glDisable(GL_MULTISAMPLE);
    }
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos){

    //Bitno nam je za koliko smo se pomerili
    if(firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; //Zbog toga sto koordinate na ekranu idu iz gornjeg levog ugla
    lastX = xpos;
    lastY = ypos;

    if(programState->ImGuiEnabled == false) {
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
    }

}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){

    programState->camera.ProcessMouseScroll(yoffset);

}
void DrawImGui(ProgramState* programState){

    // ImGui Frame init
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        static float f = 0.0f;
        ImGui::Begin("Camera info");
        auto &c = programState->camera;
        ImGui::Text("Camera position x:%f y:%f z%f", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("Camera pitch: %f", c.Pitch);
        ImGui::Text("Camera yaw: %f", c.Yaw);
        //ImGui::DragFloat("Drag slider", &f, 0.05f, 0.0, 1.0);
        //ImGui::ColorEdit3("Background color", (float*)&programState->clearColor);
        ImGui::End();
    }


    // ImGui render
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
unsigned int loadCubemap(vector<std::string> faces){

    stbi_set_flip_vertically_on_load(false);
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    unsigned char* data;

    for(int i = 0; i < faces.size(); ++i){

        data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if(data){
            GLenum format;
            if(nrChannels == 1){
                format = GL_RED;
            }
            else if(nrChannels == 3){
                format = GL_RGB;
            }
            else if(nrChannels == 4){
                format = GL_RGBA;
            }
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        }else{
            std::cerr << "Failed to load cube map texture faces\n";
            return -1;
        }
        stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}