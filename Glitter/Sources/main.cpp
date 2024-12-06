// Local Headers
#include "glitter.hpp"
#include <Shader.hpp>

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <direct.h>

Params params;
float dt = 0.0f;                  
Timer main_timer;
GUI* gui_pointer;

// Callbacks
void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

int main(int argc, char * argv[]) {

    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGL", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    // OpenGL Callback Functions
    glfwSetCursorPosCallback(mWindow, CursorPositionCallback);
    glfwSetMouseButtonCallback(mWindow, MouseButtonCallback);
    glfwSetKeyCallback(mWindow, KeyboardCallback);

    // OpenCL initialization
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    if (all_platforms.size() == 0) {
        std::cout << " No platforms found. Check OpenCL installation!\n";
        exit(1);
    }
    cl::Platform default_platform = all_platforms[0];
    std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";

    std::vector<cl::Device> all_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if (all_devices.size() == 0) {
        std::cout << " No devices found.\n";
        exit(1);
    }

    default_device = all_devices[0];
    std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";

    cl_context_properties properties[] =
    {
      CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
      CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
      CL_CONTEXT_PLATFORM, (cl_context_properties)default_platform(),
      NULL
    };

    cl_int err = CL_SUCCESS;
    context = clCreateContext(properties, 1, &default_device(), NULL, NULL, &err);

    if (err != CL_SUCCESS) {
        std::cout << "Error creating context" << " " << err << "\n";
        //exit(-1);
    }

    queue = cl::CommandQueue(context, default_device);
    
    // Read kernel source
    char buffer[1024];
    getcwd(buffer, 1024);
    //std::string kernel_char(buffer);
    //kernel_char += "\\..\\Glitter\\Sources\\gpu_src\\test.cl";
    //kernel_char += "\\..\\Glitter\\Sources\\gpu_src\\mandel.cl";
    GetModuleFileName(NULL, buffer, sizeof(buffer));
    std::string kernel_char(buffer);
    kernel_char += "\\..\\mandel.cl";
    kernel_source = ReadFile2(kernel_char.c_str());
    sources.push_back({ kernel_source.c_str(), kernel_source.length() });

    // Build program and compile
    program = cl::Program(context, sources);

    if (program.build({ default_device }) != CL_SUCCESS)
    {
        std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
        exit(1);
    }

    // Prepare buffers
    debug_buffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * mWidth * mHeight);

    // Setup OpenGL Buffers
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(hardcoded_vertices), hardcoded_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // OpenGL texture
    unsigned int gl_texture;
    glGenTextures(1, &gl_texture);
    glBindTexture(GL_TEXTURE_2D, gl_texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;

    /*std::string tex_char(buffer);
    tex_char += "\\..\\textures\\sample.jpg";
    unsigned char* data = stbi_load(tex_char.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);*/

    height = mHeight;
    width = mWidth;

    // Set empty
    std::vector<GLubyte> emptyData(mWidth * mHeight * 4, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &emptyData[0]);

    glGenerateMipmap(GL_TEXTURE_2D);

    target_texture = clCreateFromGLTexture(context(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, gl_texture, &err);
    std::cout << "Created CL Image2D with err:\t" << err << std::endl;
    copy_texture = clCreateFromGLTexture(context(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, gl_texture, &err);
    std::cout << "Created CL Image2D with err:\t" << err << std::endl;

    // Flush GL queue        
    glFinish();
    glFlush();

    // Acquire shared objects
    err = clEnqueueAcquireGLObjects(queue(), 1, &target_texture(), 0, NULL, NULL);
    err = clEnqueueAcquireGLObjects(queue(), 1, &copy_texture(), 0, NULL, NULL);
    std::cout << "Acquired GL objects with err:\t" << err << std::endl;

    // Set up kernels
    //tester = cl::Kernel(program, "tex_test");
    //mandeler = cl::Kernel(program, "Mandel");
    mandeler = cl::Kernel(program, "MandelSmooth");
    filter = cl::Kernel(program, "GaussianFilter");
    cl::NDRange global_test(width, height);
    //tester(cl::EnqueueArgs(queue, global_test), target_texture).wait();
    mandeler(cl::EnqueueArgs(queue, global_test), target_texture, 0, 0, 1.0f).wait();

    // We have to generate the mipmaps again!!!
    glGenerateMipmap(GL_TEXTURE_2D);

    // Release shared objects                                                          
    err = clEnqueueReleaseGLObjects(queue(), 1, &target_texture(), 0, NULL, NULL);
    std::cout << "Releasing GL objects with err:\t" << err << std::endl;
    err = clEnqueueReleaseGLObjects(queue(), 1, &copy_texture(), 0, NULL, NULL);
    std::cout << "Releasing GL objects with err:\t" << err << std::endl;

    // Flush CL queue
    err = clFinish(queue());
    std::cout << "Finished CL queue with err:\t" << err << std::endl;

    // Input information
    std::cout << "\n\nW or S: zoom (scale)\nA or D: offset horizontally\nE or Q: offset vertically\nR: reset parameters\nF: enable/disable filtering\n \
        P: play/pause animation\n] or [: increase/decrease animation speed" << std::endl;

    // Initialize our GUI
    GUI gui = GUI(mWindow, main_timer);
    gui.Init();
    gui_pointer = &gui;

    // Rendering Loop
    float time = glfwGetTime();
    while (glfwWindowShouldClose(mWindow) == false) {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        dt = glfwGetTime() - time;
        time = glfwGetTime();

        // Update Timer
        main_timer.UpdateTime();

        // Flush GL queue        
        glFinish();
        glFlush();

        // Acquire shared objects
        err = clEnqueueAcquireGLObjects(queue(), 1, &target_texture(), 0, NULL, NULL);
        err = clEnqueueAcquireGLObjects(queue(), 1, &copy_texture(), 0, NULL, NULL);

        /*const float dx = cos(glfwGetTime());
        const float dy = 0;
        const float scale = glfwGetTime();*/

        // Animation stuff
        float scale = params.scale;
        if (params.playAnimation)
        {
            params.animationTime += dt * params.animationSpeed;
            //scale = 1.0f + params.animationTime;
            params.scale = 1.0f + params.animationTime;
        }

        //mandeler(cl::EnqueueArgs(queue, global_test), target_texture, dx, dy, scale).wait();
        mandeler(cl::EnqueueArgs(queue, global_test), target_texture, params.dx, params.dy, scale).wait();

        // Image Copy parameters
        static const size_t imageSize[3] = { width, height, 1 };
        static const size_t imageOrigin[3] = { 0, 0, 0 };

        if (params.filterOn)
        {
            filter(cl::EnqueueArgs(queue, global_test), target_texture, copy_texture).wait();
            clEnqueueCopyImage(queue(), copy_texture(), target_texture(), imageOrigin, imageOrigin, imageSize, 0, NULL, NULL);
        }

        // We have to generate the mipmaps again!!!
        glGenerateMipmap(GL_TEXTURE_2D);

        // Release shared objects                                                          
        err = clEnqueueReleaseGLObjects(queue(), 1, &target_texture(), 0, NULL, NULL);
        err = clEnqueueReleaseGLObjects(queue(), 1, &copy_texture(), 0, NULL, NULL);

        // Flush CL queue
        err = clFinish(queue());

        // bind Texture
        glBindTexture(GL_TEXTURE_2D, gl_texture);

        // Bind Framebuffer
        static GLuint fboId = 0;
        glGenFramebuffers(1, &fboId);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);

        // Render texture directly
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, gl_texture, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
            GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // Render GUI
        if (gui.gui_enabled)
            gui.Render();

        // Reset input flags
        gui.ResetInputFlags();

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }
    
    // Cleanup GUI
    gui.Cleanup();

    glfwTerminate();
    
    return EXIT_SUCCESS;
}

/// <summary>
/// Callback function for mouse cursor movement
/// </summary>
/// <param name="window"></param>
/// <param name="xpos"></param>
/// <param name="ypos"></param>
void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    gui_pointer->MousePositionUpdate(xpos, ypos);

    //ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
}

