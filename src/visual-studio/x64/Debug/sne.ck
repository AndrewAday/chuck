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

	// inputs
	-1 => static int snakeInput;

} Globals G;

@(0.0, 0.0, 0.0) => vec3 ORIGIN;
@(0.0, 1.0, 0.0) => vec3 UP;
@(0.0, -1.0, 0.0) => vec3 DOWN;
@(1.0, 0.0, 0.0) => vec3 RIGHT;
@(-1.0, 0.0, 0.0) => vec3 LEFT;
@(0.0, 0.0, -1.0) => vec3 FORWARD;  // openGL uses right-handed sytem
@(0.0, 0.0, 1.0) => vec3 BACK;


// Resource Initialization ==================================================

// initialize geos
SphereGeo sphereGeo;
BoxGeo boxGeo;
// init materials
NormMat normMat;  
NormMat headNormMat;
normMat.useLocal(true);  // use local space normals (so we can tell orientation)
headNormMat.wireframe(true);

// ECS classes ==================================================

class GameObject {
    fun void addComponent() {}
}

class Component {

}



class Grid {
    10 => int h;
    10 => int w;
    10 => int d;

    int minX, minY, minZ, maxX, maxY, maxZ;

	CglMesh gridMesh;
	BoxGeo gridGeo;

	NormMat gridMat;
	gridMat.wireframe(true);

    fun void Constructor(
		int height, int width, int depth,
		CglScene@ scene
	) {
		height => this.h;
		width => this.w;
		depth => this.d;

		-w / 2 => minX; w / 2 => maxX;
		-h / 2 => minY; h / 2 => maxY;
		-d / 2 => minZ; d / 2 => maxZ;

		gridGeo.set(
			height * 1.0, width * 1.0, depth * 1.0,
			height, width, depth 
		);
		gridMesh.set(gridGeo, gridMat);
		scene.AddChild(gridMesh);
    }

	// returns whether point is inside grid
    fun int Inside(vec3 pos) {
		return (
			pos.x >= minX && pos.x <= maxX &&
			pos.y >= minY && pos.y <= maxY &&
			pos.z >= minZ && pos.z <= maxZ
		);
    }
}

// linked list for snake body
class Segment {

	null @=> Segment @ next;

	CglMesh mesh;

	fun void Constructor(CglGeo @ geo, CglMat @ mat) {
		// create mesh
		mesh.set(geo, mat);
	}

	fun Segment @ GetNext() {
		return this.next;
	}

	fun void SetNext(Segment @ seg) {
		seg @=> this.next;
	}

	fun vec3 GetPos() {
		return mesh.GetPosition();
	}

	fun void SetPos(vec3 pos) {
		<<< "setpos", pos >>>;
		mesh.SetPosition(pos);
	}

	// returns whether point inside segment
	fun int Inside(vec3 point) {
		GetPos() => vec3 pos;
		<<< "is ", point, "inside seg at", pos >>>;
		(pos-point) => vec3 diff;
		if (diff.magnitude() < .1)
		{
			<<< "yes!" >>>;
			return true;
		}
		<<< "no!" >>>;
		return false;
	} 
}

class Snake {

    CglGeo @ geo;  // snake segment shape 
    CglMat @ mat;  // snake material

    CglGroup snakeObj;  // parent group for all segments
    // Segment @ segments[];  // list of individual segments. Tail is first element, head is last

	null @=> Segment @ head;
	null @=> Segment @ tail;

    // movement (relative)
	0 => static int SNAKE_UP;
	1 => static int SNAKE_DOWN;
	2 => static int SNAKE_LEFT;
	3 => static int SNAKE_RIGHT;

	FORWARD => vec3 curDir;  // movement direction in world space

    // TODO: quaternion slerping for auto camera pivot

	fun void SetHead(Segment @ seg) {
		if (this.head != null)
			this.head.mesh.set(geo, mat);  // reset head material

		seg @=> this.head;
		seg.mesh.set(geo, headNormMat);  // set new head for debugging
	}

	// fun void SetTail(Segment @ seg) {
	// 	<<< "setting tail!" >>>;
	// 	// if (this.tail != null)
	// 	// 	this.tail.mesh.set(geo, mat);  // reset head material
	// 	seg @=> this.tail;
	// 	this.head.SetNext(this.tail);

	// 	// seg.mesh.set(sphereGeo, mat);
	// }

    // constructor. call this first!
    fun void Constructor(
		CglGeo @ snakeGeo, CglMat @ snakeMat,
		CglScene @ scene
	) {
        snakeGeo @=> this.geo;
        snakeMat @=> this.mat;

		// add to scene
		scene.AddChild(snakeObj);

		// create 1-seg sneck
		Segment seg;
		seg.Constructor(this.geo, this.mat);

		SetHead(seg); seg @=> this.tail;
		tail.SetNext(head);
		head.SetNext(tail);

		// add to scene
		snakeObj.AddChild(seg.mesh);
    }

