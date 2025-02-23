int test(int x){
    return x + 5;
}

int main() {
    int x = 10;
    // for(int i = 0; i < 10; i= i+1){
    //     x += i;
    // }
    // return x;
    x = test(x);
    return x;
 }
 