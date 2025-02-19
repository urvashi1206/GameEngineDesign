# EngineTest

Testing environment using DirectX 11. Most of the files here are irrelevant, but this may be useful for testing physics and other systems that require visual feedback.

You may need to change the include directory (Project > Properties > C/C++ > Addiional Include Directories) to match the location of this repo on your drive, since I didn't set it as a relative path. This is required for using the rapidobj .obj parser, which is used to create some of the meshes used.

The main functions of interest for physics testing are:
1. PhysicsSubsystem.Update()
2. PhysicsSubsytem::GJK() and its helpers
3. BoxCollider::GJK::Support() - support function (part of the GJK algorithm) inherited from ConvexCollider
4. Game::CreateGemoetry() - this is where meshes, colliders and rigidbodies are created and where their locations are initialized
5. class Rigidbody

ImGUI is used for easy modification of values and viewing of certain values. Elements in the ImGUI window are created in Game::BuildImGui().
