# myframework
A framework containing general code for writing applications with focus on AI, network and graphics.
## exts
External libraries
- [glew](https://github.com/nigels-com/glew)
- [glfw](https://github.com/glfw/glfw)
- [stb_image](https://github.com/nothings/stb)
- [vector](https://github.com/rkk09c/Vector)
- [dirent](https://github.com/tronkko/dirent)
- [imgui](https://github.com/ocornut/imgui)
- [luajit](http://luajit.org/download.html)

## libs
My Libraries
- App - base class for application
- Camera - implementation of 3D FPS Camera
- Component - base class for components that are build pieces for Object
- Drawables - basic implementation of OpenGL3.0+ figures, points, lines, planes
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
- Texture - wrapper class encapsulating OpenGL functionality of textures

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
- Scene - Scene-graph manager
- ShaderManager - manager for switching shaders and keeping track of active shader program
- Time - time class containing time related static variables
