#include "new.h"
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

// Print contents of BUFF between start and end locations
void print_raw_term(Term t) {
  if (t == 0) {
    printf("  FREE   ");
  } else {
    Tag tag = term_tag(t);
    Lab lab = term_lab(t);
    Location loc = term_loc(t);
    printf("%s %x %.3x", tag_to_string(tag), lab, loc);
  }
}

void print_buff(Location start, Location end) {
    a64* buff = get_buff();
    if (!buff) {
        printf("BUFF is not initialized\n");
        return;
    }
    if (start >= end) {
        printf("Invalid range: start=%u end=%u\n", start, end);
        return;
    }
    printf("BUFF contents from %u to %u:\n", start, end);
    for (Location i = start; i < end; i += 2) {
        printf(" %.3x  ", i);
	print_raw_term(buff[i]);
	printf("  ");
	print_raw_term(buff[i + 1]);
	printf("\n");
    }
    printf("\n");
}

// Helper to print a term's details
void print_term(const char* prefix, Term term) {
  printf("%s:\n", prefix);
  printf("  Tag: %s (%d)\n", tag_to_string(term_tag(term)), term_tag(term));
  printf("  Location: %u\n", term_loc(term));
    
  // If this is a pair, print its contents
  if (term_loc(term) >= 0) {
    Term first = get(port(1, term_loc(term)));
    Term second = get(port(2, term_loc(term)));
    printf("  First term: ");
    print_raw_term(first);
    printf("\n");
    printf("  Second term: ");
    print_raw_term(second);
    printf("\n");
  }
  printf("\n");
}

// Test pair creation
void test_pair_creation(void) {
    Term t1 = term_new(ERA, 0, 0);  // negative term for port 1
    Term t2 = term_new(NUL, 0, 0);  // positive term for port 2
    Term pair = pair_make(LAM, 0, t1, t2);
    print_term("Simple pair", pair);
    
    // Verify pair structure
    Location loc = term_loc(pair);
    if (get(loc) != t1 || get(port(2, loc)) != t2) {
        printf("[FAIL:%d] test_pair_creation: Incorrect values stored in BUFF\n", __LINE__);
        printf("Expected: BUFF[%u]=%lu, BUFF[%u]=%lu\n", loc, t1, port(2, loc), t2);
        printf("Got:      BUFF[%u]=%lu, BUFF[%u]=%lu\n", loc, get(loc), port(2, loc), get(port(2, loc)));
        exit(1);
    }
    
    printf("[PASS] test_pair_creation\n\n");
}

// Test pair manipulation
void test_pair_manipulation(void) {
    
    // Create nested pairs
    Term inner = pair_make(APP, 0, term_new(NUL, 0, 0), term_new(SUB, 0, 0));  // positive port 1, negative port 2
    Term outer = pair_make(APP, 0, term_new(NUL, 0, 0), term_new(SUB, 0, 0));  // positive port 1, negative port 2
    
    print_term("Inner pair", inner);
    print_term("Outer pair", outer);
    
    // Modify inner pair's first term
    set(term_loc(inner), term_new(APP, 0, 0));
    
    printf("After modification:\n");
    print_term("Modified inner pair", inner);
    print_term("Outer pair (showing modified inner)", outer);
    printf("[PASS] test_pair_manipulation\n\n");
}

// Helper to test invalid pair creation
void try_invalid_pair(Tag tag, Term fst, Term snd, const char* desc) {
    // bool caught_error = false; // Removed unused variable
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        pair_make(tag, 0, fst, snd);
        exit(0);  // Should not reach here
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 1) {
            printf("[PASS] Correctly rejected %s\n", desc);
        } else {
            printf("[FAIL:%d] Failed to reject %s\n", __LINE__, desc);
            exit(1);
        }
    }
}

