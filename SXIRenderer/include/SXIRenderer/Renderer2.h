
namespace sxi::renderer
{
    /**
     * @brief Initializes the renderer.
     * 
     * Must be destroyed before program end. Cannot be called more than once before
     * being destroyed.
     * 
     * @param bool releaseValidationLayers(=false): By default, validation layers are
     * 												disabled on release builds, set
     * 												this to true to force enable.
     * 
     * @throws InitializationException: if something went wrong during initialization
     * 								    or if already initialized.
     */
    void init(bool=false);

    /**
     * @brief Destroys the renderer and all associated data.
     */
    void destroy();
}