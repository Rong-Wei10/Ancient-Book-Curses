unsigned int a = 16807;
unsigned int m = 2147483647;
unsigned int seed = 1;

void srandom(unsigned int s) {
    seed = s;
}

unsigned int random() {
    if (seed == 0) {     // 0 doesn't work as a seed
        seed = 1;
    }
    seed = (a * seed) % m;
    return seed;
}