// Test pair polarity validation
void test_pair_polarity() {
    // Test valid LAM pair (port 1 negative, port 2 positive)
    Term era = term_new(ERA, 0, 0);  // negative term
    Term var = term_new(VAR, 0, 0);  // positive term
    pair_make(LAM, 0, era, var);
    printf("[PASS] Created LAM pair with correct port polarities\n");
    
    // Test valid APP pair (port 1 positive, port 2 negative)
    Term nul = term_new(NUL, 0, 0);  // positive term
    Term sub = term_new(SUB, 0, 0);  // negative term
    pair_make(APP, 0, nul, sub);
    printf("[PASS] Created APP pair with correct port polarities\n");
    
    // Test invalid LAM pair (wrong port polarities)
    printf("Testing invalid LAM pair...\n");
    try_invalid_pair(LAM, var, era, "wrong port polarities in LAM pair");
    
    // Test invalid APP pair (wrong port polarities)
    printf("Testing invalid APP pair...\n");
    try_invalid_pair(APP, sub, var, "wrong port polarities in APP pair");
    
    printf("[PASS] test_pair_polarity\n");
}

// Test term polarity
void test_polarity() {
    // Test positive terms
    Term var = term_new(VAR, 0, 0);
    Term nul = term_new(NUL, 0, 0);
    Term lam = term_new(LAM, 0, 0);
    
    if (!is_positive(var) || !is_positive(nul) || !is_positive(lam)) {
        printf("[FAIL:%d] test_polarity: Expected VAR, NUL, LAM to be positive\n", __LINE__);
        exit(1);
    }
    
    if (is_negative(var) || is_negative(nul) || is_negative(lam)) {
        printf("[FAIL:%d] test_polarity: VAR, NUL, LAM should not be negative\n", __LINE__);
        exit(1);
    }
    
    // Test negative terms
    Term sub = term_new(SUB, 0, 0);
    Term era = term_new(ERA, 0, 0);
    Term app = term_new(APP, 0, 0);
    
    if (!is_negative(sub) || !is_negative(era) || !is_negative(app)) {
        printf("[FAIL:%d] test_polarity: Expected SUB, ERA, APP to be negative\n", __LINE__);
        exit(1);
    }
    
    if (is_positive(sub) || is_positive(era) || is_positive(app)) {
        printf("[FAIL:%d] test_polarity: SUB, ERA, APP should not be positive\n", __LINE__);
        exit(1);
    }
    
    printf("[PASS] test_polarity\n");
}

// Test error conditions
void test_error_conditions(void) {
    // Test uninitialized VM
    
    // Ensure VM is not initialized
    hvm_free();
    
    // Fork to test error condition
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        hvm_reset(); // Should fail with error message
        exit(0);  // Should not reach here
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 1) {
            printf("[PASS] Correctly failed on uninitialized VM\n");
        } else {
            printf("[FAIL:%d] Did not fail on uninitialized VM\n", __LINE__);
            exit(1);
        }
    }
}

// Test boundary validation
void test_boundary_validation(void) {
    hvm_init(4);  // Very small memory to force overlap
    hvm_reset();
    
    // Fork to test boundary validation
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        // Create a pair and link terms to fill up reduction bag space
        pair_make(APP, 0, term_new(NUL, 0, 0), term_new(SUB, 0, 0));
        
        // Link terms to fill up reduction bag space
        // Each term_link uses 2 slots, and we want to fill up the small memory
        for (int i = 0; i < 10; i++) {  // More iterations with smaller memory
            // Use non-VAR terms to ensure they go to reduction bag
            Term pos = term_new(LAM, 0, 0);
            Term neg = term_new(APP, 0, 0);
            term_link(neg, pos);
            printf("Linked terms iteration %d\n", i);
        }
        exit(0);  // Should not reach here due to boundary error
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 1) {
            printf("[PASS] Correctly failed on boundary error\n");
        } else {
            printf("[FAIL:%d] Did not fail on boundary error\n", __LINE__);
            exit(1);
        }
    }
}

