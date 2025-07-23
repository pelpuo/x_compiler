int fib(int x);


int main() {
    int x = 5;
    int y = fib(x);
    return y;
 }
 
int fib(int x){
    if(x <= 1){
        return x;
    } else {
        return fib(x - 1) + fib(x - 2);
    }
}
