
// to run: clang -g -o new-test new-test.c new.c && ./new-test
//     or: gcc -g -o new-test new-test.c new.c && ./new-test

#include "new.h"
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

// External function declarations
extern bool subnul(Term sub, Term nul);

// Print contents of BUFF between start and end locations
void print_raw_term(Term t) {
  if (t == 0) {
    printf("  FREE   ");
  } else {
    Tag tag = term_tag(t);
    Lab lab = term_lab(t);
    switch(term_tag(t)) {
    case VAL:
    case SUB:
    case NUL:
    case REF:
    case ERA:
    case I56:
    case F56:
      printf("%s %x", tag_to_string(tag), lab);
      break;

    default:
      printf("%s %x %.3x", tag_to_string(tag), lab, term_loc(t));
      break;
    }
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

// Print contents of RBAG_BUFF between start and end locations
void print_rbag(Location start, Location end) {
  Term* buff = get_rbag_buff();
  if (!buff) {
    printf("RBAG_BUFF is not initialized\n");
    return;
  }
  if (start >= end) {
    printf("Invalid range: start=%u end=%u\n", start, end);
    return;
  }
  printf("RBAG contents from %u to %u:\n", start, end);
  for (Location i = start; i < end; i += 2) {
    printf(" %.3x  ", i);
    print_raw_term(buff[i]);
    printf("  ");
    print_raw_term(buff[i + 1]);
    printf("\n");
  }
  printf("\n");
}

Term identity_lambda() {
  Term lam= pair_make(LAM, 0,
		      term_new(SUB, 0, 0),
		      term_new(NUL, 0, 0));
  // make 'lam' the identity fn
  set(port(2, term_loc(lam)), term_new(VAR, 0, port(1, term_loc(lam))));
  return lam;
}

Term get_var(Term t) {
  while(term_tag(t) == VAR) {
    t = get(term_loc(t));
  }
  return t;
}

// Helper to print a term's details
void print_term(const char* prefix, Term term) {
  printf("%s:\n", prefix);
  printf("  Tag: %s (%d)\n", tag_to_string(term_tag(term)), term_tag(term));
  switch(term_tag(term)) {
  case VAL:
  case SUB:
  case NUL:
  case REF:
  case ERA:
  case I56:
  case F56:
    break;

  default:
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
    break;
  }

  printf("\n");
}

// Test pair creation
void test_pair_creation(void) {
  Term t1 = term_new(ERA, 0, 0);  // negative term for port 1
  Term t2 = term_new(NUL, 0, 0);  // positive term for port 2
  Term pair = pair_make(LAM, 0, t1, t2);
  // print_term("Simple pair", pair);

  // Verify pair structure
  Location loc = term_loc(pair);
  if (get(loc) != t1 || get(port(2, loc)) != t2) {
    printf("[FAIL:%d] test_pair_creation: Incorrect values stored in BUFF\n", __LINE__);
    printf("Expected: BUFF[%u]=%lu, BUFF[%u]=%lu\n", loc, t1, port(2, loc), t2);
    printf("Got:      BUFF[%u]=%lu, BUFF[%u]=%lu\n", loc, get(loc), port(2, loc), get(port(2, loc)));
    exit(1);
  }

  printf("[PASS] test_pair_creation\n");
}

// Test pair manipulation
void test_pair_manipulation(void) {

  // Create nested pairs
  Term inner = pair_make(APP, 0, term_new(NUL, 0, 0), term_new(SUB, 0, 0));  // positive port 1, negative port 2
  Term outer = pair_make(APP, 0, term_new(NUL, 0, 0), term_new(SUB, 0, 0));  // positive port 1, negative port 2

  // print_term("Inner pair", inner);
  // print_term("Outer pair", outer);

  // Modify inner pair's first term
  set(term_loc(inner), term_new(APP, 0, 0));

  // printf("After modification:\n");
  // print_term("Modified inner pair", inner);
  // print_term("Outer pair (showing modified inner)", outer);
  printf("[PASS] test_pair_manipulation\n");
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
  try_invalid_pair(LAM, var, era, "wrong port polarities in LAM pair");

  // Test invalid APP pair (wrong port polarities)
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

void test_interact(Term neg, Term pos) {
  interact(neg, pos);
  // printf("redexes: %ld\n", RBAG_END - RBAG_INI);
  stop_reducing = true;
  // print_buff(0, 18);
  normalize();
  // printf("redexes: %ld\n", RBAG_END - RBAG_INI);
  // print_buff(0, 18);
}

// Main function
// Test ERA SUP interaction
void test_erasup(void) {
  // Create SUP term with ports
  Term p1 = new_i56(7);  // Positive first port
  Term p2 = new_i56(8);  // Positive second port
  Term sup = pair_make(SUP, 0, p1, p2);

  // Create ERA term
  Term era = term_new(ERA, 0, 0);

  // Store locations for verification
  Location p1_loc = port(1, term_loc(sup));
  Location p2_loc = port(2, term_loc(sup));

  // Perform interaction
  test_interact(era, sup);

  // Check that ERA was linked to both ports
  Term result_p1 = get(p1_loc);
  Term result_p2 = get(p2_loc);

  if (term_tag(result_p1) != NUL) {
    printf("[FAIL:%d] test_erasup: Expected first port to be free, got tag=%s\n",
	   __LINE__, tag_to_string(term_tag(result_p1)));
    exit(1);
  }

  if (result_p2 != 0) {
    printf("[FAIL:%d] test_erasup: Expected second port to be free, got tag=%s\n",
	   __LINE__, tag_to_string(term_tag(result_p2)));
    exit(1);
  }

  printf("[PASS] test_erasup\n");
}

// Test DUP LAM interaction
void test_duplam(void) {
  // Create lambda and duplicator terms
  Term lam_result = new_i56(6);
  Term lam = pair_make(LAM, 0,
		       term_new(ERA, 0, 0),
		       lam_result);
  Term dup = pair_make(DUP, 0,
		       term_new(SUB, 0, 0),
		       term_new(SUB, 0, 0));

  // Get port locations for verification
  Location dup_loc = term_loc(dup);
  Location dup1_loc = port(1, dup_loc);  // First copy port
  Location dup2_loc = port(2, dup_loc);  // Second copy port
  Location var_loc = port(1, term_loc(lam));   // Variable port

  // Perform interaction
  test_interact(dup, lam);

  Term lam1 = get(dup1_loc);
  Term lam2 = get(dup2_loc);
  Term sup = get(var_loc);

  // Check that first copy has correct structure
  if (term_tag(lam1) != LAM) {
    printf("[FAIL:%d] test_duplam: Expected LAM tag in first copy port, got: tag=%s\n",
	   __LINE__, tag_to_string(term_tag(lam1)));
    exit(1);
  }
  Term result_var = get(port(1, term_loc(lam1)));
  if (term_tag(result_var) != ERA) {
    printf("[FAIL:%d] test_duplam: Expected ERA tag in first copy's var port, got: tag=%s\n",
	   __LINE__, tag_to_string(result_var));
    exit(1);
  }
  Term result_bod = get_var(get(port(2, term_loc(lam1))));
  if (result_bod != lam_result) {
    printf("[FAIL:%d] test_duplam: Wrong value in first copy's result port, got: %p\n",
	   __LINE__, (void *)result_bod);
    exit(1);
  }

  // Check that second copy has correct structure
  if (term_tag(lam2) != LAM) {
    printf("[FAIL:%d] test_duplam: Expected LAM tag in second copy port, got: tag=%s\n",
	   __LINE__, tag_to_string(term_tag(lam2)));
    exit(1);
  }
  result_var = get(port(1, term_loc(lam2)));
  if (term_tag(result_var) != ERA) {
    printf("[FAIL:%d] test_duplam: Expected ERA tag in second copy's var port, got: tag=%s\n",
	   __LINE__, tag_to_string(result_var));
    exit(1);
  }
  result_bod = get_var(get(port(2, term_loc(lam2))));
  if (result_bod != lam_result) {
    printf("[FAIL:%d] test_duplam: Wrong value in second copy's result port, got: %p\n",
	   __LINE__, (void *)result_bod);
    exit(1);
  }

  /*
  // TODO: finish this after DUP SUP is finished
  hvm_reset();
  lam = pair_make(LAM, 0,
  term_new(SUB, 0, 0),
  term_new(NUL, 0, 0));
  // make 'lam' the identity fn
  set(port(2, term_loc(lam)), term_new(VAR, 0, port(1, term_loc(lam))));
  dup = pair_make(DUP, 0,
  term_new(SUB, 1, 0),
  term_new(SUB, 2, 0));
  test_interact(dup, lam);

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
  // */

  printf("[PASS] test_duplam\n");
}

// Test ERA LAM interaction
void test_eralam(void) {
  // Create LAM term with ports
  Term var = term_new(ERA, 0, 0);  // Negative variable port
  Term bod = new_i56(67);  // Positive body port
  Term lam = pair_make(LAM, 0, var, bod);

  // Create ERA term
  Term era = term_new(ERA, 0, 0);

  // Perform interaction
  test_interact(era, lam);

  // Check that variable port was freed
  Term result_var = get(port(1, term_loc(lam)));
  if (term_tag(result_var) != NUL) {
    printf("[FAIL:%d] test_eralam: Expected variable port to be free, got tag=%s\n",
	   __LINE__, tag_to_string(term_tag(result_var)));
    exit(1);
  }

  // Check that variable body value was freed
  Term result_bod = get(port(2, term_loc(lam)));
  if (result_bod != 0) {
    printf("[FAIL:%d] test_eralam: Expected body port to be free, got tag=%s\n",
	   __LINE__, tag_to_string(term_tag(result_bod)));
    exit(1);
  }

  printf("[PASS] test_eralam\n");
}

// Test APP NUL interaction
void test_appnul(void) {
  // Create APP term with ports
  Term arg = new_i56(53);  // Positive argument port
  Term ret = term_new(ERA, 0, 0);  // Negative return port
  Term app = pair_make(APP, 0, arg, ret);

  // Create NUL term
  Term nul = term_new(NUL, 0, 0);

  // Perform interaction
  test_interact(app, nul);

  // Check that NUL was sent to return port
  Location ret_loc = port(2, term_loc(app));
  Term result_ret = get(ret_loc);
  if (result_ret != 0) {
    printf("[FAIL:%d] test_appnul: Expected return port to be free, got tag=%s\n",
	   __LINE__, tag_to_string(term_tag(result_ret)));
    exit(1);
  }

  printf("[PASS] test_appnul\n");
}

// Test DUP NUL interaction
void test_dupnul(void) {
  // Create DUP term with ports
  Term dp1 = term_new(SUB, 0, 0);  // Negative first copy port
  Term dp2 = term_new(SUB, 0, 0);  // Negative second copy port
  Term dup = pair_make(DUP, 0, dp1, dp2);

  // Create NUL term
  Term nul = term_new(NUL, 0, 0);

  // Perform interaction
  test_interact(dup, nul);

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

// Test APP SUP interaction
void test_appsup(void) {
  // Create SUP term with ports
  Term p1 = identity_lambda();
  Term p2 = new_i56(8);
  Term sup = pair_make(SUP, 5, p1, p2);

  // Create APP term with ports
  Term arg = new_i56(75);  // Positive argument port
  Term ret = term_new(SUB, 0, 0);  // Negative return port
  Term app = pair_make(APP, 0, arg, ret);

  // Store locations for verification
  Location p1_loc = term_loc(p1);
  Location ret_loc = port(2, term_loc(app));

  // Perform interaction
  test_interact(app, sup);

  // Check that SUP was moved to return port
  Term actual_ret = get(ret_loc);
  if (term_tag(actual_ret) != SUP) {
    printf("[FAIL:%d] test_applam: Expected SUP tag in return port, got: tag=%s\n",
	   __LINE__, tag_to_string(term_tag(actual_ret)));
    exit(1);
  }

  // Check that SUP points to the right values
  if (get_var(get(port(1, term_loc(actual_ret)))) != arg) {
    printf("[FAIL:%d] test_applam: SUP port 1 value is wrong. got:\n",
	   __LINE__);
    print_term("", get_var(get(port(1, term_loc(actual_ret)))));
    exit(1);
  }
  actual_ret = get_var(get(port(2, term_loc(actual_ret))));
  if (term_tag(actual_ret) != I56) {
    printf("[FAIL:%d] test_applam: SUP port 2 value is wrong. got:\n",
	   __LINE__);
    print_term("", actual_ret);
    exit(1);
  }

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
  test_interact(app, lam);

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
  // Create some terms to push
  Term neg1 = term_new(APP, 1, 0);
  Term pos1 = term_new(LAM, 1, 0);
  Term neg2 = term_new(APP, 2, 0);
  Term pos2 = term_new(LAM, 2, 0);
  Term neg3 = term_new(SUB, 3, 0);
  Term pos3 = term_new(VAR, 3, 0);

  // Push the terms onto the stack
  push_redex(neg1, pos1);
  push_redex(neg2, pos2);
  push_redex(neg3, pos3);

  // printf("Pushed 3 redexes onto the stack\n");

  // Pop the terms and verify they match what we pushed
  Term neg, pos;

  // First pop should get neg3, pos3 (LIFO order)
  neg = 0; pos = 0; // Reset to ensure we're getting new values
  pop_redex(&neg, &pos);
  /*
    printf("Popped redex: %s(%u), %s(%u)\n",
    tag_to_string(term_tag(neg)), term_lab(neg),
    tag_to_string(term_tag(pos)), term_lab(pos));
    // */

  if (term_tag(neg) != SUB || term_lab(neg) != 3 ||
      term_tag(pos) != VAR || term_lab(pos) != 3) {
    printf("[FAIL:%d] First pop returned incorrect values\n", __LINE__);
    exit(1);
  }

  // Second pop should get neg2, pos2
  neg = 0; pos = 0; // Reset to ensure we're getting new values
  pop_redex(&neg, &pos);
  /*
    printf("Popped redex: %s(%u), %s(%u)\n",
    tag_to_string(term_tag(neg)), term_lab(neg),
    tag_to_string(term_tag(pos)), term_lab(pos));
    // */

  if (term_tag(neg) != APP || term_lab(neg) != 2 ||
      term_tag(pos) != LAM || term_lab(pos) != 2) {
    printf("[FAIL:%d] Second pop returned incorrect values\n", __LINE__);
    exit(1);
  }

  // Third pop should get neg1, pos1
  neg = 0; pos = 0; // Reset to ensure we're getting new values
  pop_redex(&neg, &pos);
  /*
    printf("Popped redex: %s(%u), %s(%u)\n",
    tag_to_string(term_tag(neg)), term_lab(neg),
    tag_to_string(term_tag(pos)), term_lab(pos));
    // */

  if (term_tag(neg) != APP || term_lab(neg) != 1 ||
      term_tag(pos) != LAM || term_lab(pos) != 1) {
    printf("[FAIL:%d] Third pop returned incorrect values\n", __LINE__);
    exit(1);
  }

  // Note: We don't test popping from an empty stack since pop_redex now waits
  // when the stack is empty, which would cause the test to hang

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
    Term neg = term_new(APP, thread_id * 1000 + i, 0);
    Term pos = term_new(LAM, thread_id * 1000 + i, 0);

    // Push the redex
    push_redex(neg, pos);

    // Occasionally pop a redex to test both operations
    if (i % 3 == 0) {
      Term popped_neg = 0, popped_pos = 0;
      // Pop a redex - note that this might block if the stack is empty
      // but in this test there should always be redexes available
      pop_redex(&popped_neg, &popped_pos);
    }
  }

  return NULL;
}

// Test thread-safe redex operations
void test_thread_safe_redex(void) {
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

  // Pop a few redexes to verify they can be retrieved after concurrent operations
  // We can't use a while loop since pop_redex would wait indefinitely when empty
  int pop_count = 0;
  Term neg, pos;

  // Try to pop a fixed number of redexes
  // This assumes there are at least this many redexes in the stack
  for (int i = 0; i < 10; i++) {
    neg = 0; pos = 0;
    pop_redex(&neg, &pos);

    // Check if we got valid terms
    if (neg != 0 && pos != 0) {
      pop_count++;
    }
  }

  printf("Successfully popped %d redexes after concurrent operations\n", pop_count);
  printf("[PASS] test_thread_safe_redex\n");
}

// Test DUP SUP interaction
void test_dupsup(void) {
  // Create a DUP node
  Term dup = pair_make(DUP, 1, term_new(SUB, 0, 0), term_new(SUB, 0, 0));

  // Create a SUP node
  Term sup = pair_make(SUP, 2, term_new(NUL, 0, 0), term_new(NUL, 0, 0));

  // Create terms to connect to the ports
  Term a = term_new(I56, 56, 0);  // A positive term (integer 56)
  Term b = term_new(I56, 99, 0);  // Another positive term (integer 99)

  // Connect terms to SUP ports
  set(port(1, term_loc(sup)), a);
  set(port(2, term_loc(sup)), b);

  test_interact(dup, sup);

  printf("[PASS] test_dupsup\n");
}

// Helper function to count items in the free list
int count_free_list_items(void) {
  Location ptr = FREE_LIST;
  int count = 0;

  if (ptr == EMPTY_FREE_LIST) {
    return 0; // Empty list
  }

  while (ptr != EMPTY_FREE_LIST && ptr != 0 && count < 100) { // Limit to prevent infinite loops
    Term next = get(ptr);
    if (term_tag(next) != NUL) {
      break;
    }
    ptr = (Location)(next >> (TAG_SIZE + LAB_SIZE));
    count++;
  }

  return count;
}

// Test SUB/NUL interaction
void test_subnul(void) {
  Term neg_term = pair_make(APP, 42, new_i56(9), term_new(SUB, 0, 0));
  Term pos_term = pair_make(SUP, 42, new_i56(10), new_i56(11));
  // Create a SUB term with a location pointing to the pair
  // Note: We need to use a non-zero label to indicate the SUB has a location
  Term sub_term = pair_make(SUB, 1, neg_term, pos_term);
  // Create a NUL term
  Term nul_term = term_new(NUL, 0, 0);
  
  // Record the initial free list count
  int initial_count = count_free_list_items();
  
  // Directly call the subnul function
  subnul(sub_term, nul_term);
  stop_reducing = true;
  normalize();
  
  // Check that free_list has one more item in it
  int final_count = count_free_list_items();
  if (final_count != initial_count + 2) {
    printf("[FAIL:%d] test_subnul: Expected free list to have %d items, but got %d\n", 
           __LINE__, initial_count + 2, final_count);
    exit(1);
  }
  
  printf("[PASS] test_subnul\n");
}

// Test SUB terms with locations
void test_sub_with_location(void) {
  // Create a negative term (APP) for the redex
  Location app_loc = pair_alloc();
  Term neg_term = term_new(APP, 42, app_loc);
  
  // Create a positive term (SUP) for the redex
  Location sup_loc = pair_alloc();
  Term pos_term = term_new(SUP, 42, sup_loc);
  
  // Create a SUB term with a location pointing to the pair
  Term sub_term = pair_make(SUB, 1, neg_term, pos_term);
  
  // Create a location to store the SUB term
  Location sub_loc = pair_alloc();
  set(sub_loc, sub_term);
  
  // Create a dummy positive term to move to the SUB location
  Term dummy_term = term_new(LAM, 123, 0);
  
  // Clear the redex stack before our test
  Term dummy_neg, dummy_pos;
  while (pop_redex(&dummy_neg, &dummy_pos)) {
    // Just drain the stack
  }
  
  // Check that the redex stack has the redex we pushed
  Term neg, pos;
  bool has_redex;
  
  // Now test the SUB term with location functionality
  // Move the dummy term to the SUB location
  move(sub_loc, dummy_term);
  
  // Check that a redex was pushed to the stack
  has_redex = pop_redex(&neg, &pos);
  
  if (!has_redex) {
    printf("[FAIL:%d] test_sub_with_location: No redex was pushed after move to SUB\n", __LINE__);
    exit(1);
  }
  
  // The move function should have linked the terms in the pair pointed to by the SUB term
  if (term_tag(neg) != APP || term_lab(neg) != 42) {
    printf("[FAIL:%d] test_sub_with_location: Incorrect negative term after SUB move. Expected APP(42), got %s(%u)\n", 
           __LINE__, tag_to_string(term_tag(neg)), term_lab(neg));
    exit(1);
  }
  
  if (term_tag(pos) != SUP || term_lab(pos) != 42) {
    printf("[FAIL:%d] test_sub_with_location: Incorrect positive term after SUB move. Expected SUP(42), got %s(%u)\n", 
           __LINE__, tag_to_string(term_tag(pos)), term_lab(pos));
    exit(1);
  }
  
  printf("[PASS] test_sub_with_location\n");
}

// Test free list by creating, freeing, and reusing pairs
void test_free_list_reuse(void) {
  // Make sure we start with an empty free list
  if (FREE_LIST != 0) {
    printf("WARNING: Free list not empty at start of test\n");
    print_free_list();
  }

  // Create some initial pairs
  const int NUM_PAIRS = 10;
  Location pairs[NUM_PAIRS];

  for (int i = 0; i < NUM_PAIRS; i++) {
    pairs[i] = pair_alloc();
    set(port(1, pairs[i]), new_i56(i));
    set(port(2, pairs[i]), new_i56((i * 2)));
  }

  // Note the current RNOD_END
  u64 initial_rnod_end = RNOD_END;
  // print_free_list(); // Should be empty at this point

  // Free some pairs
  for (int i = 0; i < NUM_PAIRS/2; i++) {
    pair_free(pairs[i]);
  }

  // print_free_list(); // Should contain the freed pairs

  // Allocate some new pairs - these should reuse the freed locations
  Location new_pairs[NUM_PAIRS/2];
  for (int i = 0; i < NUM_PAIRS/2; i++) {
    // Check free list before allocation
    // printf("Before allocating pair %d:\n", i);
    // print_free_list();

    // Allocate a new pair
    new_pairs[i] = pair_alloc();
    // printf("Allocated new pair at location %u\n", new_pairs[i]);

    // Note the allocated location
    // printf("  New pair allocated at: %u\n", new_pairs[i]);

    // Initialize with different values
    set(port(1, new_pairs[i]), new_i56((i + 100)));
    set(port(2, new_pairs[i]), new_i56((i + 200)));

    // Check free list after allocation
    // printf("After allocating pair %d:\n", i);
    // print_free_list();
  }

  // print_free_list(); // Should be empty again after reusing all freed pairs

  // Verify RNOD_END hasn't changed significantly
  // (might have increased by a small amount if exact reuse ordering wasn't followed)
  if (RNOD_END > initial_rnod_end + 4) { // Allow a small margin
    printf("[FAIL:%d] test_free_list_reuse: RNOD_END increased too much after reusing pairs\n", __LINE__);
    printf("  Initial RNOD_END: %lu, Current RNOD_END: %lu\n", initial_rnod_end, RNOD_END);
    exit(1);
  }

  printf("[PASS] test_free_list_reuse\n");
}

// Test adding two numbers using OPX/OPY operations
void test_add_numbers(void) {
  // Create two numbers to add: 56 and 17
  Term num1 = new_i56(56);
  Term num2 = new_i56(17);

  // Create an addition operation (using OPX with OP_ADD=0 for addition)
  Term ret_port = term_new(SUB, 0, 0);  // Negative return port
  Term opx = pair_make(OPX, OP_ADD, num2, ret_port);

  // Get port locations for verification
  Location opx_loc = term_loc(opx);
  Location arg_loc = port(1, opx_loc);
  Location ret_loc = port(2, opx_loc);

  // This should trigger opxnum interaction, converting OPX to OPY
  test_interact(opx, num1);

  // Check result at return port - should be 56 + 17 = 73
  Term result = get(ret_loc);
  if (term_tag(result) != I56) {
    printf("[FAIL:%d] test_add_numbers: Expected I56 tag in result, got: tag=%s\n",
	   __LINE__, tag_to_string(term_tag(result)));
    exit(1);
  }

  // Extract the numeric value and verify
  u64 value = get_i56(result);
  if (value != 73) {
    printf("[FAIL:%d] test_add_numbers: Expected result 73, got: %lu\n",
	   __LINE__, value);
    exit(1);
  }

  printf("[PASS] test_add_numbers\n");
}

int main(int argc, char *argv[]) {
  // Initialize the VM with some memory
  hvm_init(1024);
  test_error_conditions();

  hvm_init(1024);
  hvm_reset();
  test_redex_stack();

  hvm_reset();
  test_thread_safe_redex();

  hvm_reset();
  test_polarity();

  hvm_reset();
  test_pair_polarity();

  hvm_reset();
  test_pair_creation();

  hvm_reset();
  test_pair_manipulation();

  hvm_reset();
  test_erasup();

  hvm_reset();
  test_eralam();

  hvm_reset();
  test_appnul();

  hvm_reset();
  test_dupnul();

  hvm_reset();
  test_applam();

  hvm_reset();
  test_appsup();

  hvm_reset();
  test_dupsup();

  hvm_reset();
  test_duplam();

  hvm_reset();
  test_add_numbers();

  hvm_reset();
  test_free_list_reuse();

  hvm_reset();
  test_sub_with_location();

  hvm_reset();
  test_subnul();

  // Test APP/REF interaction
  // hvm_reset();
  // test_appref();

  // Final cleanup
  hvm_free();

  printf("\nAll tests passed!\n");
  return 0;
}