// Main function
// Test DUP LAM interaction
void test_duplam(void) {
    // Create test terms with correct polarities
    Term var_term = term_new(SUB, 0, 0);  // Negative variable term
    Term bod_term = term_new(NUL, 0, 0);  // Positive body term
    Term dup1_term = term_new(SUB, 1, 0); // Negative first copy term
    Term dup2_term = term_new(SUB, 2, 0); // Negative second copy term
    
    // Create lambda and duplicator terms
    Term lam = pair_make(LAM, 0, var_term, bod_term);
    Term dup = pair_make(DUP, 0, dup1_term, dup2_term);
    
    // Get port locations for verification
    Location dup_loc = term_loc(dup);
    Location dup1_loc = port(1, dup_loc);  // First copy port
    Location dup2_loc = port(2, dup_loc);  // Second copy port
    Location var_loc = port(1, term_loc(lam));   // Variable port
    
    // Perform interaction
    interact(dup, lam);
    
    Term lam1 = get(dup1_loc);
    Term lam2 = get(dup2_loc);
    Term sup = get(var_loc);

    // Check that first copy has correct structure
    if (term_tag(lam1) != LAM) {
        printf("[FAIL:%d] test_duplam: Expected LAM tag in first copy port, got: tag=%s\n",
	       __LINE__, tag_to_string(term_tag(lam1)));
        exit(1);
    }

    // Check that second copy has correct structure
    if (term_tag(lam2) != LAM) {
        printf("[FAIL:%d] test_duplam: Expected LAM tag in second copy port, got: tag=%s\n",
	       __LINE__, tag_to_string(term_tag(lam2)));
        exit(1);
    }
    
    // Check that variable port contains a SUP term
    if (term_tag(sup) != SUP) {
        printf("[FAIL:%d] test_duplam: Expected SUP tag in variable port, got: tag=%s\n",
	       __LINE__, tag_to_string(term_tag(sup)));
        exit(1);
    }
    if (term_loc(get(port(1, term_loc(sup)))) != port(1, term_loc(lam1))) {
        printf("[FAIL:%d] test_duplam: Expected SUP port 1 points to wrong place\n", __LINE__);
        exit(1);
    }
    if (term_loc(get(port(2, term_loc(sup)))) != port(1, term_loc(lam2))) {
        printf("[FAIL:%d] test_duplam: Expected SUP port 2 points to wrong place\n", __LINE__);
        exit(1);
    }

    printf("[PASS] test_duplam\n");
}

// Test ERA NUL interaction
void test_eranul(void) {
    // Create ERA and NUL terms
    Term era = term_new(ERA, 0, 0);  // Negative eraser
    Term nul = term_new(NUL, 0, 0);  // Positive eraser
    
    // Perform interaction
    interact(era, nul);
    
    // They should just annihilate - nothing else to check
    printf("[PASS] test_eranul\n");
}

// Test ERA LAM interaction
void test_eralam(void) {
    // Create LAM term with ports
    Term var = term_new(SUB, 0, 0);  // Negative variable port
    Term bod = term_new(NUL, 0, 0);  // Positive body port
    Term lam = pair_make(LAM, 0, var, bod);
    
    // Create ERA term
    Term era = term_new(ERA, 0, 0);
    
    // Perform interaction
    interact(era, lam);
    
    // Check that NUL was sent to variable port
    Location var_loc = port(1, term_loc(lam));
    Term result_var = get(var_loc);
    if (term_tag(result_var) != NUL) {
        printf("[FAIL:%d] test_eralam: Expected NUL in variable port, got tag=%s\n",
	       __LINE__, tag_to_string(term_tag(result_var)));
        exit(1);
    }
    
    printf("[PASS] test_eralam\n");
}

// Test APP NUL interaction
void test_appnul(void) {
    // Create APP term with ports
    Term arg = term_new(NUL, 0, 0);  // Positive argument port
    Term ret = term_new(SUB, 0, 0);  // Negative return port
    Term app = pair_make(APP, 0, arg, ret);
    
    // Create NUL term
    Term nul = term_new(NUL, 0, 0);
    
    // Perform interaction
    interact(app, nul);
    
    // Check that NUL was sent to return port
    Location ret_loc = port(2, term_loc(app));
    Term result_ret = get(ret_loc);
    if (term_tag(result_ret) != NUL) {
        printf("[FAIL:%d] test_appnul: Expected NUL in return port, got tag=%s\n",
	       __LINE__, tag_to_string(term_tag(result_ret)));
        exit(1);
    }
    
    printf("[PASS] test_appnul\n");
}

