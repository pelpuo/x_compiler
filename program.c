int test(int q){
    return q + 5;
}

int main() {
    int x = 0;
    int y = 0;
    for(int i = 0; i <= 5; i= i+1){
        x = test(x);
        y++;
    }
    return y;
 }
 