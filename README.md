# EngineTest

Testing environment using DirectX 11. Most of the files here are irrelevant, but this may be useful for testing physics and other systems that require visual feedback.

The main functions of interest for physics testing are:
1. PhysicsSubsystem.Update()
2. PhysicsSubsytem::GJK() and its helpers
3. BoxCollider::GJK::Support() - support function (part of the GJK algorithm) inherited from ConvexCollider
4. Game::CreateGemoetry() - this is where meshes, colliders and rigidbodies are created and where their locations are initialized
5. class Rigidbody

ImGUI is used for easy modification of values and viewing of certain values. Elements in the ImGUI window are created in Game::BuildImGui().