    // add new segment to head at position pos
    fun void AddSegment(vec3 pos) {
		Segment seg;
		seg.Constructor(this.geo, this.mat);

        // update position
        seg.SetPos(pos);

		// add to scene
		snakeObj.AddChild(seg.mesh);

		// update linked list
		seg.SetNext(tail);
		head.SetNext(seg);
		
		// update head
		SetHead(seg);
    }

    // moves the snake by 1 step
    fun void Slither() {
		<<< "Slither" >>>;
		if (this.head == this.tail) {
			<<< "head === tail" >>>;
		}
		// rather than move position of every segment, we only need to change
		// the tail and move it to the new head

		curDir + head.GetPos() => vec3 newPos;
		// set tail to new pos
		tail.SetPos(newPos);


		// update new tail, head, and pointers
		
		SetHead(this.head.GetNext());
		this.tail.GetNext() @=> this.tail;
    }

	// change snake direction according to input
	// wonder if there's a cleaner way to do this
	fun void UpdateDir(int input) {
		if (input < 0 ) return;  // base case, no input. continue along current direction
		if (curDir == FORWARD) {
			if (input == SNAKE_UP) { UP => curDir; }
			else if (input == SNAKE_DOWN) { DOWN => curDir; }
			else if (input == SNAKE_LEFT) { LEFT => curDir; }
			else if (input == SNAKE_RIGHT) { RIGHT => curDir; }
		} else if (curDir == BACK) {
			if (input == SNAKE_UP) { UP => curDir; }
			else if (input == SNAKE_DOWN) { DOWN => curDir; }
			else if (input == SNAKE_LEFT) { RIGHT => curDir; }
			else if (input == SNAKE_RIGHT) { LEFT => curDir; }
		} else if (curDir == RIGHT) {
			if (input == SNAKE_UP) { UP => curDir; }
			else if (input == SNAKE_DOWN) { DOWN => curDir; }
			else if (input == SNAKE_LEFT) { FORWARD => curDir; }
			else if (input == SNAKE_RIGHT) { BACK => curDir; }
		} else if (curDir == LEFT) {
			if (input == SNAKE_UP) { UP => curDir; }
			else if (input == SNAKE_DOWN) { DOWN => curDir; }
			else if (input == SNAKE_LEFT) { BACK => curDir; }
			else if (input == SNAKE_RIGHT) { FORWARD => curDir; }
		} else if (curDir == UP) {
			if (input == SNAKE_UP) { BACK => curDir; }
			else if (input == SNAKE_DOWN) { FORWARD => curDir; }
			else if (input == SNAKE_LEFT) { LEFT => curDir; }
			else if (input == SNAKE_RIGHT) { RIGHT => curDir; }
		} else if (curDir == DOWN) {
			if (input == SNAKE_UP) { FORWARD => curDir; }
			else if (input == SNAKE_DOWN) { BACK => curDir; }
			else if (input == SNAKE_LEFT) { LEFT => curDir; }
			else if (input == SNAKE_RIGHT) { RIGHT => curDir; }
		}
	}


}


// Scene Setup =============================================================
CglUpdate UpdateEvent;
CglFrame FrameEvent;
CglCamera mainCamera; 
CglScene scene;

Snake snake;
snake.Constructor(boxGeo, normMat, scene);

// testing
Grid grid;
grid.Constructor(21, 21, 21, scene);


fun void Movement() {
	while (1::second => now) {
		snake.UpdateDir(G.snakeInput);
		if (IM.isKeyDown(IM.KEY_SPACE))
			snake.AddSegment(snake.head.GetPos() + snake.curDir);
		else
			snake.Slither();
		-1 => G.snakeInput; // reset input state
	}
}
spork ~ Movement();

fun void SnakeInputHandler() {
	while (true) {
		IM.anyKeyDownEvent => now;
		if (IM.isKeyDown(IM.KEY_UP)) { Snake.SNAKE_UP => G.snakeInput; }
		else if (IM.isKeyDown(IM.KEY_DOWN)) { Snake.SNAKE_DOWN => G.snakeInput; }
		else if (IM.isKeyDown(IM.KEY_LEFT)) { Snake.SNAKE_LEFT => G.snakeInput; }
		else if (IM.isKeyDown(IM.KEY_RIGHT)) { Snake.SNAKE_RIGHT => G.snakeInput; }
	}
}
spork ~ SnakeInputHandler();

// barf...
// fun void WorldMovement() {
// 	while (true) {
// 		10::ms => now;
// 		scene.RotateOnLocalAxis(UP, 0.002);
// 	}
// } spork ~ WorldMovement();




fun void Update() {

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
		1 +=> G.frameCounter;
		
		// compute timing
		now - G.lastTime => G.deltaTime;
		now => G.lastTime;

		// Update logic
		cameraUpdate(now, G.deltaTime);
		Update();

		// End update, begin render
		CGL.Render();
	}
} 

GameLoop();
