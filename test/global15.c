int a[3];
int main() {
    int i;
    for(i = 0; i < 3; i = i + 1)
        a[i] = i + 1;
    return a[2];
}
