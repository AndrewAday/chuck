@(1., 2., 3., 4.) => vec4 vec;
@(1., 2., 3., 4.) => vec4 other_vec;

@(1.0, 0., 0.) => vec3 vecA;
@(1.00, 0., 0.) => vec3 vecB;

<<< vec.x >>>;
<<< vec.y >>>;
<<< vec.z >>>;
<<< vec.w >>>;

Phasor drive => blackhole;

if (vecA == vecB) {
    <<< "vec approx equal" >>>;
} else {
    <<< "vec not equal" >>>;

}


vecA.help();

while (true) {
    // <<< drive.last() >>>;
    1::samp => now;
}