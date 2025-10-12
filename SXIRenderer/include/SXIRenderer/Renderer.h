#pragma once

#include <vector>
#include <string>

#include <SXICore/Timing.h>
#include <SXICore/Types.h>

namespace sxi::renderer
{
    /**
     * @brief Initializes the renderer.
     * 
     * Must be destroyed before program end. Cannot be called more than once before
     * being destroyed.
     * 
     * @param uint32_t width: width of the window
     * @param uint32_t height: height of the window
     * @param bool releaseValidationLayers(=false): By default, validation layers are
     * 												disabled on release builds, set
     * 												this to true to force enable.
     */
    void init(u32, u32, bool=false);

    /**
     * @brief Pass in SPIRV shader code to construct graphics pipeline.
     * 
     * @param const std::vector<char>& vertCode: Bytes representing the contents of 
     *                                           the vertex shader of the desired
     *                                           pipeline.
     * @param const std::vector<char>& fragCode: Bytes representing the contents of 
     *                                           the fragment shader of the desired
     *                                           pipeline.
     */
    void addGraphicsPipeline(const std::vector<char>&, const std::vector<char>&);

    /**
     * @brief Pass in path to image to create a texture for models
     * 
     * @param const std::string& path: Path to an image file.
     */
    void addTexture(const std::string&);

    /**
     * @brief Pass in path to obj file to create a model
     * 
     * @param const std::string& path: Path to an obj file.
     */
    void addModel(const std::string&);

    /**
     * @brief Renders the frame to the screen.
     * 
     * Must be called only once per frame.
     */
    void render(const Time& time);

    /**
     * @brief Destroys the renderer and all associated data.
     */
    void destroy();
}