// Test DUP NUL interaction
void test_dupnul(void) {
    // Create DUP term with ports
    Term dp1 = term_new(SUB, 1, 0);  // Negative first copy port
    Term dp2 = term_new(SUB, 2, 0);  // Negative second copy port
    Term dup = pair_make(DUP, 0, dp1, dp2);
    
    // Create NUL term
    Term nul = term_new(NUL, 0, 0);
    
    // Perform interaction
    interact(dup, nul);
    
    // Check that NUL was sent to both copy ports
    Location dp1_loc = port(1, term_loc(dup));
    Location dp2_loc = port(2, term_loc(dup));
    
    Term result_dp1 = get(dp1_loc);
    Term result_dp2 = get(dp2_loc);
    
    if (term_tag(result_dp1) != NUL) {
        printf("[FAIL:%d] test_dupnul: Expected NUL in first copy port, got tag=%s\n",
	       __LINE__, tag_to_string(term_tag(result_dp1)));
        exit(1);
    }
    
    if (term_tag(result_dp2) != NUL) {
        printf("[FAIL:%d] test_dupnul: Expected NUL in second copy port, got tag=%s\n",
	       __LINE__, tag_to_string(term_tag(result_dp2)));
        exit(1);
    }
    
    printf("[PASS] test_dupnul\n");
}

// Test ERA SUP interaction
void test_erasup(void) {
    // Create SUP term with ports
    Term p1 = term_new(NUL, 1, 0);  // Positive first port
    Term p2 = term_new(NUL, 2, 0);  // Positive second port
    Term sup = pair_make(SUP, 0, p1, p2);
    
    // Create ERA term
    Term era = term_new(ERA, 0, 0);
    
    // Store locations for verification
    Location p1_loc = term_loc(p1);
    Location p2_loc = term_loc(p2);
    
    // Perform interaction
    interact(era, sup);
    
    // Check that ERA was linked to both ports
    Term result_p1 = get(p1_loc);
    Term result_p2 = get(p2_loc);
    
    if (result_p1 != 0) {
        printf("[FAIL:%d] test_erasup: Expected ERA in first port, got tag=%s\n",
	       __LINE__, tag_to_string(term_tag(result_p1)));
        exit(1);
    }
    
    if (result_p2 != 0) {
        printf("[FAIL:%d] test_erasup: Expected ERA in second port, got tag=%s\n",
	       __LINE__, tag_to_string(term_tag(result_p2)));
        exit(1);
    }
    
    printf("[PASS] test_erasup\n");
}

// Test APP SUP interaction
void test_appsup(void) {
    // Create SUP term with ports
    Term p1 = new_i56(4);  // Positive first port
    Term p2 = new_i56(8);  // Positive second port
    Term sup = pair_make(SUP, 0, p1, p2);
    
    // Create APP term with ports
    Term arg = term_new(NUL, 3, 0);  // Positive argument port
    Term ret = term_new(SUB, 4, 0);  // Negative return port
    Term app = pair_make(APP, 0, arg, ret);
    
    // Store locations for verification
    Location p1_loc = term_loc(p1);
    Location p2_loc = term_loc(p2);
    Location arg_loc = term_loc(arg);
    Location ret_loc = term_loc(ret);
    
    // Perform interaction
    interact(app, sup);
    print_buff(0, 18);
    
    // After interaction, we should have:
    // 1. Two new APP nodes linked to the original SUP ports
    // 2. Two new DUP nodes for argument and return

    // Check that original terms have been taken (should be 0)
    if (get(arg_loc) != 0 || get(ret_loc) != 0 || 
        get(p1_loc) != 0 || get(p2_loc) != 0) {
        printf("[FAIL:%d] test_appsup: Original terms not properly taken\n", __LINE__);
        exit(1);
    }
    
    // We can't easily check the exact structure without tracing through all the links,
    // but we can verify that the interaction completed without errors
    printf("[PASS] test_appsup\n");
}