/// <summary>
/// Callback function for mouse button press
/// </summary>
/// <param name="window"></param>
/// <param name="button"></param>
/// <param name="action"></param>
/// <param name="mods"></param>
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (!gui_pointer->clicked && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        std::cout << "MOUSE CLICK on x: " << gui_pointer->mouse_xpos << " y: " << gui_pointer->mouse_ypos << std::endl;
        gui_pointer->clicked = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        std::cout << "MOUSE RELEASE on x: " << gui_pointer->mouse_xpos << " y: " << gui_pointer->mouse_ypos << std::endl;
        gui_pointer->clicked = false;
    }

    //ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}


/// <summary>
/// Callback Function for keyboard button press
/// </summary>
/// <param name="window"></param>
/// <param name="key"></param>
/// <param name="scancode"></param>
/// <param name="action"></param>
/// <param name="mods"></param>
void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if ((key == GLFW_KEY_W || key == GLFW_PRESS) && action == GLFW_PRESS)
        params.scale += force * dt;
    else if ((key == GLFW_KEY_S || key == GLFW_KEY_DOWN) && action == GLFW_PRESS)
        params.scale -= force * dt;
    else if ((key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) && action == GLFW_PRESS)
        params.dx += force * dt;
    else if ((key == GLFW_KEY_A || key == GLFW_KEY_LEFT) && action == GLFW_PRESS)
        params.dx -= force * dt;
    else if (key == GLFW_KEY_E && action == GLFW_PRESS)
        params.dy += force * dt;
    else if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        params.dy -= force * dt;
    else if (key == GLFW_KEY_F && action == GLFW_PRESS)
        params.filterOn = !params.filterOn;
    else if (key == GLFW_KEY_R && action == GLFW_PRESS)
        params.Reset();
    else if (key == GLFW_KEY_P && action == GLFW_PRESS)
        params.playAnimation = !params.playAnimation;
    else if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS)
        params.animationSpeed = (params.animationSpeed - 0.2f < 0.1f) ? 0.1f : params.animationSpeed - 0.2f;
    else if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS)
        params.animationSpeed += 0.2f;
    // Enable/Disable Cursor
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        if (gui_pointer->cursor_enabled)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);

        gui_pointer->cursor_enabled = !gui_pointer->cursor_enabled;
    }
    // Enable/Disable GUI
    else if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    {
        gui_pointer->gui_enabled = !gui_pointer->gui_enabled;
    }
}
