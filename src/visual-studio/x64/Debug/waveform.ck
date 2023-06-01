// Example of using CGL to visualize waveform and spectrum

// Managers ========================================================
InputManager IM;
spork ~ IM.start(0);

MouseManager MM;
spork ~ MM.start(0);

// Globals ==============================================
class Globals {
	// static vars
	0 => static int frameCounter;
	now => static time lastTime;
	0::samp => static dur deltaTime;

} Globals G;

@(0.0, 0.0, 0.0) => vec3 ORIGIN;
@(0.0, 1.0, 0.0) => vec3 UP;
@(0.0, -1.0, 0.0) => vec3 DOWN;
@(1.0, 0.0, 0.0) => vec3 RIGHT;
@(-1.0, 0.0, 0.0) => vec3 LEFT;
@(0.0, 0.0, -1.0) => vec3 FORWARD;  // openGL uses right-handed sytem
@(0.0, 0.0, 1.0) => vec3 BACK;
@(1.0, 1.0, 1.0) => vec3 UNIFORM;

fun string VecToString(vec3 v) {
	return v.x + "," + v.y + "," + v.z;
}

fun int VecEquals(vec3 a, vec3 b) {
	b - a => vec3 c;
	return c.magnitude() < .01;
}

// chuck ugen setup
// 512 => int WAVEFORM_LENGTH;
256 => int WAVEFORM_LENGTH;

adc => Gain g => dac;
float waveformThis[WAVEFORM_LENGTH];
// float waveformThat[WAVEFORM_LENGTH];
waveformThis @=> float waveformThat[];
waveformThis @=> float writeWaveform[];
waveformThat @=> float readWaveform[];
// waveform update
fun void WaveformWriter() {
    0 => int i;
    while (true) {
        // dac.last() => writeWaveform[i++];
        g.last() => writeWaveform[i++];
        if (i >= writeWaveform.size()) {
            0 => i;
            if (writeWaveform == waveformThis) {
                waveformThat @=> writeWaveform;
                waveformThis @=> readWaveform;
            } else {
                waveformThis @=> writeWaveform;
                waveformThat @=> readWaveform;
            }
        }
        // if (i % 64 == 0)
        //     <<< dac.last() >>>;
        1::samp => now;
    }
} 
spork ~ WaveformWriter();

// Scene Setup =============================================================
CglUpdate UpdateEvent;
CglFrame FrameEvent;
CglCamera mainCamera; 
CglScene scene;

BoxGeo boxGeo;
NormMat normMat;  

CglMesh boxMeshes[WAVEFORM_LENGTH];
50.0 / WAVEFORM_LENGTH => float boxScale;

// initialize boxes for waveform
for (0 => int i; i < WAVEFORM_LENGTH; i++) {
    boxMeshes[i].set(boxGeo, normMat);
    boxMeshes[i].SetScale(boxScale * UNIFORM);
    boxMeshes[i].SetPosition(((-WAVEFORM_LENGTH/2) + i) * RIGHT * boxScale);
    scene.AddChild(boxMeshes[i]);
}

fun void UpdateVisualizer() {
    // <<< "===updating visualizer===" >>>;
    for (0 => int i; i < WAVEFORM_LENGTH; i++) {
        // boxMeshes[i].SetScale(@(1, 10 * (waveform[i] + 1.001), 1));
        // boxMeshes[i].SetPosition((boxMeshes[i].GetPosition() + (readWaveform[i] * UP)));
        boxMeshes[i].PosY((5 * (readWaveform[i])));
    }
}


// flycamera controls
fun void cameraUpdate(time t, dur dt)
{

	// set camera to always be above snake head
	// TODO: needs to be relative to the snake direction
	// OR just have wasd control camera pivot
	// OR use orbit camera controls
	// mainCamera.SetPosition(snake.head.GetPos() + @(0.0, 3.0, 3.0));
	// OR make grid, snake, everything child of one group
	// and use mouse/keys to ROTATE the group itself, like spinning the whole world around

	// mouse lookaround
	.001 => float mouseSpeed;
	MM.GetDeltas() * mouseSpeed => vec3 mouseDeltas;

	// for mouse deltaY, rotate around GetRight axis
	mainCamera.RotateOnLocalAxis(RIGHT, -mouseDeltas.y);

	// for mouse deltaX, rotate around (0,1,0)
	mainCamera.RotateOnWorldAxis(UP, -mouseDeltas.x);

	2.5 * (dt / second) => float cameraSpeed;
	if (IM.isKeyDown(IM.KEY_LEFTSHIFT))
		2.5 *=> cameraSpeed;
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

}


// Game loop 
fun void GameLoop(){
	CGL.Render(); // kick of the renderer
	while (true) {
		UpdateEvent => now;
		// <<< "==== update loop " >>>;
		1 +=> G.frameCounter;
		
		// compute timing
		now - G.lastTime => G.deltaTime;
		now => G.lastTime;

		// Update logic
		cameraUpdate(now, G.deltaTime);
        UpdateVisualizer();

		// End update, begin render
		CGL.Render();
	}
} 

GameLoop();