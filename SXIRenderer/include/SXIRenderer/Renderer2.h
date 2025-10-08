#pragma once

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
     * 
     * @throws InitializationException: if something went wrong during initialization
     * 								    or if already initialized.
     */
    void init(u32, u32, bool=false);

    /**
     * @brief Destroys the renderer and all associated data.
     */
    void destroy();
}