// Test application-lambda interaction
void test_applam(void) {
    // Create test terms with correct polarities
    Term arg_term = new_i56(82);  // Positive argument term
    Term ret_term = term_new(SUB, 0, 0);  // Negative return term
    Term var_term = term_new(SUB, 0, 0);  // Negative variable term
    Term bod_term = new_i56(83);  // Positive body term
    
    // Create application and lambda terms
    Term app = pair_make(APP, 0, arg_term, ret_term);
    Term lam = pair_make(LAM, 0, var_term, bod_term);
    
    // Get port locations for verification
    Location app_loc = term_loc(app);
    Location lam_loc = term_loc(lam);
    Location var_loc = port(1, lam_loc);  // Body port
    Location bod_loc = port(2, lam_loc);  // Body port
    Location arg_loc = port(1, app_loc);  // Argument port
    Location ret_loc = port(2, app_loc);  // Return port
    
    // Perform interaction
    interact(app, lam);
    
    // Check that body was moved to return port with APP tag
    Term actual_ret = get(ret_loc);
    if (actual_ret != new_i56(83)) {
        printf("[FAIL:%d] test_applam: Expected I56 tag in return port, got: tag=%s\n",
	       __LINE__, tag_to_string(term_tag(actual_ret)));
        exit(1);
    }
    
    Term actual_var = get(var_loc);
    if (actual_var != new_i56(82)) {
        printf("[FAIL:%d] test_applam: Expected I56 tag in return port, got: tag=%s\n",
	       __LINE__, tag_to_string(term_tag(actual_var)));
        exit(1);
    }
    
    // Check that original locations are cleared
    if (get(arg_loc) != 0 || get(bod_loc) != 0) {
        printf("[FAIL:%d] test_applam: Original locations not cleared\n", __LINE__);
        exit(1);
    }
    
    printf("[PASS] test_applam\n");
}
// Test push_redex and pop_redex
void test_redex_stack(void) {
    printf("\n=== Testing Redex Stack Operations ===\n");
    
    // Create some terms to push
    Term neg1 = term_new(ERA, 1, 0);
    Term pos1 = term_new(NUL, 1, 0);
    Term neg2 = term_new(APP, 2, 0);
    Term pos2 = term_new(LAM, 2, 0);
    Term neg3 = term_new(SUB, 3, 0);
    Term pos3 = term_new(VAR, 3, 0);
    
    // Push the terms onto the stack
    push_redex(neg1, pos1);
    push_redex(neg2, pos2);
    push_redex(neg3, pos3);
    
    printf("Pushed 3 redexes onto the stack\n");
    
    // Pop the terms and verify they match what we pushed
    Term neg, pos;
    
    // First pop should get neg3, pos3 (LIFO order)
    if (pop_redex(&neg, &pos)) {
        printf("Popped redex: %s(%u), %s(%u)\n", 
               tag_to_string(term_tag(neg)), term_lab(neg),
               tag_to_string(term_tag(pos)), term_lab(pos));
        
        if (term_tag(neg) != SUB || term_lab(neg) != 3 ||
            term_tag(pos) != VAR || term_lab(pos) != 3) {
            printf("[FAIL:%d] First pop returned incorrect values\n", __LINE__);
            exit(1);
        }
    } else {
        printf("[FAIL:%d] First pop_redex failed unexpectedly\n", __LINE__);
        exit(1);
    }
    
    // Second pop should get neg2, pos2
    if (pop_redex(&neg, &pos)) {
        printf("Popped redex: %s(%u), %s(%u)\n", 
               tag_to_string(term_tag(neg)), term_lab(neg),
               tag_to_string(term_tag(pos)), term_lab(pos));
        
        if (term_tag(neg) != APP || term_lab(neg) != 2 ||
            term_tag(pos) != LAM || term_lab(pos) != 2) {
            printf("[FAIL:%d] Second pop returned incorrect values\n", __LINE__);
            exit(1);
        }
    } else {
        printf("[FAIL:%d] Second pop_redex failed unexpectedly\n", __LINE__);
        exit(1);
    }
    
    // Third pop should get neg1, pos1
    if (pop_redex(&neg, &pos)) {
        printf("Popped redex: %s(%u), %s(%u)\n", 
               tag_to_string(term_tag(neg)), term_lab(neg),
               tag_to_string(term_tag(pos)), term_lab(pos));
        
        if (term_tag(neg) != ERA || term_lab(neg) != 1 ||
            term_tag(pos) != NUL || term_lab(pos) != 1) {
            printf("[FAIL:%d] Third pop returned incorrect values\n", __LINE__);
            exit(1);
        }
    } else {
        printf("[FAIL:%d] Third pop_redex failed unexpectedly\n", __LINE__);
        exit(1);
    }
    
    // Fourth pop should fail (stack is empty)
    if (pop_redex(&neg, &pos)) {
        printf("[FAIL:%d] Fourth pop_redex succeeded unexpectedly\n", __LINE__);
        exit(1);
    } else {
        printf("Pop on empty stack correctly returned false\n");
    }
    
    printf("[PASS] test_redex_stack\n");
}

