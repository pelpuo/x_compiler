int main(void) {
    int x = 10;
    // for(int i = 0; i < 10; i= i+1){
    //     x += i;
    // }
    // return x;
    switch(x){
        case 1:
            x = 1;
            break;
        case 2:
            x = 2;
            break;
        default:
            x = 3;
    }
    return x;
 }