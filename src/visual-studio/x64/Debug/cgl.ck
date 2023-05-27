InputManager IM;
spork ~ IM.start(0);

MouseManager MM;
spork ~ MM.start(0);

CglUpdate UpdateEvent;
CglCamera mainCamera;
0 => int frameCounter;
1 => int autoRender;
now => time lastTime;

// flycamera controls
@(0.0, 1.0, 0.0) => vec3 UP;
@(1.0, 0.0, 0.0) => vec3 RIGHT;
fun void cameraUpdate(time t, dur dt)
{
	2.5 * (dt / second) => float cameraSpeed;
	// camera movement
	if (IM.isKeyDown(IM.KEY_W))
		mainCamera.TranslateBy(cameraSpeed * mainCamera.GetForward());
	if (IM.isKeyDown(IM.KEY_S))
		mainCamera.TranslateBy(-cameraSpeed * mainCamera.GetForward());
	if (IM.isKeyDown(IM.KEY_D))
		mainCamera.TranslateBy(cameraSpeed * mainCamera.GetRight());
	if (IM.isKeyDown(IM.KEY_A))
		mainCamera.TranslateBy(-cameraSpeed * mainCamera.GetRight());
	if (IM.isKeyDown(IM.KEY_Q))
		mainCamera.TranslateBy(cameraSpeed * UP);
	if (IM.isKeyDown(IM.KEY_E))
		mainCamera.TranslateBy(-cameraSpeed * UP);

	// mouse lookaround

	.001 => float mouseSpeed;
	MM.GetDeltas() * mouseSpeed => vec3 mouseDeltas;

	// <<< mouseDeltas >>>;

	// for mouse deltaY, rotate around GetRight axis
	mainCamera.RotateOnLocalAxis(RIGHT, -mouseDeltas.y);

	// for mouse deltaX, rotate around (0,1,0)
	mainCamera.RotateOnWorldAxis(UP, -mouseDeltas.x);
}


/*  wrap these into single event, good ergonomics
{
CGL.Render();  // tell renderer its safe to copy and draw
UpdateEvent => now;
}
*/
// Game loop 
CGL.Render(); // kick of the renderer
fun void GameLoop(){
	while (true) {
		UpdateEvent => now;
		frameCounter++;
		
		// compute timing
		now - lastTime => dur deltaTime;
		now => lastTime;

		// Update logic
		cameraUpdate(now, deltaTime);

		// End update, begin render
		if (autoRender) { CGL.Render(); } // tell renderer its safe to copy and draw
	}
} spork ~ GameLoop();

// TODO: create Entity-component-system abstraction and move update logic into components
fun void TriggerRenderListener() {
	while (true) {
		IM.keyDownEvent(IM.KEY_SPACE) => now;
		CGL.Render();
	}
} spork ~ TriggerRenderListener();

fun void ToggleRenderListener() {
	while (true) {
		IM.keyDownEvent(IM.KEY_Z) => now;
		1 - autoRender => autoRender;
		CGL.Render();
	}
} spork ~ ToggleRenderListener();

while (true) {
	1::second => now;
}