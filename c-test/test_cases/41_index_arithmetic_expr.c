int main() {
    int a;
    int b;
    int result[3];
    a = 56;
    b = 12;
    result[0] = 1;
    result[1] = 2;
    result[2] = 3;
    int t;
    t = result[(a % b + b) / 5 - 2];
    return 0;
}
