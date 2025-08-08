// A global variable to test taking the address of global symbols.

int main() {
    int global_var = 50; 
    int local_var_a;
    int local_var_b;
    int *ptr; // Our pointer

    // --- Test 1: Address-of and Store to a Pointer ---
    // Should generate 'lea' for the address of 'local_var_a'
    // and store it into the stack space for 'ptr'.
    ptr = &local_var_a;

    // --- Test 2: Assignment to a Dereferenced Pointer ---
    // This is the most important test. It should generate a 'store_ptr'
    // (sw or sd) instruction using the address stored in the 'ptr' register.
    *ptr = 100;

    // --- Test 3: Reading from a Dereferenced Pointer ---
    // This should generate a 'load_ptr' (lw or ld) instruction.
    local_var_b = *ptr; // local_var_b should now be 100.

    // --- Test 4: Using a Global Variable's Address ---
    // Should generate 'la' for the address of 'global_var'.
    ptr = &global_var;

    // --- Test 5: Complex expression ---
    // Tests loading a local, loading a pointer, dereferencing the pointer,
    // performing an operation, and storing the result.
    local_var_b = local_var_b + *ptr; // 100 + 50 = 150

    // The program should exit with the final calculated value.
    return local_var_b; 
}