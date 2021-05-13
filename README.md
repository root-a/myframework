# myframework
A framework containing general code for writing applications with focus on AI, network and graphics.
## exts
External libraries
- [glew](https://github.com/nigels-com/glew)
- [glfw](https://github.com/glfw/glfw)
- [stb](https://github.com/nothings/stb) + [soil2](https://github.com/SpartanJ/soil2)
- [vector](https://github.com/rkk09c/Vector)
- [dirent](https://github.com/tronkko/dirent)
- [imgui](https://github.com/ocornut/imgui)
- [luajit](https://github.com/9chu/LuaJIT-cmake)

## libs
My Libraries
- App - base class for application
- Camera - implementation of 3D FPS Camera
- Component - base class for components that are build pieces for Object
- DrawablesDebug - basic implementation of OpenGL3.0+ figures, points, lines, planes
- DrawablesSystems - One drawcall systems for BBs, Lines, Points and any geometry
- ImguiWrapper - Includes opengl implementation so I don't have to include these files in each project
- GLwindow - class that encapsulate GLFW window functionality
- HalfEdgeMesh - Half-Edge Mesh lib for 3D meshes(generation, subdivision and conversion)
- HalfEdgeMesh2D - Half-Edge Mesh lib for 2D meshes(generation, subdivision and conversion)
- Light - components defining different types of lights
- Material - object's material that describes its properties
- Mesh - object's mesh keeps track of VAO and VBO's 
- MyMathLib - double and single floating point precision math lib, not sse yet
- Node - object's node used for updating the transforms in scenegraph
- OBJ - loads obj files, performs indexing and stores the indexed data
- Object - an object which can be placed in scene
- Particle - contains definitions of particle and particle system components
- PoolParty - memory pool allocator
- RigidBody - component encapsulating the rigidbody behaviour, integration, applying and reacting to impulses
- RenderBuffer - class for creating and using render buffers
- RenderElement - it's meant to be used in future as base class for render nodes
- Texture - wrapper class encapsulating OpenGL functionality of textures
- Script - simple class for loading/unloading/calling lua scripts
- Shader - class for storing shader related data
- ShaderBlock - library for handling of uniform and storage data with tools to map or add variables
## utils
Useful utilities
- CameraManager - manager for cameras, responsible for calculating and providing various combinations of M V P
- DebugDraw - manager for rendering basic 2D and 3D shapes (mostly useful with forward rendering)
- FBOManager - simple fbo manager for storing, deleting and updating of fbos
- Frustum - frustum culling manager, uses bounding spheres for culling
- GraphicsManager - manager for loading all assets like models, textures, shaders
- GraphicsStorage - storage for loaded assets, static assets only, for now
- LuaTools - some useful tools for debugging LUA, erorr checkin, traceback, stackdump etc.
- PhysicsManager - physics engine, collision detection, contacts generation, collision response
- Render - set of functions to render different passes
- SceneGraph - Scene-graph manager
- ShaderManager - manager for switching shaders and keeping track of active shader program
- Times - time class containing time related static variables
