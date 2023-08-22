# CGL diff with chuck main as of 8/21  

**Goal: rebase cgl onto newest chuck, map out immediate next step**
- importantly: editor or no editor? I think no, but give tools to create own inspector for rapid iteration
- https://www.amulet.xyz/doc/ 
  - express scenegraph construction syntax
  - also supports transformation nodes, shader nodes, etc. 
    - similar to Ge's GG engine (where did Ge get the idea?)

> 4c1d6a5 2023-05-02 | update webchuck build to 1.5.0.0-rc1 (origin/main, origin/HEAD) [Ge Wang]

`git diff --name-only main...`

`git diff main <filename>`

[Fork pull request workflow](https://gist.github.com/james-priest/74188772ef2a6f8d7132d0b9dc065f9c)

[Merge/rebase tutorial](https://www.atlassian.com/git/tutorials/merging-vs-rebasing)

## CGL rendering engine

    src/CGL/Includes.h
    src/CGL/IndexBuffer.cpp
    src/CGL/IndexBuffer.h
    src/CGL/Main.cpp
    src/CGL/Renderer.cpp
    src/CGL/Renderer.h
    src/CGL/RendererState.cpp
    src/CGL/RendererState.h
    src/CGL/Shader.cpp
    src/CGL/Shader.h
    src/CGL/Texture.cpp
    src/CGL/Texture.h
    src/CGL/Util.h
    src/CGL/VertexArray.cpp
    src/CGL/VertexArray.h
    src/CGL/VertexBuffer.cpp
    src/CGL/VertexBuffer.h
    src/CGL/VertexBufferLayout.h
    src/CGL/Window.cpp
    src/CGL/Window.h

    // shaders
    src/CGL/res/shaders/BasicFrag.glsl
    src/CGL/res/shaders/BasicLightingFrag.glsl
    src/CGL/res/shaders/BasicLightingVert.glsl
    src/CGL/res/shaders/BasicVert.glsl
    src/CGL/res/shaders/LightSourceFrag.glsl
    src/CGL/res/shaders/NormalFrag.glsl
    src/CGL/res/shaders/include/Globals.glsl
    src/CGL/res/shaders/include/Lighting.glsl

    // default textures
    src/CGL/res/textures/awesomeface.png
    src/CGL/res/textures/chuck-logo.png
    src/CGL/res/textures/container.jpg
    src/CGL/res/textures/default-black.png
    src/CGL/res/textures/default-white.png
    src/CGL/res/textures/wall.jpg

    // scenegraph (ideally this is decoupled from renderer)
    src/CGL/scenegraph/Camera.cpp
    src/CGL/scenegraph/Camera.h
    src/CGL/scenegraph/Command.cpp
    src/CGL/scenegraph/Command.h
    src/CGL/scenegraph/Geometry.cpp
    src/CGL/scenegraph/Geometry.h
    src/CGL/scenegraph/Group.h
    src/CGL/scenegraph/Light.cpp
    src/CGL/scenegraph/Light.h
    src/CGL/scenegraph/Material.cpp
    src/CGL/scenegraph/Material.h
    src/CGL/scenegraph/Mesh.cpp
    src/CGL/scenegraph/Mesh.h
    src/CGL/scenegraph/Scene.cpp
    src/CGL/scenegraph/Scene.h
    src/CGL/scenegraph/SceneGraphNode.cpp
    src/CGL/scenegraph/SceneGraphNode.h
    src/CGL/scenegraph/SceneGraphObject.cpp
    src/CGL/scenegraph/SceneGraphObject.h
    src/CGL/scenegraph/TransformationStack.cpp
    src/CGL/scenegraph/TransformationStack.h

    // misc
    src/CGL/scenegraph/notes.txt
    src/CGL/scenegraph/tmp.txt

## Chuck Core

    // load cgl module
    src/core/chuck_compile.cpp  

    // CGL dll module (how does the dll_query stuff work exactly?)
    src/core/ulib_cgl.cpp
    src/core/ulib_cgl.h

## Chuck Host

    // fork main thread (TODO use main hook)
    src/host/chuck_main.cpp

    // window helper class
    src/host/chuck_window.cpp
    src/host/chuck_window.h

---

>>> 3rd Party

## glm

    src/vendor/glm/*

## imgui

    src/vendor/imgui/imconfig.h
    src/vendor/imgui/imgui.cpp
    src/vendor/imgui/imgui.h
    src/vendor/imgui/imgui_demo.cpp
    src/vendor/imgui/imgui_draw.cpp
    src/vendor/imgui/imgui_impl_glfw.cpp
    src/vendor/imgui/imgui_impl_glfw.h
    src/vendor/imgui/imgui_impl_opengl3.cpp
    src/vendor/imgui/imgui_impl_opengl3.h
    src/vendor/imgui/imgui_impl_opengl3_loader.h
    src/vendor/imgui/imgui_internal.h
    src/vendor/imgui/imgui_tables.cpp
    src/vendor/imgui/imgui_widgets.cpp
    src/vendor/imgui/imstb_rectpack.h
    src/vendor/imgui/imstb_textedit.h
    src/vendor/imgui/imstb_truetype.h
    src/vendor/imgui/main.cpp

## shadinclude

    src/vendor/shadinclude/Shadinclude.hpp

## stb_image

    src/vendor/stb/stb_image.cpp
    src/vendor/stb/stb_image.h

## glfw (window management library, mouse/keyboard input)

    src/host/GLFW/glfw3.lib
    src/host/GLFW/include/GLFW/glfw3.h
    src/host/GLFW/include/GLFW/glfw3native.h

## glad (OpenGL Headers and function pointers to gpu driver OpenGL implementation)

    src/host/glad/include/KHR/khrplatform.h
    src/host/glad/include/glad/glad.h
    src/host/glad/src/glad.c



____________________________________________________________

# Documentation

## Synchronization

**Relevant files**

- `chuck_window.cpp`
- `ulib_cgl.cpp`
  - `CGL::WaitOnUpdateDone()` 
    - blocks render thread, waiting on a condition variable for chuck to finish its update logic and trigger the next render.
    - Called on graphics side, in window display loop
  - `CGL::Render()`
    - Called by Chuck script, after update() logic is done for the frame (at the end of chuck game loop)
    - basically just wakes up the render thread via a condition var
  - `CGL::PushCommand()`

Window Display loop
```cpp
while (windowIsActive) {
    CGL::WaitOnUpdateDone();  // wait for chuck script to trigger render
    { // critical section: swap command queus
        CGL::SwapCommandQueues();
    }
    CglEvent::Broadcast(CglEventType::CGL_UPDATE);
    // now apply changes from the command queue chuck is NO Longer writing to 
    CGL::FlushCommandQueue(scene, false);

    // now renderer can work on drawing the copied scenegraph ===
    renderer.Clear();
    renderer.RenderScene(&scene, &camera);
}
```

in chuck land
```cpp
fun void GameLoop(){
	CGL.Render(); // kick of the renderer
	while (true) {
		UpdateEvent => now; // will deadlock if UpdateEvent is broadcast before this shred begins waiting 
            // also important:: this is what passes time!!!
        update();
		CGL.Render();  // unblocks render thread
        
        
        // CANNOT PASS TIME HERE, OTHERWISE WILL DEADLOCK
	}
} 
```

Major problem! Possible deadlock if chuck begins waiting on UpdateEvent AFTER render thread has already broadcast CGL_UPDATE
- soln: in implementation of CGL.Render(), put shred on event queue before waking up renderer
- OR: like the cpp condition var, make UpdateEvent a flag, rather than a notif.
  - either create a chuck Event type that will immediately unblock if it was broadcast before (i remember nick asking for this?)
  - OR replease with a cpp call CGL.WaitOnRenderDone() that waits on a second condition var
    - and then collapse CGL.REnder() and CGL.WaitOnSwap() into one call 
    - actually this won't work because we need something that can PASS TIME  in the chuck gameloop. Option 1 here seems best
- // solution is to somehow get on UpdateEvent queue before calling Render(), something like CGL.Render() => now;
- also having both update and frame event is redundant, we only need 1 event broadcoast once per frame to keep graphics and audio in lockstep

Problem: how to expose these synchronization mechanisms through a chugin?
- does the scenegraph + double-buffered command queue live entirely within the chugin? how does that affect performance?
- but no, to support the arrow syntax `->` doesn't it need to live in chuck core?
- then how to we communicate this info to a chugin? need to pass it through host API somehow?


## Workflow for Adding to the CGL Graphics API

- add API call to ulib_cgl dll
- add a corresponding scenegraph command
- Implement in renderer

## Scenegraph Architecture

**Scenegraph Class Hierarchy**
- `SceneGraphNode` (base class for all entities including meshObjects, geometry data, materials, textures...used to assign unique IDs (node IDs are shared across the 2 scenegraph copies))
  - `SceneGraphObject` anything with a transform
  - `Material` (material is shader + uniforms)
    - shader type
    - local shader uniforms  
    - `genUpdate()` and `ApplyUpdate()` to work with Scenegraph Update Commands
    - `Clone()` for Scenegraph create commands
  - `Geometry`
    - vertex positons, normals, texCoords
  - `Texture`
    - TODO


- `SceneGraphCommands` Command objects to diff the scenegraph
  - Create commands (create new node in scenegrpah)
  - Edge (modify scenegraph parent/child relations)
  - Update commands (change params of existing material / transform / geometry)

- `Shaders`
  - this one is kind of a mess, has a dependency with Scenegraph Material, also dependent rn on the underlying graphics API (openGL)
  - ideally we want
    - default shaders
    - users to be able to create their own shaders, with access to global uniforms supplied during the render pass (MVP matrix, normal mat, eyePos, time etc) as well as custom local uniforms
  - so where do shaders live? as part of scenegraph or in decoupled renderer? probably renderer, because it's gpu and graphicsAPI dependent

## Rendering Engine
- For each scenegraph type, create corresponding renderer type that can be drawn with underlying graphics impl
  - e.g. SceneGraphMaterial --> RendererMaterial
  - Mesh --> RendererMesh
- draws the abstract, decoupled CGL scenegraph

## System Diagram

Major components
1. Rendering Engine (ideally completely decoupled from chuck core)
2. Chugl Scenegraph + Scenegraph commands
3. Synchro mechanism

## Next Features

______________________

# How to Decouple

Ideally CGL API is independent of the rendering engine implementation
- so scenegraph + API calls in chuck
- rendering engines as chugin

But problem: the synchronization mechanism bridges both graphics/audio land. If we do the architecture above, the components of the synchronization mechanism need to be exposed read and write both ways to the chugin

The bridging interface across chuck core and chugin includes: 
- Renderer (+ window manager) needs to
  - READ:
    - CGL scenegraph
    - double-buffered command queue
  - Write:
    - CGLUpdate event
    - Swap the command queue buffers
- Chuck core needs to:
  - READ
    - Receive UpdateEvent
  - WRITE
    - Trigger Render() command
    - write to the doubled-buffered command queue


But wait, shouldn't window manager be independent too? Should that live in chuck core? are window manager and rendering engine separate? 