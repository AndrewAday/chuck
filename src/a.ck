TriOsc tri => dac;

ChuGL chugl;

fun void animateColor() {
    while (true) {
        // chugl.clear();
        .5 * (Math.sin(now/second) + 1) => float c;
        chugl.color(c, c, c);
        // chugl.rect( c*1000, 0, 100, 100);
        10::ms => now;
    }
} spork ~ animateColor();

fun void rectangle() {
    while (true) {
        
        1::ms => now;
    }

} //spork ~ rectangle();

<<< "running chugl test" >>>;

.1 => tri.gain;
420 => tri.freq;
while (1::ms => now) {}
