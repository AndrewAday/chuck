TriOsc tri => dac;

ChuGL chugl;

(1.0/60.0)::second => dur frame;

fun void animateColor() {
    while (true) {
        chugl.clear();
        tri.freq() / 440. => float c;
        1.0 - c => c;
        chugl.color(c, c, c);
        .5::frame => now;
    }
} spork ~ animateColor();

fun void rectangle() {
    while (true) {
        chugl.rect( tri.freq() / 420. * 1000., 0, 100, 100);
        <<< tri.freq() / 420. * 1000.>>>;
        .5::frame => now;
    }

} spork ~ rectangle();

<<< "running chugl test" >>>;

.1 => tri.gain;
while (1::ms => now) {
    420. * .5 * (Math.sin(now/second) + 1.1) => tri.freq;
}
