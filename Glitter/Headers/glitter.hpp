// Preprocessor Directives
#ifndef GLITTER
#define GLITTER
#pragma once

// System Headers
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <CL/cl.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <Timer.hpp>
#include <GUI.hpp>

// Reference: https://github.com/nothings/stb/blob/master/stb_image.h#L4
// To use stb_image, add this in *one* C++ source file.
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct Params {
    float dx = 0;
    float dy = 0;
    float scale = 1.0f;
    bool filterOn = false;
    bool playAnimation = false;
    float animationTime = 0.0f;
    float animationSpeed = 1.0f;

    void Reset()
    {
        dx = 0;
        dy = 0;
        scale = 1.0f;
        filterOn = false;
        playAnimation = false;
        animationTime = 0.0f;
        animationSpeed = 1.0f;
    }
};

// Define Some Constants
int mWidth = 1920;
int mHeight = 1080;
const float force = 10.0f;

// **********************************************************************************
// OpenCL section
// **********************************************************************************

cl::Device default_device;
cl::Context context;
std::string kernel_source;
cl::Program::Sources sources;
cl::CommandQueue queue;
cl::Program program;
cl::Buffer test_buffer;
cl::Buffer debug_buffer;
cl::Kernel test_kernel;
cl::Kernel mandel_Kernel;
cl::Kernel filter_Kernel;
cl::NDRange global_tex(mWidth, mHeight);

float hardcoded_vertices[] = {
    // positions          // colors           // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
};

unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

cl::make_kernel<cl::Image2D> tester(test_kernel);
cl::make_kernel<cl::Image2D, float, float, float> mandeler(mandel_Kernel);
cl::make_kernel<cl::Image2D, cl::Image2D> filter(filter_Kernel);

cl::Image2D target_texture;
cl::Image2D copy_texture;

std::string ReadFile2(const char* f_name = "kernels.cl")
{
    std::string kernel_code;
    std::ifstream kernel_file;
    // ensure ifstream objects can throw exceptions:
    kernel_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        
        kernel_file.open(f_name);
        std::stringstream kernel_stream;
        
        kernel_stream << kernel_file.rdbuf();
        
        kernel_file.close();
        
        kernel_code = kernel_stream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }

    // return char sequence
    return kernel_code.c_str();
}

#endif //~ Glitter Header
