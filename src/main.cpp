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


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//PPocetna vrednost kamere


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

    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);

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

    Model ourModel("resources/objects/chair2/Armchair.obj");

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
            0, 2, 3  // second triangle
    };
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

    Texture2D texture0("resources/textures/marble_tiles_difffuse2.jpg", GL_LINEAR, GL_REPEAT);
    Texture2D texture1("resources/textures/floor_specular2.png", GL_NEAREST, GL_CLAMP_TO_EDGE);
    floorShader.use();
    floorShader.setInt("texture_diffuse", 0);
    floorShader.setInt("texture_specular", 1);

    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindVertexArray(0);

    PointLight pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0f, 5.0f);
    pointLight.ambient = glm::vec3(0.4, 0.4, 0.2);
    pointLight.diffuse = glm::vec3 (0.6, 0.5, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);
    pointLight.c = 1.0f;
    pointLight.l = 0.09f;
    pointLight.q = 0.032f;



    while (!glfwWindowShouldClose(window)) {
        glm::vec3 lightPosition(2.0f, 1.0f, 2.0f);

        //Zbog razzlicitog broja fps, a da bi brzia bila ista bez obzira na broj prolazaka kroz petlju
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        // input
        // -----
        processInput(window);



        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        //chair
        ourShader.use();

        pointLight.position = glm::vec3(4.0f * cos(currentFrame), 4.0f, 4.0f * sin(currentFrame));
        ourShader.setVec3("pointLight.position", pointLight.position);
        ourShader.setVec3("pointLight.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight.specular", pointLight.specular);
        ourShader.setFloat("pointLight.c", pointLight.c);
        ourShader.setFloat("pointLight.l", pointLight.l);
        ourShader.setFloat("pointLight.q", pointLight.q);
        ourShader.setVec3("viewPosition", programState->camera.Position);

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(programState->camera.Zoom), (float) SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("view",view);
        ourShader.setMat4("projection", projection);

        float time = glfwGetTime();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model,glm::vec3(0.02f));
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);


//        model = glm::mat4(1.0f);
//        model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f));
//        model = glm::scale(model,glm::vec3(0.5f));
//        ourShader.setUniformMatrix4fv("model", glm::value_ptr(model));
//        ourModel.Draw(ourShader);
        //glBindVertexArray(0); // no need to unbind it every time

        //floor
        floorShader.use();

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

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        model = glm::scale(model,glm::vec3(30.0f));
        floorShader.setMat4("model", model);
        floorShader.setMat4("view",view);
        floorShader.setMat4("projection", projection);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


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
        ImGui::Begin("Test window");
        ImGui::Text("Hello world");
        ImGui::DragFloat("Drag slider", &f, 0.05f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float*)&programState->clearColor);
        ImGui::End();
    }


    // ImGui render
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


}