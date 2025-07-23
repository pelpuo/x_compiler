int test(int x, int y);

int putchar(int c);


int main() {
    int x = 20;
    x = test(x, 0);

    putchar(69);
    putchar(68);
    putchar(87);
    putchar(49);
    putchar(78);
    putchar(3);
    putchar(10);
    
    return x;
 }
 
int test(int x, int y) {
    for(int i=0; i<5;i++){
        x++;
    }
    return x + y;
 }