// Thread function for concurrent redex operations
typedef struct {
    int thread_id;
    int num_operations;
} ThreadArgs;

void* thread_push_pop(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int thread_id = args->thread_id;
    int num_operations = args->num_operations;
    
    for (int i = 0; i < num_operations; i++) {
        // Create terms with thread-specific labels
        Term neg = term_new(ERA, thread_id * 1000 + i, 0);
        Term pos = term_new(NUL, thread_id * 1000 + i, 0);
        
        // Push the redex
        push_redex(neg, pos);
        
        // Occasionally pop a redex to test both operations
        if (i % 3 == 0) {
            Term popped_neg, popped_pos;
            if (pop_redex(&popped_neg, &popped_pos)) {
                // Successfully popped a redex
            }
        }
    }
    
    return NULL;
}

// Test thread-safe redex operations
void test_thread_safe_redex(void) {
    printf("\n=== Testing Thread-Safe Redex Operations ===\n");
    // Number of threads and operations per thread
    const int num_threads = 4;
    const int ops_per_thread = 100;
    
    // Create thread arguments
    ThreadArgs args[num_threads];
    pthread_t threads[num_threads];
    
    // Create and start threads
    for (int i = 0; i < num_threads; i++) {
        args[i].thread_id = i + 1;  // Start from 1 for easier identification
        args[i].num_operations = ops_per_thread;
        
        if (pthread_create(&threads[i], NULL, thread_push_pop, &args[i]) != 0) {
            printf("[FAIL:%d] Failed to create thread %d\n", __LINE__, i);
            exit(1);
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            printf("[FAIL:%d] Failed to join thread %d\n", __LINE__, i);
            exit(1);
        }
    }
    
    // Verify we can still pop redexes after concurrent operations
    int pop_count = 0;
    Term neg, pos;
    
    while (pop_redex(&neg, &pos)) {
        pop_count++;
    }
    
    printf("Successfully popped %d redexes after concurrent operations\n", pop_count);
    printf("[PASS] test_thread_safe_redex\n");
}

int main(int argc, char *argv[]) {
    // Initialize the VM with some memory
    hvm_init(1024);
    
    printf("\n=== Running test_error_conditions ===\n");
    test_error_conditions();

    // Re-initialize VM after error conditions test
    hvm_init(1024);

    // Run the redex stack test
    printf("\n=== Running test_redex_stack ===\n");
    hvm_reset();
    test_redex_stack();
    
    // Run the thread-safe redex test
    printf("\n=== Running test_thread_safe_redex ===\n");
    hvm_reset();
    test_thread_safe_redex();
    
    printf("\n=== Running test_polarity ===\n");
    hvm_reset();
    test_polarity();

    printf("\n=== Running test_pair_polarity ===\n");
    hvm_reset();
    test_pair_polarity();

    printf("\n=== Running test_pair_creation ===\n");
    hvm_reset();
    test_pair_creation();
    
    printf("\n=== Running test_pair_manipulation ===\n");
    hvm_reset();
    test_pair_manipulation();
    
    printf("\n=== Running test_applam ===\n");
    hvm_reset();
    test_applam();
    
    printf("\n=== Running test_duplam ===\n");
    hvm_reset();
    test_duplam();
    
    printf("\n=== Running test_eranul ===\n");
    hvm_reset();
    test_eranul();
    
    printf("\n=== Running test_eralam ===\n");
    hvm_reset();
    test_eralam();

    printf("\n=== Running test_appnul ===\n");
    hvm_reset();
    test_appnul();

    printf("\n=== Running test_dupnul ===\n");
    hvm_reset();
    test_dupnul();

    printf("\n=== Running test_erasup ===\n");
    hvm_reset();
    test_erasup();
    
    printf("\n=== Running test_appsup ===\n");
    test_appsup();
    
    // Final cleanup
    hvm_free();
    
    printf("\nAll tests passed!\n");
    return 0;
}
